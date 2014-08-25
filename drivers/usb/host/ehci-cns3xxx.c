/*
 * (C) Copyright 2010
 * Armando Visconti, ST Micoelectronics, <armando.visconti@st.com>.
 *
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <usb.h>
#include "ehci.h"
#include <asm/arch/hardware.h>

#define CNS3XXX_USB_BASE	0x82000000  /* USB Host Control */

/* Helper macros to read/write registers */
#define IO_USB_WRITE(addr, val) (*(volatile unsigned int *)(addr) = (val))
#define IO_USB_READ(addr) (*(volatile unsigned int *)(addr))

/* PMU REgisters to Enable Clock and Soft Reset EHCI Block */
#define PMU_CLK_GATE_REG 0x77000000
#define PMU_SOFT_RST   0x77000004
#define PMU_HM_PD_CTRL 0x7700001c
#define MISC_CHIP_CFG 0x76000004
/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	unsigned int temp;

	// Enable USB PLL
	temp = IO_USB_READ(PMU_HM_PD_CTRL);
	temp &= ~0x8;
	IO_USB_WRITE(PMU_HM_PD_CTRL, temp);

	// Enable Clock
	temp = IO_USB_READ(PMU_CLK_GATE_REG);
	temp |= (1 << 16);
	IO_USB_WRITE(PMU_CLK_GATE_REG, temp);

	// Issue Soft Reset to EHCI Block
	temp = IO_USB_READ(PMU_SOFT_RST);
	temp &= ~(1 << 16);
	IO_USB_WRITE(PMU_SOFT_RST, temp);
	temp |= (1 << 16);
	IO_USB_WRITE(PMU_SOFT_RST, temp);

	// Set Burst mapping to INCR
	temp = IO_USB_READ(MISC_CHIP_CFG);
	temp |= (0x2 << 24);
	IO_USB_WRITE(MISC_CHIP_CFG, temp);


	*hccr = (struct ehci_hccr *)(CNS3XXX_USB_BASE);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr
			+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("cns3xxx-ehci: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)*hccr, (uint32_t)*hcor,
		(uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	return 0;
}
