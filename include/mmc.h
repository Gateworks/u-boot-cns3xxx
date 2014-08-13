/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef _MMC_H_
#define _MMC_H_
#include <asm/arch/mmc.h>
#include <linux/list.h>

#define SD_VERSION_SD			0x20000
#define SD_VERSION_2			(SD_VERSION_SD | 0x20)
#define SD_VERSION_1_0			(SD_VERSION_SD | 0x10)
#define SD_VERSION_1_10			(SD_VERSION_SD | 0x1a)
#define MMC_VERSION_MMC			0x10000
#define MMC_VERSION_UNKNOWN		(MMC_VERSION_MMC)
#define MMC_VERSION_1_2			(MMC_VERSION_MMC | 0x12)
#define MMC_VERSION_1_4			(MMC_VERSION_MMC | 0x14)
#define MMC_VERSION_2_2			(MMC_VERSION_MMC | 0x22)
#define MMC_VERSION_3			(MMC_VERSION_MMC | 0x30)
#define MMC_VERSION_4			(MMC_VERSION_MMC | 0x40)

#define MMC_MODE_HS			0x001
#define MMC_MODE_HS_52MHz		0x010
#define MMC_MODE_4BIT			0x100
#define MMC_MODE_8BIT			0x200

#define SD_DATA_4BIT			0x00040000

#define IS_SD(x)			(mmc->version & SD_VERSION_SD)

#define MMC_DATA_READ			1
#define MMC_DATA_WRITE			2

#define NO_CARD_ERR			-16 /* No SD/MMC card inserted */
#define UNUSABLE_ERR			-17 /* Unusable Card */
#define COMM_ERR			-18 /* Communications Error */
#define TIMEOUT				-19

/* MMC command numbers */
#define MMC_CMD_GO_IDLE_STATE		0
#define MMC_CMD_SEND_OP_COND		1
#define MMC_CMD_ALL_SEND_CID		2
#define MMC_CMD_SET_RELATIVE_ADDR	3
#define MMC_CMD_SET_DSR			4
#define MMC_CMD_SWITCH			6
#define MMC_CMD_SELECT_CARD		7
#define MMC_CMD_SEND_EXT_CSD		8
#define MMC_CMD_SEND_CSD		9
#define MMC_CMD_SEND_CID		10
#define MMC_CMD_STOP_TRANSMISSION	12
#define MMC_CMD_SEND_STATUS		13
#define MMC_CMD_SET_BLOCKLEN		16
#define MMC_CMD_READ_SINGLE_BLOCK	17
#define MMC_CMD_READ_MULTIPLE_BLOCK	18
#define MMC_CMD_WRITE_BLOCK		24 /* old */
#define MMC_CMD_WRITE_SINGLE_BLOCK	24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	25
#define MMC_CMD_APP_CMD			55

/* SD Card command numbers */
#define SD_CMD_SEND_RELATIVE_ADDR	3
#define SD_CMD_SWITCH			6 /* old */
#define SD_CMD_SWITCH_FUNC		6
#define SD_CMD_SEND_IF_COND		8

#define SD_CMD_APP_SET_BUS_WIDTH	6
#define SD_CMD_APP_SEND_OP_COND		41
#define SD_CMD_APP_SEND_SCR		51

int mmc_init(int verbose);
int mmc_read(ulong src, uchar *dst, int size);
int mmc_write(uchar *src, ulong dst, int size);
int mmc2info(ulong addr);

#endif /* _MMC_H_ */
