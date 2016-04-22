/*
 * Copyright 215, François Revol
 * Distributed under the terms of the MIT License.
 */


#include "video.h"

#include <arch/cpu.h>
#include <boot/stage2.h>
#include <boot/platform.h>
#include <boot/menu.h>
#include <boot/kernel_args.h>
#include <boot/platform/generic/video.h>
#include <util/list.h>
#include <drivers/driver_settings.h>
#include <board_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch_framebuffer.h"
#include "arch_mmu.h"
#include "mmu.h"


/*
	This is a crude hack that tries to find U-Boot's framebuffer configuration
	by probing memory locations known from the U-Boot symbol table.
	Of course you must have System.map for the U-Boot you actually use.
	TODO:check for the U-Boot version string
 */


/* U-Boot's own framebuffer descriptor */

typedef struct {
    unsigned int isaBase;
    unsigned int pciBase;
    unsigned int dprBase;
    unsigned int vprBase;
    unsigned int cprBase;
    unsigned int frameAdrs;
    unsigned int memSize;
    unsigned int mode;
    unsigned int gdfIndex;
    unsigned int gdfBytesPP;
    unsigned int fg;
    unsigned int bg;
    unsigned int plnSizeX;
    unsigned int plnSizeY;
    unsigned int winSizeX;
    unsigned int winSizeY;
    char modeIdent[80];
} GraphicDevice;


static struct {
	const char *name;
	const char *fw;
	GraphicDevice *gd;	// global GraphicDevice struct from the driver
	void **fb;	// direct pointer to the framebuffer, should match gd->frameAdrs
	void **fb_phys_base;	// physical address
} sMatchList[] = {
#ifdef __POWERPC__
	{
		"sm502",
		"u-boot-2010.06.05_20110414_prod (Sam460)",
		// nm u-boot| grep 'B sm501$'
		(GraphicDevice *)0x00011320,
		// nm u-boot| grep 'B video_fb_address$'
		(void **)0x000001ec,
		NULL
	},
	{
		"sm502",
		"u-boot-2010.06.05_20110414_prod (Sam460)",
		// nm u-boot| grep 'B sm501$'
		(GraphicDevice *)0x00011318,
		// nm u-boot| grep 'B video_fb_address$'
		(void **)0x000001ec,
		NULL
	},
#endif
	{}
};

class ArchGDFrameBuffer : public ArchFramebuffer {
public:
							ArchGDFrameBuffer(addr_t base, GraphicDevice *gd)
								: ArchFramebuffer(base) {
									memcpy(&fGD, gd, sizeof(fGD));
									fPhysicalBase = 0xC84000000LL;
								}
							~ArchGDFrameBuffer() {}

virtual	status_t			Init();
virtual	status_t			Probe();
virtual	status_t			SetDefaultMode();
virtual	status_t			SetVideoMode(int width, int height, int depth);

GraphicDevice				fGD;
};


typedef struct uboot_gd {
	// those are the only few members that we can trust
	// others depend on compile-time config
	struct board_data *bd;
	uint32 flags;
	uint32 baudrate;
	// those are ARM-only
	uint32 have_console;
	uint32 reloc_off;
	uint32 env_addr;
	uint32 env_valid;
	uint32 fb_base;
} uboot_gd;

extern uboot_gd *gUBootGlobalData;

struct board_data {
	unsigned long	bi_memstart;	/* start of DRAM memory */
	uint64	bi_memsize;	/* size	 of DRAM memory in bytes */
	unsigned long	bi_flashstart;	/* start of FLASH memory */
	unsigned long	bi_flashsize;	/* size	 of FLASH memory */
	unsigned long	bi_flashoffset; /* reserved area for startup monitor */
	unsigned long	bi_sramstart;	/* start of SRAM memory */
	unsigned long	bi_sramsize;	/* size	 of SRAM memory */
};

extern "C" ArchFramebuffer*
video_gd_probe()
{
	dprintf("Probing U-Boot's framebuffer config...\n");
	//	return NULL;

	dprintf("candidates ? ...\n");
	for (uint8 *pi = (uint8 *)/*0xe500000*/0; pi < (uint8 *)/*0xe500000*/0+0x13000; pi++) {
		GraphicDevice *p = (GraphicDevice *)pi;
		//dprintf("? %p\n", p);
		if (p->gdfIndex > 5)
			continue;
		if (p->gdfBytesPP != 1
			&& p->gdfBytesPP != 2
			&& p->gdfBytesPP != 3
			&& p->gdfBytesPP != 4)
			continue;
		dprintf("@ %p bytespp %d winSizeX %d winSizeY %d\n", p, p->gdfBytesPP, p->winSizeX, p->winSizeY);
		if (p->winSizeX < 300)
			continue;
		if (p->winSizeY < 200)
			continue;
		if (p->winSizeX > 4000)
			continue;
		if (p->winSizeY > 4000)
			continue;
		dprintf("candidate @ %p\n", p);
	}

	for (int i = 0; sMatchList[i].name; i++) {
		// some sanity checks
		dprintf("GD @ %p ?\n", sMatchList[i].gd);
		dprintf("bytespp %d winSizeX %d winSizeY %d\n", sMatchList[i].gd->gdfBytesPP, sMatchList[i].gd->winSizeX, sMatchList[i].gd->winSizeY);
		if (sMatchList[i].gd->gdfIndex > 5)
			continue;
		if (sMatchList[i].gd->gdfBytesPP != 1
			&& sMatchList[i].gd->gdfBytesPP != 2
			&& sMatchList[i].gd->gdfBytesPP != 3
			&& sMatchList[i].gd->gdfBytesPP != 4)
			continue;
		if (sMatchList[i].gd->winSizeX < 300)
			continue;
		if (sMatchList[i].gd->winSizeY < 200)
			continue;
		if (sMatchList[i].gd->winSizeX > 4000)
			continue;
		if (sMatchList[i].gd->winSizeY > 4000)
			continue;
		dprintf("GD-frameAdrs = %lx ?\n", sMatchList[i].gd->frameAdrs);
		if ((void *)sMatchList[i].gd->frameAdrs == NULL)
			continue;
		if ((void *)sMatchList[i].gd->frameAdrs != *sMatchList[i].fb)
			continue;
		dprintf("match\n");
		addr_t base = sMatchList[i].gd->frameAdrs;
		dprintf("Detected %s card from %s\n", sMatchList[i].name,
			sMatchList[i].fw);
		dprintf("Framebuffer @ %p\n", *sMatchList[i].fb);

	    return new ArchGDFrameBuffer(base, sMatchList[i].gd);
	}

	GraphicDevice forced = {0, 0x84000000LL, 0, 0, 0, 0x84000000, 0, 0, 0, 1, 0, 0, 0, 0, 640, 400, "test"
};
	forced.frameAdrs = mmu_map_physical_memory(0xC84000000LL, 16*1024*1024, kDefaultPageFlags);
	dprintf("forced FB @ %lx\n", forced.frameAdrs);
	uint32 * p = (uint32 *)forced.frameAdrs;
	*p++ = 0xffffffff;
	*p++ = 0x55555555;
	*p++ = 0xaaaaaaaa;
	return new ArchGDFrameBuffer(forced.frameAdrs, &forced);

    return NULL;
}


status_t
ArchGDFrameBuffer::Init()
{
	dprintf("FB::Init\n");
	gKernelArgs.frame_buffer.enabled = true;
	return B_OK;
}


status_t
ArchGDFrameBuffer::Probe()
{
	dprintf("FB::Probe\n");
	return B_OK;
}


status_t
ArchGDFrameBuffer::SetDefaultMode()
{
	dprintf("FB::SetDefaultMode\n");
	status_t result;
	do {
		result = SetVideoMode(fGD.winSizeX, fGD.winSizeY, fGD.gdfBytesPP * 8);
	} while (result != B_OK);

	return B_OK;
}


status_t
ArchGDFrameBuffer::SetVideoMode(int width, int height, int depth)
{
	dprintf("FB::SetVideoMode\n");
//	gKernelArgs.frame_buffer.physical_buffer.start = );;//0xC84000000;
	fCurrentWidth = width;
	fCurrentHeight = height;
	fCurrentDepth = depth;
	fCurrentBytesPerRow = width * depth / 8;
	fSize = fCurrentBytesPerRow * fCurrentHeight;



	//debug_assert(((uint32)&sFramebufferConfig & 0x0f) == 0);
#if 0
	sFramebufferConfig.width = width;
	sFramebufferConfig.height = height;
	sFramebufferConfig.virtual_width = sFramebufferConfig.width;
	sFramebufferConfig.virtual_height = sFramebufferConfig.height;
	sFramebufferConfig.bytes_per_row = 0; // from GPU
	sFramebufferConfig.bits_per_pixel = depth;
	sFramebufferConfig.x_offset = 0;
	sFramebufferConfig.y_offset = 0;
	sFramebufferConfig.frame_buffer_address = 0; // from GPU
	sFramebufferConfig.screen_size = 0; // from GPU

	if (depth < 16) {
		const int colorMapEntries = sizeof(sFramebufferConfig.color_map)
			/ sizeof(sFramebufferConfig.color_map[0]);
		for (int i = 0; i < colorMapEntries; i++)
			sFramebufferConfig.color_map[i] = 0x1111 * i;
	}

	status_t result = gMailbox->Write(ARM_MAILBOX_CHANNEL_FRAMEBUFFER,
		(uint32)&sFramebufferConfig | BCM283X_VIDEO_CORE_L2_COHERENT);
	if (result != B_OK)
		return result;

	uint32 value;
	result = gMailbox->Read(ARM_MAILBOX_CHANNEL_FRAMEBUFFER, value);
	if (result != B_OK)
		return result;

	if (value != 0) {
		dprintf("failed to configure framebuffer: %" B_PRIx32 "\n", value);
		return B_ERROR;
	}

	if (sFramebufferConfig.frame_buffer_address == 0) {
		dprintf("didn't get the framebuffer address\n");
		return B_ERROR;
	}

	//debug_assert(sFramebufferConfig.x_offset == 0
	//	&& sFramebufferConfig.y_offset == 0
	//	&& sFramebufferConfig.width == (uint32)width
	//	&& sFramebufferConfig.height == (uint32)height
	//	&& sFramebufferConfig.virtual_width == sFramebufferConfig.width
	//	&& sFramebufferConfig.virtual_height == sFramebufferConfig.height
	//	&& sFramebufferConfig.bits_per_pixel == (uint32)depth
	//	&& sFramebufferConfig.bytes_per_row
	//		>= sFramebufferConfig.bits_per_pixel / 8
	//			* sFramebufferConfig.width
	//	&& sFramebufferConfig.screen_size >= sFramebufferConfig.bytes_per_row
	//		* sFramebufferConfig.height);

	fPhysicalBase
		= BCM283X_BUS_TO_PHYSICAL(sFramebufferConfig.frame_buffer_address);
	fSize = sFramebufferConfig.screen_size;

	fBase = (addr_t)mmu_map_physical_memory(fPhysicalBase, fSize, kDefaultPageFlags);

	dprintf("video framebuffer: va: %p pa: %p\n", (void *)fBase,
		(void *)fPhysicalBase);

	fCurrentWidth = width;
	fCurrentHeight = height;
	fCurrentDepth = depth;
	fCurrentBytesPerRow = sFramebufferConfig.bytes_per_row;

	gKernelArgs.frame_buffer.physical_buffer.start = (addr_t)fPhysicalBase;
#endif
//	fCurrentWidth = fGD.winSizeX;
//	fCurrentHeight = fGD.winSizeY;
//	fCurrentDepth = fGD.gdfBytesPP * 8;
//	fCurrentBytesPerRow = fGD.gdfBytesPP * fGD.winSizeX;
	return B_OK;
}

