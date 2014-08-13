/*
 * Board specific setup info
 *
 * (C) Copyright 2003, ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <version.h>

#include <cns3000.h>
.global asm_i2c_init
asm_i2c_init:
#if 0
	// Enable I2C Clock
	ldr	r0, =0x77000000
	ldr r1, [r0]
	orr r1, r1, #0x8
	str r1, [r0]

	// Do a Soft Reset on I2C
	ldr r1, [r0, #0x4]
	bic r1, r1, #0x8
	str r1, [r0, #0x4]
	orr r1, r1, #0x8
	str r1, [r0, #0x4]
#endif

	// Disable the Controller
	ldr r0, =0x71000000
	mov r1, #0
	str r1, [r0, #0x20]

	// Disable GPIOB functions on i2c pins
	ldr r0, =0x76000000
	ldr r1, [r0, #0x18]
	orr r1, r1, #0x3000
	str r1, [r0, #0x18]

	// Set I2C Drive Strength
	ldr r1, [r0, #0x20]
	bic r1, r1, #0x300
	str r1, [r0, #0x20]
	orr r1, r1, #0x300
	str r1, [r0, #0x20]

	// Set I2C Timers
	//mov r0, #0x71000000
	ldr r0, =0x71000000
	//mov r1, #0xff
	//orr r1, r1, #0x7600
	//orr r1, r1, #0x10000
	ldr r1, =0x000176ff	
	str r1, [r0, #0x24]
	mov r1, #0x3
	str r1, [r0, #0x3c]

	// Clear Interrupt Status
	ldr r1, [r0, #0x34]
	orr r1, r1, #0x3
	str r1, [r0, #0x34]

	// Enable I2C
	mov r1, #0x80000000
	str r1, [r0, #0x20]

	mov pc, lr


i2c_wait_for_status:

	ldr r0, =0x71000000

1:
	// Check Status Register
	ldr r1, [r0, #0x34]
	ands r1, r1, #0x3
	beq 1b

	// Delay
	mov r2, #0x8000
2:
	subs r2, r2, #1
	bne 2b

	mov r3, #0x1
	cmp r1, #0x2
	bne i2c_wfs_exit
	mov r3, #0

i2c_wfs_exit:
	mov pc, lr


// Device Address in r3, offset in r4, return byte in lsb of r5
.globl i2c_read_byte
i2c_read_byte:
	mov r11, lr
	mov r10, r3
1:
	mov r3, r10
	ldr r0, =0x71000000

	// Enable I2C
	mov r1, #0x80000000
	str r1, [r0, #0x20]

	// Load device address R3
	lsl r3, r3, #1
	str r3, [r0, #0x28]

	// Write then Read Operation
	ldr r1, [r0, #0x20]
	orr r1, r1, #0x20
	str r1, [r0, #0x20]

	// offset in Write Register
	str r4, [r0, #0x2c]
	
	// Clear Interrupt
	ldr r1, [r0, #0x34]
	orr r1, r1, #0x3
	str r1, [r0, #0x34]

	// Issue Start
	ldr r1, [r0, #0x20]
	orr r1, r1, #0x40
	str r1, [r0, #0x20]

	// Return Status in r3, if r3 == 0 it passed
	bl i2c_wait_for_status
	cmp r3, #0
	bne 1b	// Retry

	// Store read byte in R5
	ldr r5, [r0, #0x30]
	and r5, r5, #0xff

	nop
	nop
	nop
	nop
	nop
	nop

	mov r3, r10
	mov lr, r11
	mov pc, lr

.globl secondary_cores
secondary_cores:
	mov	r12, lr

	/* Have we already remapped. */
	ldr	r8, =CNS3000_VEGA_MISC_BASE
	ldr	r6, [r8, #0x00]
	and	r6, r6, #0x01

	tst	r6, #0x01
	beq 2f
	mov pc, #0x10000000

2:
        /* Enable software interrupt */
	ldr	r5, =CNS3000_TC11MP_GIC_CPU_BASE
	mov	r6, #0x01
	str	r6, [r5, #GIC_CPU_CTRL_OFFSET]
	mov	r6, #0xF0
	str	r6, [r5, #GIC_CPU_PRIMASK_OFFSET]

	/* Read core number into r0 */
//	mrc	p15, 0, r0, c0, c0, 5
//	and	r0, r0, #0x0f
1:
	/* Set WFI */
	mov r2, #0
	mcr	p15, 0, r2, c7, c0, 4

	ldr	r5, =CNS3000_VEGA_MISC_BASE
	ldr	r6, [r5, #0x0600]
	ldr	r5, =0x35678855
	cmp	r6, r5
	movne	pc, r6

	b	1b

	mov	lr, r12
	mov	pc, lr

/* Set up the platform, once the cpu has been initialized */
.globl lowlevel_init
lowlevel_init:
	mov	r12, lr

	/* regulator control */
    ldr r0, =CNS3000_VEGA_PMU_BASE
    ldr r1, =0x00469A59
    str r1, [r0, #0x20]

	/* ?? Disable Watchdog */
	/*bl disable_wdt*/

	/* ?? External interrupt pending clear */

	/* ?? Disable all interrupts */

	/* ?? init system clock */
	/* light on GPIOA0 and A1 */
    ldr r0, =CNS3000_VEGA_GPIOA_BASE
    ldr r1, =0x00000203
    str r1, [r0, #0x08]
    ldr r1, =0x00000203
    str r1, [r0, #0x14]

/*	bl	pmu_ctrl_init*/

	/* Initiallise DDR2 SDRAM Control -- Setup pll, mux, memory */
	bl	ddr2_sdram_ctrl_init

	/* Initiallise Static Memory Controller (SMC: NOR flash) */
	bl	smc_ctrl_init

#if defined(CONFIG_SPI) && defined(CONFIG_CNS3000)
	/* Initiallise SPI Control */
	bl	spi_ctrl_init
#endif

	mov	lr, r12
	mov	pc, lr


ddr2_sdram_ctrl_init:
	mov r13, lr

	/* R6 - SDRAM Size (1=32MB, 2=64MB, 3=128MB, 4=256MB, 5=512MB) */
  mov r3, #0x51   // EEPROM Device Address
  mov r4, #0x2b // EEPROM Offset
  bl i2c_read_byte
	mov r6, r5

	/* R5 - SDRAM width (1=16bit 2=32bit=2x16bit parts) */
  mov r3, #0x51   // EEPROM Device Address
  mov r4, #0x2d // EEPROM Offset
  bl i2c_read_byte

	/* */
/*
	ldr r0, =0x77000004
	mov r1, #0xFFFFFFFD
	str r1, [r0]

	nop
	nop
	nop
	nop
	nop
	nop

	ldr r0, =0x77000004
	mov r1, #0xFFFFFFFF
	str r1, [r0]
*/

	/* SDRAM control register base address 0x72000000 */
	ldr	r0, =CNS3000_VEGA_DMC_BASE

	/* DLL initialization */
	ldr r1, =0x00000000
	str r1, [r0, #0x400]

	ldr r1, =0x00001410
	str r1, [r0, #0x408]

	ldr r1, =0x00000006
	str r1, [r0, #0x40C]

	ldr r1, =0x00000000
	str r1, [r0, #0x410]

	ldr r1, =0x00000000
	str r1, [r0, #0x414]

	ldr r1, =0x00000000
	str r1, [r0, #0x418]

	ldr r1, =0x00000000
	str r1, [r0, #0x41C]

	ldr r1, =0x00000000
	str r1, [r0, #0x420]

	ldr r1, =0x00000018
	str r1, [r0, #0x424]

	ldr r1, =0x00000001
	str r1, [r0, #0x404]

	ldr r1, =0x00000003
	str r1, [r0, #0x404]

	/* ZQ initialization */
	ldr r1, =0x0006E550
	str r1, [r0, #0x428]

	ldr r1, =0x0006E551
	str r1, [r0, #0x428]

	/*  TODO: wait 1 sec for InitCmp bit (bit-1 of 0x7200042C) to become 1 */

	nop
	nop
	nop
	nop
	nop
	nop

	ldr r1, =0x0006E550
	str r1, [r0, #0x428]

	ldr r1, =0x0006E552
	str r1, [r0, #0x428]

	ldr r1, =0x0006E550
	str r1, [r0, #0x428]

	ldr r1, =0x0007E550
	str r1, [r0, #0x428]

	ldr r1, =0x00000002
	str r1, [r0, #0x430]

	/* Set CAS Latency = 4 * mclk */
	ldr	r1, =0x0000000C
	str r1, [r0, #DMC_CAS_LATENCY_OFFSET]   /* 0x014 */

	/* Memory Size/Width config */
	/* R6 - SDRAM Size (1=32MB, 2=64MB, 3=128MB, 4=256MB, 5=512MB) */
	/* R5 - SDRAM width (1=16bit 2=32bit=2x16bit parts) */

	mov r1, #0x4 /* default: dqm_init, 16-bit, bank-bits=2 (4banks) */
	cmp r5, #0x2        /* 32bit */
	beq	ddr_width_32
	cmp r5, #0x20       /* 32bit (previous config value for 32bit) */
	beq	ddr_width_32

ddr_width_16:
	/* bank_bits config */
	cmp r6, #0x3        /* 1x128MB part */
	orreq r1, r1, #0x30 /* bank_bits: 3 (8banks) */
	cmp r6, #0x4        /* 1x256MB part */
	orreq r1, r1, #0x30 /* bank_bits: 3 (8banks) */
	b ddr_width_end

ddr_width_32:
	orreq r1, r1, #0x40 /* memory_width: 32bit (2x16bit parts) */
	/* bank_bits config */
	cmp r6, #0x4        /* 2x128MB parts = 256MB */
	orreq r1, r1, #0x30 /* bank_bits: 3 (8banks) */
	cmp r6, #0x5        /* 2x256MB parts = 512MB */
	orreq r1, r1, #0x30 /* bank_bits: 3 (8banks) */

ddr_width_end:
	str r1, [r0, #DMC_MEMORY_CFG2_OFFSET]   /* 0x04C */

        /* Set request memory for LCD high priority */
	ldr	r1, =0x00000400
	str r1, [r0, #DMC_USER_CONFIG0_OFFSET]   /* 0x304 */

	/* Set t_mrd = 2 * mclk */
	ldr	r1, =0x00000002
	str r1, [r0, #DMC_T_MRD_OFFSET]     /* 0x01C */

	mov r1, r6
	ldr	r5, =CNS3000_VEGA_MISC_BASE     /* 0x7600_0000 MISC */
	ldr	r6, [r5, #0x04]                 /* 0x7600_0004 CHIP_CFG */
	and	r6, r6, #0x600                  /* Bit 10:9 DDR2 Clock freq select */
	ldr	r5, =0x0                        /* 200Mhz */
	cmp r5, r6
	beq	ddr_200	
	ldr	r5, =0x200                      /* 266MHz */	
	cmp r5, r6
	beq	ddr_266	
	ldr	r5, =0x400                      /* 333MHz */
	cmp r5, r6
	beq	ddr_333	
	ldr	r5, =0x600                      /* 400MHz */
	cmp r5, r6
	beq	ddr_400	

ddr_200:
	mov r6, r1
	/* Set t_ras = 13 * mclk */
	ldr	r1, =0x00000009
	str r1, [r0, #DMC_T_RAS_OFFSET]     /* 0x020 */

	/* Set t_rc = 18 * mclk */
	ldr	r1, =0x0000000D
	str r1, [r0, #DMC_T_RC_OFFSET]      /* 0x024 */

	/* Set t_rcd = 5 * mclk */
	ldr	r1, =0x00000204
	str r1, [r0, #DMC_T_RCD_OFFSET]     /* 0x028 */

	/* Set t_rfc = 38 * mclk */
	ldr	r1, =0x00002628
	str r1, [r0, #DMC_T_RFC_OFFSET]     /* 0x02C */

	/* Set t_rp = 5 * mclk */
	ldr	r1, =0x00000204
	str r1, [r0, #DMC_T_RP_OFFSET]      /* 0x030 */

	/* Set t_rrd = 3 * mclk */
	ldr	r1, =0x00000002
	str r1, [r0, #DMC_T_RRD_OFFSET]     /* 0x034 */

	/* Set t_wr = 5 * mclk */
	ldr	r1, =0x00000003
	str r1, [r0, #DMC_T_WR_OFFSET]      /* 0x038 */

	/* Set t_wtr = 2 * mclk */
	ldr	r1, =0x00000002
	str r1, [r0, #DMC_T_WTR_OFFSET]     /* 0x03C */

	ldr r1, =0x0000080A
	str r1, [r0, #0x054]            /* */
	b ddr_clock_end

ddr_266:
	/* Set t_ras = 13 * mclk */
	ldr	r1, =0x0000000C
	str r1, [r0, #DMC_T_RAS_OFFSET]     /* 0x020 */

	/* Set t_rc = 18 * mclk */
	ldr	r1, =0x00000010
	str r1, [r0, #DMC_T_RC_OFFSET]      /* 0x024 */

	/* Set t_rcd = 5 * mclk */
	ldr	r1, =0x00000204
	str r1, [r0, #DMC_T_RCD_OFFSET]     /* 0x028 */

	/* Set t_rfc = 38 * mclk */
	ldr	r1, =0x00003335
	str r1, [r0, #DMC_T_RFC_OFFSET]     /* 0x02C */

	/* Set t_rp = 5 * mclk */
	ldr	r1, =0x00000204
	str r1, [r0, #DMC_T_RP_OFFSET]      /* 0x030 */

	/* Set t_rrd = 3 * mclk */
	ldr	r1, =0x00000003
	str r1, [r0, #DMC_T_RRD_OFFSET]     /* 0x034 */

	/* Set t_wr = 5 * mclk */
	ldr	r1, =0x00000004
	str r1, [r0, #DMC_T_WR_OFFSET]      /* 0x038 */

	/* Set t_wtr = 2 * mclk */
	ldr	r1, =0x00000002
	str r1, [r0, #DMC_T_WTR_OFFSET]     /* 0x03C */

	ldr r1, =0x00000C0E
	str r1, [r0, #0x054]            /* */
	b ddr_clock_end

ddr_333:
	mov r6, r1
	/* Set t_ras = 13 * mclk */
	ldr	r1, =0x0000000E
	str r1, [r0, #DMC_T_RAS_OFFSET]     /* 0x020 */

	/* Set t_rc = 18 * mclk */
	ldr	r1, =0x00000013
	str r1, [r0, #DMC_T_RC_OFFSET]      /* 0x024 */

	/* Set t_rcd = 5 * mclk */
	ldr	r1, =0x00000305
	str r1, [r0, #DMC_T_RCD_OFFSET]     /* 0x028 */

	/* Set t_rfc = 38 * mclk */
	ldr	r1, =0x00003A3C
	str r1, [r0, #DMC_T_RFC_OFFSET]     /* 0x02C */

	/* Set t_rp = 5 * mclk */
	ldr	r1, =0x00000305
	str r1, [r0, #DMC_T_RP_OFFSET]      /* 0x030 */

	/* Set t_rrd = 3 * mclk */
	ldr	r1, =0x00000004
	str r1, [r0, #DMC_T_RRD_OFFSET]     /* 0x034 */

	/* Set t_wr = 5 * mclk */
	ldr	r1, =0x00000005
	str r1, [r0, #DMC_T_WR_OFFSET]      /* 0x038 */

	/* Set t_wtr = 2 * mclk */
	ldr	r1, =0x00000003
	str r1, [r0, #DMC_T_WTR_OFFSET]     /* 0x03C */

	ldr r1, =0x00000E10
	str r1, [r0, #0x054]            /* */
	b ddr_clock_end

ddr_400:
	mov r6, r1

	/* Set t_ras = 13 * mclk */
	ldr	r1, =0x00000012 
	str r1, [r0, #DMC_T_RAS_OFFSET]     /* 0x020 */

	/* Set t_rc = 18 * mclk */
	ldr	r1, =0x00000018
	str r1, [r0, #DMC_T_RC_OFFSET]      /* 0x024 */

	/* Set t_rcd = 5 * mclk */
	ldr	r1, =0x00000406
	str r1, [r0, #DMC_T_RCD_OFFSET]     /* 0x028 */

	/* Set t_rfc = 38 * mclk */
	ldr	r1, =0x00004D4F
	str r1, [r0, #DMC_T_RFC_OFFSET]     /* 0x02C */

	/* Set t_rp = 5 * mclk */
	ldr	r1, =0x00000406
	str r1, [r0, #DMC_T_RP_OFFSET]      /* 0x030 */

	/* Set t_rrd = 3 * mclk */
	ldr	r1, =0x00000004
	str r1, [r0, #DMC_T_RRD_OFFSET]     /* 0x034 */

	/* Set t_wr = 5 * mclk */
	ldr	r1, =0x00000006
	str r1, [r0, #DMC_T_WR_OFFSET]      /* 0x038 */

	/* Set t_wtr = 2 * mclk */
	ldr	r1, =0x00000003
	str r1, [r0, #DMC_T_WTR_OFFSET]     /* 0x03C */

	ldr r1, =0x00001220
	str r1, [r0, #0x054]            /* */

ddr_clock_end:
	/* Set t_xp = 2 * mclk */
	ldr	r1, =0x00000003
	str r1, [r0, #DMC_T_XP_OFFSET]      /* 0x040 */

	/* Set t_xsr = 41 * mclk */
	ldr	r1, =0x000000C8
	str r1, [r0, #DMC_T_XSR_OFFSET]     /* 0x044 */

	/* Set t_esr = 200 * mclk */
	ldr	r1, =0x000000C8
	str r1, [r0, #DMC_T_ESR_OFFSET]     /* 0x048 */

	/* Set burst, row and col size */
	/* R6 - SDRAM Size (1=32MB, 2=64MB, 3=128MB, 4=256MB, 5=512MB) */
	cmp r6, #0x5                    /* 512MB (2x256MB part used is 14bit) */
	ldreq r1, =0x0001401A           /* Burst 4, 10bit cols, 14bit rows */
	ldrne r1, =0x00014012           /* Burst 4, 10bit cols, 13bit rows */
	str r1, [r0, #DMC_MEMORY_CFG_OFFSET]  /* 0x00C memory_cfg*/

	/* Set memory refresh period */
	ldr r1, =0x00000618
	str r1, [r0, #DMC_REFRESH_PRD_OFFSET]           /* 0x010 bit14:0 refresh_prd in clock cycles */

	/* set chip decode config */
	ldr r1, =0x000000FF
	str r1, [r0, #DMC_CHIP_0_CFG_OFFSET]            /* 0x200 */

	/* direct memory commands */
	ldr r1, =0x000C0000
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]            /* 0x008 */

	ldr r1, =0x00000000
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	ldr r1, =0x000A0000
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	ldr r1, =0x000B0000
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	ldr r1, =0x00090004
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	ldr r1, =0x00080B62
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	ldr r1, =0x00000000
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]
	
	ldr r1, =0x00040000
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	ldr r1, =0x00040000
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	ldr r1, =0x00080A62
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	ldr r1, =0x00090384
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	ldr r1, =0x00090004
	str r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	ldr r1, =0x00000000
	str r1, [r0, #DMC_MEMC_CMD_OFFSET]


	/* Set t_faw = 15 * mclk */
//	ldr	r1, =0x0000080A
//	str	r1, [r0, #DMC_T_FAW_OFFSET]

	/* Set Memory Configuration Register: 1Gb, 16x2 ?? */
//	ldr	r1, =0x00014012
//	str	r1, [r0, #DMC_MEMORY_CFG_OFFSET]

	/* Set Refresh Period Register: 1560 * mclk */
//	ldr	r1, =0x00000938
//	str	r1, [r0, #DMC_REFRESH_PRD_OFFSET]

#ifdef PHYS_SDRAM_32BIT
	/* Set 32-bit physical memory width, 3-bit for bank address, DDR32, Async */
//	ldr	r1, =0x00000074
//	str	r1, [r0, #DMC_MEMORY_CFG2_OFFSET]
#else
	/* Set 16-bit physical memory width, 3-bit for bank address, DDR16, Async */
//	ldr	r1, =0x00000034
//	str	r1, [r0, #DMC_MEMORY_CFG2_OFFSET]
#endif

	/* Select the memory organization as decoded from the AXI address: Row, Bank, Column */
//	ldr	r1, =0x000000FF
//	str	r1, [r0, #DMC_CHIP_0_CFG_OFFSET]

	/* Disable DQSN */
//	ldr	r1, =0x00000004
//	str	r1, [r0, #DMC_USER_CONFIG1_OFFSET]

	/* Set Direct Command Register: NOP */
//	ldr	r1, =0x000C0000
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Set Direct Command Register: Pre-charge all */
//	ldr	r1, =0x00000000
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Set Direct Command Register: Extended Mode Register 2 */
//	ldr	r1, =0x000A0000
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Set Direct Command Register: Extended Mode Register 3 */
//	ldr	r1, =0x000B0000
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Extended Mode Register 1, DLL enable/AL = 0 */
//	ldr	r1, =0x00090044
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Mode Register/BL = 4, DLL reset/CASL = 4, WR = 2 */
//	ldr	r1, =0x00080342
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Pre-charge all */
//	ldr	r1, =0x00000000
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Auto-refresh */
//	ldr	r1, =0x00040000
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Auto-refresh */
//	ldr	r1, =0x00040000
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Mode Register/BL = 4, Disable DLL reset/CASL = 4, WR = 2 */
//	ldr	r1, =0x00080242
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Extended Mode Register 1 / OCD default */
//	ldr	r1, =0x000903C4
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* Extended Mode Register 1 / OCD exit */
//	ldr	r1, =0x00090044
//	str	r1, [r0, #DMC_DIRECT_CMD_OFFSET]

	/* */
//	ldr	r1, =0x00000000
//	str	r1, [r0, #DMC_MEMC_CMD_OFFSET]

	/* DMC base address */
	mov lr, r13
	mov	pc, lr


smc_ctrl_init:
	/* SMC control register base address 0x73000000 */
	ldr	r0, =CNS3000_VEGA_SMC_BASE

	/* set cycles for CS0 NOR FLASH */
	/* ldr r1, [r0, #SMC_SRAM_CYCLES0_0_OFFSET] */
	ldr	r1, =0x001fffff					// HACKITY HACK HACK
	str	r1, [r0, #SMC_SET_CYCLES_OFFSET]

	/* set opmode bit, 16-bits width */
	ldr	r1, =0x00000AA9
	str	r1, [r0, #SMC_SET_OPMODE_OFFSET]

	/* an idle cycle occurs after each burst */
	ldr	r1, =0x00000001
	str	r1, [r0, #SMC_REFRESH_PERIOD_0_OFFSET]

	/* select CS0 */
	ldr	r1, =0x00400000
	str	r1, [r0, #SMC_DIRECT_CMD_OFFSET]

	mov	pc, lr

#if defined(CONFIG_SPI) && defined(CONFIG_CNS3000)
spi_ctrl_init:
	/* SPI control register base address 0x71000000 */
	ldr	r0, =CNS3000_VEGA_SSP_BASE

	/* enable SPI high speed read for system boot up */
        ldr     r1, [r0, #SPI_CFG_OFFSET]
        mov     r2, #0x40000000
        orr     r1, r1, r2
        str     r1, [r0, #SPI_CFG_OFFSET]

        /* set bit rate */
        mov     r1, #0x01
        str     r1, [r0, #SPI_BIT_RATE_OFFSET]

	mov	pc, lr
#endif

disable_wdt:
	/* PMU control register base address 0x00000600 */
	ldr	r0, =CNS3000_VEGA_TIMER_AND_WDT_BASE
	ldr	r1, =0x00000000				/* disable global watchdog */
	str	r1, [r0, #WDT_CTRL_OFFSET]
	mov	pc, lr
	
	


pmu_ctrl_init:
	/* PMU control register base address 0x77000000 */
	ldr	r0, =CNS3000_VEGA_PMU_BASE
	/* enable UART0, UART1, switch, and SDIO clock */
	ldr	r1, =0x0200098f			
	str	r1, [r0, #PM_CLK_GATE_OFFSET]
	/* reset UART0, UART1, switch, and SDIO clock */
/*
	ldr	r1, =0x0000000f		
	str	r1, [r0, #PM_SOFT_RST_OFFSET]
	ldr	r1, =0x0200098f	
	str	r1, [r0, #PM_SOFT_RST_OFFSET]
*/
	mov	pc, lr

