/*
 * Copyright (C) 2014 Gateworks Corporation
 *
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
 * SPDX-License-Identifier: GPL-2.0+
 */
#include <common.h>
#include <cns3000.h>
#include <i2c.h>
#include <mmc.h>

#include <asm/hardware.h>
#include <asm/io.h>

extern int cns3xxx_sdhci_init(u32 regbase);
extern int cns3xxx_eth_initialize(bd_t *bis);
DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
    printf("Boot reached stage %d\n", progress);
}
#endif

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

static int pcie_init(void)
{
	unsigned char eeprom;
	unsigned int temp;
	unsigned char external_clkgen;
	int gpio_perst = -1;
	uint8_t model[16];
	uint8_t rev;
	int sp;

	/* Errata C-01 */
	IO_WRITE(PCIE0_PHY_ERRATA, 0xe2c);
	IO_WRITE(PCIE1_PHY_ERRATA, 0xe2c);

	/* determine board model */
	i2c_read(0x51, 0x30, 1, model, sizeof(model));

	/* determine board revision */
	rev = '\0';
	for (temp = sizeof(model) - 1; temp >= 0; temp--) {
		if (model[temp] >= 'A') {
			rev = model[temp];
			break;
		}
	}

	/* determine board sp number (assume 3 digit special number) */
	sp = 0;
	for (temp = sizeof(model) - 4; temp > 0; temp--) {
		if (model[temp - 1] == 'S' && model[temp] == 'P') {
			/* Convert SPXXX to numeric */
			sp = (sp * 10) + (model[temp+1] - '0');
			sp = (sp * 10) + (model[temp+2] - '0');
			sp = (sp * 10) + (model[temp+3] - '0');
			break;
		}
	}

	/* determine gpio used for PCI PERST# which is board specific */
	if (strncmp((char *)model, "GW2380", 6) == 0 ||
	    strncmp((char *)model, "GW2382", 6) == 0 ||
	    strncmp((char *)model, "GW2383", 6) == 0) {
		gpio_perst = 2;
	}
	else if (strncmp((char *)model, "GW2391", 6) == 0 ||
		   strncmp((char *)model, "GW2393", 6) == 0)
		gpio_perst = 8;
	else
		gpio_perst = 11;

	/* boards with external PCI clockgen have GPIOB26 high at powerup */
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

	return 0;
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	icache_enable ();

	return 0;
}


int misc_init_r (void)
{
	int i;
	uint8_t buf[20];
	char str[20];

	printf("Gateworks Corporation Copyright 2010\n");
	i2c_read(0x51, 0x30, 1, buf, 16);
	buf[16] = 0;
	printf("Model Number: %s\n", buf);
	i2c_read(0x51, 0x20, 1, buf, 4);
	printf("Manufacturer Date: %02x-%02x-%02x%02x\n",
	       buf[0], buf[1], buf[2], buf[3]);
	i2c_read(0x51, 0x18, 1, buf, 4);
	printf("Serial #: %i\n",
	       buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));

	for (i = 0; i < 3; i++) {
		i2c_read(0x51, i*6, 1, buf, 6);
		if (i == 0)
			sprintf(str, "ethaddr");
		else
			sprintf(str, "eth%daddr", i);
		if (getenv(str))
			continue;
		eth_setenv_enetaddr(str, buf);
		printf("### Setting environment from ROM MAC address = \"%s\"\n",
		       getenv(str));
	}

	return (0);
}

int checkboard(void)
{
	return pcie_init();
}

int dram_init (void)
{
	unsigned char eeprom;
	unsigned int temp = 0x01000000;
	gd->bd->bi_dram[0].start = PHYS_SDRAM;

	i2c_read(0x51, 0x2b, 1, &eeprom, 1);
	gd->bd->bi_dram[0].size = temp << eeprom;
	gd->ram_size = temp << eeprom;
	return 0;
}

void dram_init_banksize(void)
{
	unsigned char eeprom;
	unsigned int temp = 0x01000000;

	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	i2c_read(0x51, 0x2b, 1, &eeprom, 1);
	gd->bd->bi_dram[0].size = temp << eeprom;
}

int board_eth_init(bd_t *bis)
{
	return cns3xxx_eth_initialize(bis);
}

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
	u32 reg = readl(CNS3000_VEGA_PMU_BASE + PM_CLK_GATE_OFFSET);

	/* init MMC if supported on board */
	if (reg & (1 << 25)) {
		HAL_MISC_ENABLE_SD_PINS();
		return (cns3xxx_sdhci_init(CNS3000_VEGA_SDIO_BASE));
	} else {
		puts("n/a\n");
	}
	return 0;
}
#endif
