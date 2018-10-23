/*
 * Copyright 2018 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef KERNEL_BOOT_PLATFORM_BEBOX_STAGE2_H
#define KERNEL_BOOT_PLATFORM_BEBOX_STAGE2_H

#ifndef KERNEL_BOOT_STAGE2_ARGS_H
#	error This file is included from <boot/stage2_args.h> only
#endif

struct platform_stage2_args {
	/* U-Boot compatible part */
	void *boot_tgz_data;
	uint32 boot_tgz_size;
	void *fdt_data;
	uint32 fdt_size;

	/* BeBox-specific stuff passed by the bootrom */
	uint32 ram_size;
	struct bootitem *bootitems;
};

#endif	/* KERNEL_BOOT_PLATFORM_BEBOX_STAGE2_H */
