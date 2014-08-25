/*
 * Copyright (C) 2014 Gateworks Corporation
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <cns3000.h>

#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_ARM11MPCORE
#define CONFIG_CNS3000			/* Cavium Networks cns3xxx SoC */
#define CONFIG_DISPLAY_CPUINFO		/* display cpu info */
#define CONFIG_DISPLAY_BOARDINFO	/* display board info */

#define CONFIG_MACH_TYPE	2635	/* Gateworks Laguna Platform */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)

/* Init Functions */
#define CONFIG_MISC_INIT_R

/* Timer configuration */
#undef	CONFIG_SYS_CLKS_IN_HZ   /* everything, incl board info, in Hz */
#define CONFIG_SYS_HZ	       		(1000)
#define CONFIG_SYS_HZ_CLOCK		1000000		/* Timers at 1Mhz */

/* ATAGs */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
/*#define CONFIG_SERIAL_TAG*/
#define CONFIG_REVISION_TAG

/* UART Configuration */
#define CONFIG_CNS3000_SERIAL
#define CONFIG_CNS3000_CLOCK		24000000
#define CONFIG_SYS_SERIAL0		0x78000000
#define CONFIG_SYS_SERIAL1		0x78400000
#define CONFIG_SYS_SERIAL2		0x78800000
#define CONFIG_SYS_SERIAL3		0x78C00000

/* serial console (ttyS0,115200) */
#define CONFIG_CONS_INDEX		0
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 2400, 4800, 9600, 19200, 38400, 57600, 115200 }

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* I2C */
#define CONFIG_SYS_I2C_CNC3XXX
#define CONFIG_HARD_I2C				/* I2C with hardware support */
#undef  CONFIG_SOFT_I2C				/* I2C bit-banged */
#define CONFIG_SYS_I2C_SPEED		100000	/* I2C speed */
#define CONFIG_SYS_I2C_SLAVE		0x7f	/* UNUSED */

/* Various command support */
#include <config_cmd_default.h>
#undef CONFIG_CMD_IMLS
#ifndef CONFIG_BOOT_FLASH_SPI
#define CONFIG_CMD_JFFS2
#endif
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_GSC
#define CONFIG_LZO

/* Networking */
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3

/* Miscellaneous configurable options */
/* #define CONFIG_SYS_LONGHELP */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT		"Laguna > "
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* Memory Configuration */
#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		0x10000000
#define CONFIG_SYS_TEXT_BASE            0x00000000	/* before relocation */
#define CONFIG_SYS_LOAD_ADDR		0x20800000

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM		       	CNS3000_VEGA_DDR2SDRAM_BASE
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000000	/* 64MB */
#define CONFIG_SYS_INIT_RAM_ADDR	0x00000000	/* before relocation */
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH configuration */
#ifndef CONFIG_BOOT_FLASH_SPI
  #define PHYS_FLASH_1			0x10000000
  #define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
  #define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
  #define CONFIG_SYS_MAX_FLASH_BANKS	1
  #define CONFIG_SYS_FLASH_CFI		/* flash is CFI compatible  */
  #define CONFIG_FLASH_CFI_DRIVER	/* Use common CFI driver  */
  #define CONFIG_ENV_IS_IN_FLASH
  #define CONFIG_SYS_FLASH_BANKS_LIST	{ PHYS_FLASH_1 }
  #define CONFIG_SYS_FLASH_CFI_WIDTH FLASH_CFI_16BIT /* no byte writes */
  #define CONFIG_SYS_FLASH_EMPTY_INFO	/* flinfo: show empty sectors */
  #define CONFIG_ENV_ADDR		(PHYS_FLASH_1 + 0x40000)
  #define CONFIG_ENV_SIZE		0x20000
  #define CONFIG_SYS_MAX_FLASH_SECT	0x40000
#else
  #define CONFIG_SYS_NO_FLASH /* no NOR flash */
  #define CONFIG_ENV_IS_IN_SPI_FLASH
  #define CONFIG_ENV_OFFSET		0x40000
  #define CONFIG_ENV_SIZE		0x40000 /* 256KB */
  #define CONFIG_ENV_SECT_SIZE		0x40000
#endif

/* SPI */
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF
#define CONFIG_CNS3XXX_SPI
#define CONFIG_SPI_FLASH_MTD
#define CONFIG_SPI_FLASH_BAR
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_SPANSION

/* USB Configs */
#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_CNS3XXX
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_USB_KEYBOARD

/* MMC Configs */
#define CONFIG_MMC
#define CONFIG_MMC_SDMA
#define CONFIG_GENERIC_MMC
#define CONFIG_SDHCI
#define CONFIG_CNS3XXX_SDHCI
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT

/* Environment */
#define CONFIG_BOOTDELAY	2
#define CONFIG_LOADADDR		CONFIG_SYS_LOAD_ADDR
#define CONFIG_IPADDR		192.168.1.211
#define CONFIG_SERVERIP		192.168.1.14
#define CONFIG_NETMASK		255.255.0.0
#define CONFIG_BOOTARGS		\
	"console=ttyS0,115200 root=/dev/mtdblock3 rootfstype=squashfs,jffs2"
#ifndef CONFIG_BOOT_FLASH_SPI
#define CONFIG_BOOTCOMMAND	\
	"bootm 0x10060000"
#else
#define CONFIG_BOOTCOMMAND	\
	"sf probe && sf read 0x800000 0x80000 0x180000 && bootm"
#endif

#endif /* __CONFIG_H */
