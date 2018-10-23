/*
 * Copyright 2009, Johannes Wischert
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch_framebuffer.h"


ArchFramebuffer *gFramebuffer = NULL;


//	#pragma mark -


bool
video_mode_hook(Menu *menu, MenuItem *item)
{
	return true;
}


Menu *
video_mode_menu()
{
	Menu *menu = new(nothrow) Menu(CHOICE_MENU, "Select Video Mode");
	MenuItem *item;

	menu->AddItem(item = new(nothrow) MenuItem("Default"));
	item->SetMarked(true);
	item->Select(true);
	item->SetHelpText("The Default video mode is the one currently configured "
		"in the system. If there is no mode configured yet, a viable mode will "
		"be chosen automatically.");


	menu->AddSeparatorItem();
	menu->AddItem(item = new(nothrow) MenuItem("Return to main menu"));
	item->SetType(MENU_ITEM_NO_CHOICE);

	return menu;
}


//	#pragma mark -


extern "C" void
platform_set_palette(const uint8 *palette)
{
}


extern "C" void
platform_blit4(addr_t frameBuffer, const uint8 *data, uint16 width,
	uint16 height, uint16 imageWidth, uint16 left, uint16 top)
{
}


extern "C" void
platform_switch_to_logo(void)
{
	// in debug mode, we'll never show the logo
	if ((platform_boot_options() & BOOT_OPTION_DEBUG_OUTPUT) != 0)
		return;

	// bootrom hardcodes video mode
	//gKernelArgs.frame_buffer.physical_buffer.start = address;
	gKernelArgs.frame_buffer.physical_buffer.size = 640 * 480;
	gKernelArgs.frame_buffer.width = 640;
	gKernelArgs.frame_buffer.height = 480;
	gKernelArgs.frame_buffer.depth = 8;
	gKernelArgs.frame_buffer.bytes_per_row = 640;

	dprintf("video mode: %ux%ux%u\n", gKernelArgs.frame_buffer.width,
		gKernelArgs.frame_buffer.height, gKernelArgs.frame_buffer.depth);

	gKernelArgs.frame_buffer.enabled = true;

	// the memory will be identity-mapped already
	video_display_splash(gKernelArgs.frame_buffer.physical_buffer.start);
}


extern "C" void
platform_switch_to_text_mode(void)
{
}


extern "C" status_t
platform_init_video(void)
{
	//TODO: find base in pciconfig
	return B_OK;
}
