/*
 *  armboot - Startup Code for unknown CPU-core
 *
 *  Copyright (c) 2014  Gateworks Corporation 
 *  Copyright (c) 2003  Texas Instruments
 *
 *  ----- Adapted for OMAP1610 OMAP730 from ARM925t code ------
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <gj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */


#include <config.h>
#include <version.h>

#include <cns3000.h>
#include <linux/linkage.h>

/*
 * the actual start code
 */
.globl reset
reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0, cpsr
	bic	r0, r0,#0x1f
	orr	r0, r0,#0xd3
	msr	cpsr, r0


	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
	bl cpu_init_crit
	bl _main

  .globl  c_runtime_cpu_setup
c_runtime_cpu_setup:

  bx  lr

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */
cpu_init_crit:
	/*
	 * To run direct from reset some importent
	 * initialisation code must be added here
	 */
	/* Ensure that the MMU and caches are off */
	/* ?? flush v4 I/D caches */

	mov     r0, #0
	mcr p15, 0, r0, c7, c7, 0 /* invalidate both caches */
	mcr p15, 0, r0, c7, c10, 4 /* Data sync barrier */

	/* ?? disable MMU stuff and caches */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * Handle secondary mpcores
	 * CPU1 enter dormant mode
	 */

	/* Read core number into r0 */
	mrc	p15, 0, r0, c0, c0, 5
	and	r0, r0, #0x0f

	cmp	r0, #0x00
	bne	secondary_cores

	mov r8, lr
	bl asm_i2c_init

	/*
	 * Initialise memory
	 * Setup memory and board specific bits prior to relocation.
	 */
	bl	lowlevel_init


	/* copy */
	ldr r0, =__image_copy_start
	ldr r1, =_end
	mov r2, #0x20000000
cavium_copy_loop:
	ldr r3, [r0], #4
	str r3, [r2], #4
	cmp r0, r1
	blo cavium_copy_loop

	/* Re-map (alias memory to 0x00000000) */
	ldr	r0, =0x76000000
	mov	r1, #1
	str	r1, [r0]

	/* Invalidate both caches to clean up */
	mov     r0, #0
	mcr p15, 0, r0, c7, c7, 0 /* invalidate both caches */
	mcr p15, 0, r0, c7, c10, 4 /* Data sync barrier */

	/* enable all clocks */
	ldr r0, =0x7700001C
	mov r1, #0x00000000
	str r1, [r0]

/* configure periperhal support from eeprom 0x51: 4bytes at offset 0x4a */
#if 1
	mov r3, #0x51		// EEPROM Device Address
	mov r4, #0x4A		// EEPROM Offset for peripheral config

	bl i2c_read_byte	// returns val in r5
	mov r6, r5
	mov r3, #0x51		// EEPROM Device Address
	add r4, r4, #1
	bl i2c_read_byte
	orr r6, r6, r5, lsl #8
	mov r3, #0x51		// EEPROM Device Address
	add r4, r4, #1
	bl i2c_read_byte
	orr r6, r6, r5, lsl #16
	mov r3, #0x51		// EEPROM Device Address
	add r4, r4, #1
	bl i2c_read_byte
	orr r6, r6, r5, lsl #24
#else
	ldr r6, =0xDC0347DF
#endif

	// CLK_GATE: enable clocks for peripheral blocks
	ldr r0, =0x77000000
	str r6, [r0]

	orr r6, r6, #0x30000000 // CPU0/CPU1 active must be set

	// SOFT_RST: take peripheral blocks out of reset
	ldr r0, =0x77000004
	str r6, [r0]

	mov lr, r8
	mov pc, lr

.globl reset_cpu
reset_cpu:
        /* Global Software Reset */
	ldr	r0, =0x77000004
	mov	r1, #0
	str	r1, [r0]

_loop_forever:
        b       _loop_forever

