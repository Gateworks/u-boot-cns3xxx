/*
 * (C) Copyright 2013 Inc.
 *
 * Xilinx Zynq SD Host Controller Interface
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <sdhci.h>

int cns3xxx_sdhci_init(u32 regbase)
{
	struct sdhci_host *host = NULL;

	host = (struct sdhci_host *)malloc(sizeof(struct sdhci_host));
	if (!host) {
		return 1;
	}

	host->name = "cns3xxx_sdhci";
	host->ioaddr = (void *)regbase;
	host->version = (sdhci_readl(host, SDHCI_HOST_VERSION - 2) >> 16) & 0xff;

	host->host_caps = MMC_MODE_HC;
	host->bus_width = 4;

	add_sdhci(host, 150000000, 52000000 >> 9);
	return 0;
}
