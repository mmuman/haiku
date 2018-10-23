/*
 * Copyright 2018, François Revol, revol@free.fr. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef KERNEL_BOOT_PLATFORM_BEBOX_BEBOX_H
#define KERNEL_BOOT_PLATFORM_BEBOX_BEBOX_H

#ifndef _ASSEMBLER
#include <SupportDefs.h>
#endif /* _ASSEMBLER */

/* BeBox boot protocol */
/* cf. http://testou.free.fr/www.beatjapan.org/mirror/www.be.com/aboutbe/benewsletter/Issue27.html#Cookbook */

#define RAMZIZE_ADDR 0x3010	/* RAM size, in bytes */
#define BOOTITEMS_ADDR 0x301C

#ifndef _ASSEMBLER

typedef struct bootitem {
	struct bootitem	*next;	/* -> next item, or null */
	char			*name;	/* -> item name */
	int				size;	/* item size */
	void			*data;	/* -> item data */
} __attribute__((packed)) bootitem;


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* _ASSEMBLER */

#endif	/* KERNEL_BOOT_PLATFORM_BEBOX_BEBOX_H */
