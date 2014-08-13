/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Versatile PB.
 *
 * (C) Copyright 2008
 * Cavium Networks Ltd.
 * Scott Shu <scott.shu@caviumnetworks.com>
 * Configuration for Cavium Networks CNS3000 Platform
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H
/*
 *  Code, etc. common to all ARM supplied development boards
 */
#include <armsupplied.h>

/*
 * Board info register
 */
#define SYS_ID  (0x10000000)
#define ARM_SUPPLIED_REVISION_REGISTER SYS_ID

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_CNS3000		1		/* in a Cavium Networks CNS3000 SoC */

#define CONFIG_DISPLAY_CPUINFO	1		/* display cpu info */
#define CONFIG_DISPLAY_BOARDINFO 1		/* display board info */

#define CFG_MEMTEST_START	0x100000
#define CFG_MEMTEST_END		0x10000000

#define CFG_HZ	       		(1000)
#define CFG_HZ_CLOCK		1000000		/* Timers clocked at 1Mhz */
#define CFG_TIMERBASE		0x7C800000	/* Timer 1 base	*/
#define CFG_TIMER_RELOAD	0xFFFFFFFF
#define TIMER_LOAD_VAL		CFG_TIMER_RELOAD

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_MISC_INIT_R

/*
 * Size of malloc() pool
 */
/* scott.check */
#define CFG_MALLOC_LEN		(CONFIG_ENV_SIZE + 512*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define GPIOA_PERST			2

/*
 * NS16550 Configuration
 */
# define CFG_SERIAL0		0x78000000
# define CFG_SERIAL1		0x78400000
# define CFG_SERIAL2		0x78800000
# define CFG_SERIAL3		0x78C00000

/*
 * select serial console configuration
 */
#define CONFIG_CNS3000_SERIAL
#define CONFIG_CNS3000_CLOCK	24000000
#define CONFIG_CNS3000_PORTS	{ (void *)CFG_SERIAL0, (void *)CFG_SERIAL1, (void *)CFG_SERIAL2 }
#define CONFIG_CONS_INDEX	0

#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 2400, 4800, 9600, 19200, 38400, 57600, 115200 }
#define CFG_CONSOLE_INFO_QUIET
#define CONFIG_CMDLINE_EDITING

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/*
 * I2C
 */
#define CONFIG_DRIVER_CNS3XXX_I2C 
#define CONFIG_HARD_I2C     /* I2C with hardware support  */
#undef  CONFIG_SOFT_I2C     /* I2C bit-banged   */
#define CFG_I2C_SPEED   100000  /* I2C speed and slave address  */
#define CFG_I2C_SLAVE   0x7f /* UNUSED */

#define CFG_I2C_EEPROM_ADDR 0x50    /* base address */
#define CFG_I2C_EEPROM_ADDR_LEN 1   /* bytes of address */
/* mask of address bits that overflow into the "EEPROM chip address"    */
#define CFG_I2C_EEPROM_ADDR_OVERFLOW  0x07
#define CFG_EEPROM_PAGE_WRITE_BITS  3 /* 8 byte write page size */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS  10  /* and takes up to 10 msec */


/*
 * Real Time Clock
 */
#define CONFIG_RTC_CNS3000	1

/* 
 * MMC/SD Host Controller
 */
#define CONFIG_CNS3000_MMC	1
#ifdef CONFIG_CNS3000_MMC
#define CONFIG_MMC              1
#define CONFIG_DOS_PARTITION    1
#endif

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DIAG
//#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_SAVES
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SETGETDCR

#ifdef CONFIG_RTC_CNS3000
#define CONFIG_CMD_DATE
#endif

#ifdef CONFIG_CNS3000_MMC
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#endif

#define CONFIG_BOOTDELAY	2

#define CONFIG_UDP_FRAGMENT	1

#ifdef CONFIG_UDP_FRAGMENT
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"tftp_bsize=512\0"		\
	"udp_frag_size=512\0"
#endif

/*
The kernel command line & boot command below are for a Cavium Networks CNS3000 board
0x00000000  u-boot
0x0000????  knuxernel 
0x0000????  Root File System
*/

#define CONFIG_BOOTARGS "console=ttyS0,115200 root=/dev/mtdblock3 rootfstype=squashfs,jffs2 noinitrd init=/etc/preinit"
#define CONFIG_BOOTCOMMAND "bootm 0x60080000"

#define CONFIG_NET_MULTI
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3

#define ENV_ETH_PRIME     "eth0"

/*
 * Static configuration when assigning fixed address
 */
#define CONFIG_NETMASK		255.255.0.0		/* talk on MY local net */
#define CONFIG_IPADDR			192.168.1.211		/* static IP I currently own */
#define CONFIG_SERVERIP		192.168.1.14		/* current IP of my dev pc */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory		 */
#define CFG_PROMPT		"Laguna > "
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
/* Print Buffer Size */
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS		16		/* max number of command args	 */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size		*/

#undef	CFG_CLKS_IN_HZ				/* everything, incl board info, in Hz */
#define CFG_LOAD_ADDR		0x00800000	/* default load address */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define PHYS_SDRAM_32BIT					/* undefined: 16 bits, defined: 32 bits */
// #undef PHYS_SDRAM_32BIT					/* undefined: 16 bits, defined: 32 bits */

#define CONFIG_NR_DRAM_BANKS		1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		       	0x20000000	/* SDRAM Bank #1 */

#ifdef PHYS_SDRAM_32BIT
#define PHYS_SDRAM_1_SIZE		0x4000000	/* 0x10000000 = 256 MB */
#else
#define PHYS_SDRAM_1_SIZE		0x08000000	/* 0x08000000 = 128 MB */
#endif

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
/*
 *  Use the CFI flash driver for ease of use
 */
#define PHYS_FLASH_1						0x60000000

#define CFG_FLASH_BASE			PHYS_FLASH_1
#define CFG_MONITOR_BASE		CFG_MONITOR_BASE
#define CFG_MONITOR_LEN			(256 << 10)

#define CFG_MAX_FLASH_BANKS     1       /* max number of memory banks           */
#define CFG_MAX_FLASH_SECT      64 /* max number of sectors on one chip    */

#define CONFIG_ENV_IS_IN_DATAFLASH  1

#define CONFIG_ENV_SECT_SIZE  0x40000 /* size of one complete sector  */
#define CONFIG_ENV_ADDR   (PHYS_FLASH_1 + 0x40000)
#define CONFIG_ENV_SIZE   0x40000  /* Total Size of Environment Sector */

#define CONFIG_HAS_DATAFLASH 1
#define CONFIG_SPI_FLASH_BOOT 1

#define CFG_SPI_FLASH_BASE    0x60000000

#define CFG_MAX_DATAFLASH_BANKS   1
#define CFG_DATAFLASH_LOGIC_ADDR_CS0  CFG_SPI_FLASH_BASE  /* Logical adress for CS0 */

#define CONFIG_SPI 1


#endif /* __CONFIG_H */
