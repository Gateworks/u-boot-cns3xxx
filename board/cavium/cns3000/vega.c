/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
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
/*
 * The RealView Emulation BaseBoard provides timers and soft reset
 * - the cpu code does not need to provide these.
 */
#include <common.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong timestamp;
static ulong lastdec;
uint8_t model[16];
uint8_t rev;
int sp;

#define READ_TIMER1 (*(volatile ulong *)(CFG_TIMERBASE))
#define READ_TIMER2 (*(volatile ulong *)(CFG_TIMERBASE + 0x10))

static void timer_init(void);

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
		"subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}


#define IO_WRITE(addr, val) (*(volatile unsigned int *)(addr) = (val))
#define IO_READ(addr) (*(volatile unsigned int *)(addr))
#define PCIE_CLK_GATE_REG 0x77000014
#define MISC_PCIE0_REG 0x76000900
#define MISC_PCIE1_REG 0x76000A00
#define PCIE0_CTRL 0x7600095C
#define PCIE1_CTRL 0x76000A5C
#define PMU_SOFT_RST   0x77000004
#define PCIE0_PHY_ERRATA 0x76000940
#define PCIE1_PHY_ERRATA 0x76000A40
#define GPIOA_DIR 0x74000008
#define GPIOA_OUT 0x74000000
#define GPIOB_DIR 0x74800008
#define GPIOB_IN  0x74800004
#define GPIOB_OUT 0x74800000

static void pcie_init(void)
{
	unsigned char eeprom;
	unsigned int temp;
	unsigned char external_clkgen;
	int gpio_perst = -1;

	/* Errata C-01 */
	IO_WRITE(PCIE0_PHY_ERRATA, 0xe2c);
	IO_WRITE(PCIE1_PHY_ERRATA, 0xe2c);

#ifdef GPIOA_PERST
	gpio_perst = GPIOA_PERST;
	if (strncmp(model, "GW2391", 6) == 0) {
		gpio_perst = 8;
	}
#endif

	/**
	 * All boards with external PCI clockgen have GPIOB26 high at powerup.
	 * However, exceptions exist with the GW2386 after a certain revision
	 * and special number, the GW2391, and GW2393.
	 */
	if (strncmp(model, "GW2386", 6) == 0) {
		if ((sp == 217) && (rev >= 'E'))
			external_clkgen = 1;
		else if ((sp == 0) && (rev == 'D'))
			external_clkgen = 1;
		else
			external_clkgen = 0;
	}
	else if (strncmp(model, "GW2391", 6) == 0) {
		if (rev >= 'C')
			external_clkgen = 1;
		else
			external_clkgen = 0;
	}
	else if (strncmp(model, "GW2393", 6) == 0)
		external_clkgen = 1;
	else
		external_clkgen = (IO_READ(GPIOB_IN) & (1 << 26)) ? 1 : 0;

	printf("PCI:   PERST:GPIOA%d clock:%s\n", gpio_perst,
	       external_clkgen ? "external" : "internal");

	/* enable and assert PERST# */
	if (gpio_perst != -1) {
		temp = IO_READ(GPIOA_DIR);
		temp |= (1 << gpio_perst);  /* output */
		IO_WRITE(GPIOA_DIR, temp);

		temp = IO_READ(GPIOA_OUT);
		temp &= ~(1 << gpio_perst); /* low */
		IO_WRITE(GPIOA_OUT, temp);
	}

	i2c_read(0x51, 0x43, 1, &eeprom, 1);

	/* pcie0 init */
	if (eeprom & 0x2) {
		if (external_clkgen) {
			/* select external clock */
			temp = IO_READ(MISC_PCIE0_REG);
			temp &= ~(1 << 11);
			IO_WRITE(MISC_PCIE0_REG, temp);
		} else {
			/* enable PCIe Ref clock 0 */
			temp = IO_READ(PCIE_CLK_GATE_REG);
			temp |= (1 << 28);
			IO_WRITE(PCIE_CLK_GATE_REG, temp);
			/* select internal clock */
			temp = IO_READ(MISC_PCIE0_REG);
			temp |= (1 << 11);
			IO_WRITE(MISC_PCIE0_REG, temp);
		}

		/* soft reset PCIe0 */
		temp = IO_READ(PMU_SOFT_RST);
		temp &= ~(1 << 17);
		IO_WRITE(PMU_SOFT_RST, temp);
		temp |= (1 << 17);
		IO_WRITE(PMU_SOFT_RST, temp);
		/* exit L1 and enable LTSSM */
		temp = IO_READ(PCIE0_CTRL);
		temp |= 0x3;
		IO_WRITE(PCIE0_CTRL, temp);
	}

	/* pcie1 init */
	if (eeprom & 0x4) {
		if (external_clkgen) {
			/* select external clock */
			temp = IO_READ(MISC_PCIE1_REG);
			temp &= ~(1 << 11);
			IO_WRITE(MISC_PCIE1_REG, temp);
		} else {
			/* enable PCIe Ref clock 1 */
			temp = IO_READ(PCIE_CLK_GATE_REG);
			temp |= (1 << 29);
			IO_WRITE(PCIE_CLK_GATE_REG, temp);
			/* select internal clock */
			temp = IO_READ(MISC_PCIE1_REG);
			temp |= (1 << 11);
			IO_WRITE(MISC_PCIE1_REG, temp);
		}

		/* soft reset PCIe1 */
		temp = IO_READ(PMU_SOFT_RST);
		temp &= ~(1 << 18);
		IO_WRITE(PMU_SOFT_RST, temp);
		temp |= (1 << 18);
		IO_WRITE(PMU_SOFT_RST, temp);
		/* exit L1 and enable LTSSM */
		temp = IO_READ(PCIE1_CTRL);
		temp |= 0x3;
		IO_WRITE(PCIE1_CTRL, temp);
	}

	/* de-assert PERST# after some delay for clock to become stable */
	if (gpio_perst != -1) {
		udelay(1000);
		temp = IO_READ(GPIOA_OUT);
		temp |= (1 << gpio_perst); /* high */
		IO_WRITE(GPIOA_OUT, temp);
	}
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	gd->bd->bi_arch_number = 2635;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x00000100;

	gd->flags = 0;

	icache_enable ();
	timer_init();
#ifdef CONFIG_AHCI_CNS3000
	//dram_init();
	scsi_init();
#endif
	return 0;
}


int misc_init_r (void)
{
	int i;
	uint8_t eeprom_enetaddr[6];
	uint8_t env_enetaddr[6];
	uint32_t serial = 0;
	uint8_t date[4];

	char ethaddr[20];

	char *tmp = getenv("ethaddr");
	char *tmp1 = getenv("eth1addr");
	char *tmp2 = getenv("eth2addr");
	char *end;

	i2c_read(0x51, 0x20, 1, date, 4);
	i2c_read(0x51, 0x18, 1, eeprom_enetaddr, 4);
	serial |= ((eeprom_enetaddr[0]) | (eeprom_enetaddr[1] << 8) |
	          (eeprom_enetaddr[2] << 16) | (eeprom_enetaddr[3] << 24));


	printf("Gateworks Corporation Copyright 2010\n");
	printf("Model Number: %s\n", model);
	printf("Manufacturer Date: %02x-%02x-%02x%02x\n", date[0], date[1],
	       date[2], date[3]);
	printf("Serial #: %i\n", serial);	

	i2c_read(0x51, 0x0, 1, eeprom_enetaddr, 6);

	for (i = 0; i < 6; i++) {
		env_enetaddr[i] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
		if (tmp)
			tmp = (*end) ? end+1 : end;
	}

	if (!tmp) {
		sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
			eeprom_enetaddr[0], eeprom_enetaddr[1],
			eeprom_enetaddr[2], eeprom_enetaddr[3],
			eeprom_enetaddr[4], eeprom_enetaddr[5]) ;	
		printf("### Setting environment from ROM MAC address = \"%s\"\n",
			ethaddr);
		setenv("ethaddr", ethaddr);
	}

	i2c_read(0x51, 0x6, 1, eeprom_enetaddr, 6);

	for (i = 0; i < 6; i++) {
		env_enetaddr[i] = tmp1 ? simple_strtoul(tmp1, &end, 16) : 0;
		if (tmp1)
			tmp1 = (*end) ? end+1 : end;
	}

	if (!tmp1) {
		sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
			eeprom_enetaddr[0], eeprom_enetaddr[1],
			eeprom_enetaddr[2], eeprom_enetaddr[3],
			eeprom_enetaddr[4], eeprom_enetaddr[5]) ;	
		printf("### Setting environment from ROM MAC address = \"%s\"\n",
			ethaddr);
		setenv("eth1addr", ethaddr);
	}

	i2c_read(0x51, 0xc, 1, eeprom_enetaddr, 6);

	for (i = 0; i < 6; i++) {
		env_enetaddr[i] = tmp2 ? simple_strtoul(tmp2, &end, 16) : 0;
		if (tmp2)
			tmp2 = (*end) ? end+1 : end;
	}

	if (!tmp2) {
		sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
			eeprom_enetaddr[0], eeprom_enetaddr[1],
			eeprom_enetaddr[2], eeprom_enetaddr[3],
			eeprom_enetaddr[4], eeprom_enetaddr[5]) ;	
		printf("### Setting environment from ROM MAC address = \"%s\"\n",
			ethaddr);
		setenv("eth2addr", ethaddr);
	}

	return (0);
}

/*************************************************************
 Routine:checkboard
 Description: Check Board Identity
*************************************************************/
int checkboard(void)
{
	int i;

	/* determine board model */
	i2c_read(0x51, 0x30, 1, model, 16);

	/* determine board revision */
	rev = '\0';
	for (i = sizeof(model) - 1; i >= 0; i--) {
		if (model[i] >= 'A') {
			rev = model[i];
			break;
		}
	}

	/* determine board sp number (assume 3 digit special number) */
	sp = 0;
	for (i = sizeof(model) - 4; i > 0; i--) {
		if (model[i - 1] == 'S' && model[i] == 'P') {
			/* Convert SPXXX to numeric */
			sp = (sp * 10) + (model[i+1] - '0');
			sp = (sp * 10) + (model[i+2] - '0');
			sp = (sp * 10) + (model[i+3] - '0');
			break;
		}
	}

	pcie_init();
	return (0);
}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	unsigned char eeprom;
	unsigned int temp = 0x01000000;
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;

	i2c_read(0x51, 0x2b, 1, &eeprom, 1);
	gd->bd->bi_dram[0].size = temp << eeprom;

	return 0;
}

/*
 * Start the timer
 * U-Boot expects a 32 bit timer, running at CFG_HZ == 1000
 */
#include "cns3000.h"

#define PMU_REG_VALUE(addr) (*((volatile unsigned int *)(CNS3000_VEGA_PMU_BASE+addr)))
#define CLK_GATE_REG PMU_REG_VALUE(0) 
#define SOFT_RST_REG PMU_REG_VALUE(4) 
#define HS_REG PMU_REG_VALUE(8) 

static void timer_init(void)
{
	// Setup timer to be a 1khz timer to make things easy
	CLK_GATE_REG |= (1 << 14);
	SOFT_RST_REG &= (~(1 << 14));
	SOFT_RST_REG |= (1 << 14);
	//HS_REG |= (1 << 14);
	/*
	 * Now setup timer1
	 */	
	*(volatile ulong *)(CFG_TIMERBASE + 0x00) = CFG_TIMER_RELOAD;
	*(volatile ulong *)(CFG_TIMERBASE + 0x04) = CFG_TIMER_RELOAD;
	*(volatile ulong *)(CFG_TIMERBASE + 0x30) |= 0x0203;	/* Enabled,
								 * down counter,
								 * no interrupt,
								 * 32-bit,
								 */

	reset_timer_masked();
}

int interrupt_init (void){
	return 0;
}

// udelay, exactly what it says, delay 1us
void udelay (unsigned long usec)
{

	/*
	 *  This could possibly be a problem due to overflow
	 *  If usec is greater than 57266230 then we would have
	 *  an overflow and it wouldn't work right
	 */

	*(volatile ulong *)(CFG_TIMERBASE + 0x30) &= ~0x8;	/* Disable */
	*(volatile ulong *)(CFG_TIMERBASE + 0x10) = 75000000 / 1000000 * usec;
	*(volatile ulong *)(CFG_TIMERBASE + 0x14) = 0;
	*(volatile ulong *)(CFG_TIMERBASE + 0x30) |= 0x0408;	/* Enabled, */

	while (READ_TIMER2 != 0);
}

// timestamp ticks at 1ms
ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

void reset_timer_masked (void)
{
	/* reset time */
	lastdec = READ_TIMER1;
	timestamp = 0;	       	    /* start "advancing" time stamp from 0 */
}

/* ASSUMES 1kHz timer */
ulong get_timer_masked(void)
{
	ulong now = READ_TIMER1;

	if (lastdec >= now) {
		/* normal mode */
		timestamp += lastdec - now; /* move stamp forward with absolute diff ticks */
	} else {			/* we have overflow of the count down timer */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;
	return timestamp;
}

/*
 *  u32 get_board_rev() for ARM supplied development boards
 */
ARM_SUPPLIED_GET_BOARD_REV

