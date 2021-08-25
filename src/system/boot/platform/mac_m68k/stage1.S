/*
 * Copyright 2020, François Revol, revol@free.fr. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

#include <asm_defs.h>

/*
 * stage1 boot code for Mac 68k for use as boot block of HDD partitions.
 * cf.
 * https://web.archive.org/web/20170110011921/http://jonsharp.net/retrocomputing/bare-metal-macintosh-programming-part-1/
 * https://web.archive.org/web/20170110055707/http://jonsharp.net/retrocomputing/bare-metal-macintosh-programming-part-2/
 * 
 */

//#include "romcalls.h"

// 1 enabled verbose output
//#define DEBUG 1

.equ stage1_size, 1024

.equ ScrnBase, 0x0824

.macro PSTRING s
pstring_begin_\@:
	.byte pstring_end_\@ - pstring_string_\@ - 1
pstring_string_\@:
	.string "\s"
pstring_end_\@:
	.fill 16 - (pstring_end_\@ - pstring_begin_\@), 1, 0
.endm

/*
#define	BOOT_BLOCK_START_ADDRESS	0x7c00
#define	STACK_ADDRESS				BOOT_BLOCK_START_ADDRESS
#define	READ_BUFFER_STACK			STACK_ADDRESS - 0x2000
#define	PARTITION_OFFSET_OFFSET		506
#define	BFS_SUPERBLOCK_OFFSET		512
*/

// BFS definitions

#define SUPER_BLOCK_MAGIC1			'1SFB'		; nasm reverses '...' consts
#define SUPER_BLOCK_MAGIC2			0xdd121031
#define SUPER_BLOCK_MAGIC3			0x15b6830e

#define	INODE_MAGIC1				0x3bbe0ad9

#define	NUM_DIRECT_BLOCKS			12

#define	S_IFMT						00000170000o
#define	S_IFDIR						00000040000o



boot_block:
	.short	0x4C4B	// signature
Entry:
	bra	floppy_start
	.short 0x4418	// version
	.short 0x00
	// Some not very useful filenames. XXX: we could fit code in there maybe
	PSTRING "Haiku"
	PSTRING "Haiku"
	PSTRING "Haiku"
	PSTRING "Haiku"
	PSTRING "Haiku"
	PSTRING "Haiku"
	PSTRING "Haiku"
	.short	10
	.short	20
	// heap sizes
	.long	0x00004300
	.long	0x00008000
	.long	0x00020000

FUNCTION(floppy_start):

// TODO: display a leaf folder icon?
	movel	(ScrnBase), %a0
/* clear screen */
	movel	#0, %d0
	movel	#5472, %d1
clear_loop:
	movel	%d0, (%a0)+
	subi	#1, %d1
	bne	clear_loop

loop:
	bra loop
FUNCTION_END(floppy_start)


#if 0 // XXX: ATARI stuff below

// NOTE: normal programs (.prg) run as user mode, 
// while boot sector is chained in supervisor mode.
// this means using Super(SUP_INQUIRE) we can know 
// from the same entry point if we were run from boot code or prg.


	//Pterm0
	//move.w	#1,%d0
	//trap	#1
	//rts
	
	lea.l	str,%a0
.loopt:
	move.b	(%a0)+,%d0
	beq	.strout
	bsr	putc
	bra	.loopt
.strout:
	//Pterm0
	move.w	#1,%d0
	trap	#1

	rts
	
.loop:
	move	#'.',%d0
	bsr		putc
	bra		.loop
	rts

/* prints the char in d0.b to the console */
putc:
	movem.l	%a0,-(%sp)
	move.w	%d0,-(%sp)
	move.w	#DEV_CON,-(%sp)	// DEV_CON
	move.w	#3,-(%sp)	// Bconout
	trap	#13
	add.l	#6,%sp
	movem.l	(%sp)+,%a0
	rts
str:
	.ascii "Haiku!"
	.byte 0
#endif
