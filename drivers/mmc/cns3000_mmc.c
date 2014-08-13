/*
 * Copyright (c) 2008 Cavium Networks
 *
 * Scott Shu
 *
 * SD memory and SDIO host controller access based on
 * standard SD/SDIO host controller spec 2.0 register set.
 *
 * This file is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License, Version 2, as 
 * published by the Free Software Foundation. 
 *
 * This file is distributed in the hope that it will be useful, 
 * but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 * NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License 
 * along with this file; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 * visit http://www.gnu.org/licenses/. 
 *
 * This file may also be available under a different license from Cavium. 
 * Contact Cavium Networks for more information
 */

#include <config.h>
#include <common.h>
#include <mmc.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>
#include <part.h>

#include <cns3000.h>

#undef MMC_DEBUG

#define MAX_WAIT_TIMEOUT	1000

#define FAIL_IN_COMMAND0	1
#define FAIL_IN_COMMAND2	2
#define FAIL_IN_COMMAND3	3
#define FAIL_IN_COMMAND7	4
#define FAIL_IN_COMMAND8	5
#define FAIL_IN_COMMAND9	6	
#define FAIL_IN_ACOMMAND41	7

#define FAIL_IN_ACMD6		6	

#define MMC_WRITE(reg, value)		(*(volatile unsigned int*)(reg) = (value))
#define MMC_READ(reg)			(*(volatile unsigned int*)(reg))

#define MMC_WRITE_REGl(reg, value)	(*(volatile unsigned long*)(reg) = (value))
#define MMC_WRITE_REGw(reg, value)	(*(volatile unsigned short*)(reg) = (value))
#define MMC_WRITE_REGb(reg, value)	(*(volatile unsigned char*)(reg) = (value))

#define MMC_READ_REGl(reg)		(*(volatile unsigned long*)(reg))
#define MMC_READ_REGw(reg)		(*(volatile unsigned short*)(reg))
#define MMC_READ_REGb(reg)		(*(volatile unsigned char*)(reg))

#define UNSTUFF_BITS(resp,start,size)					\
	({								\
		const int __size = size;				\
		const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
		const int __off = 3 - ((start) / 32);			\
		const int __shft = (start) & 31;			\
		u32 __res;						\
									\
		__res = resp[__off] >> __shft;				\
		if (__size + __shft > 32)				\
			__res |= resp[__off-1] << ((32 - __shft) % 32);	\
		__res & __mask;						\
	})

#ifdef CONFIG_MMC

extern int fat_register_device(block_dev_desc_t *dev_desc, int part_no);

int issue_data_read(unsigned char cmd_code, unsigned char response_type, unsigned char command_type,
		    unsigned char index_check, unsigned char crc_check, unsigned long arg,
		    unsigned short block_size, unsigned short block_count, unsigned short transfer_mode,
		    void *resp, unsigned char *data_buffer);

static block_dev_desc_t mmc_dev;

block_dev_desc_t * mmc_get_dev(int dev)
{
	return ((block_dev_desc_t *) &mmc_dev);
}

int send_cmd0(void)
{
	unsigned long reg_val;
	int i = 0;

	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x01) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* write 0 to argument register */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), 0x00);

	/* enable complete(dat) and complete(cmd) interrupt */
	/* disable all interrupts except "completed data" and "completed command" */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), 0x03); 
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x03);
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x36), 0x00);
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x3a), 0x00);

	/* write CMD0 */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), 0x00);
	
	/* wait for interrupt */
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* get intr and read reg30 */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30);

	if (reg_val & 0x1)
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01); /* write reg30 to clear interrupt */
	else 
		return -1; /* wrong interrupt */
		
	return 0;
}

int send_cmd2(void *resp)
{
        unsigned long *response = resp;
	unsigned long reg_val;
	int i = 0;

	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x01) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* write 0 to argument register */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), 0x00);

	/* enable complete(dat) and complete(cmd) interrupt */
	/* disable all interrupts except "completed data" and "completed command" */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), 0x03); 
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x03);

	/* write CMD2, crc check enable and response type select */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), 0x0209);
	
	/* wait for interrupt */
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* get intr and read reg30 */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30);

	if (reg_val & 0x1) {
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01); /* write reg30 to clear interrupt */
		/* response[] >> 8 is the real response data (R[127:8]), Ref. SDHC Simplified Spec. v2.0, page 31. */
		response[0] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x10); /* 31:0 */
		response[1] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x14); /* 63:32 */
		response[2] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x18); /* 95:64 */
		response[3] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x1c); /* 127:96 */
	} else 
		return -1; /* wrong interrupt */

	return 0;
}

int send_cmd3(unsigned short *mmc_rca)
{
	unsigned long reg_val;
	int i = 0;

	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x01) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* write 0 to argument register */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), 0x00);

	/* enable complete(dat) and complete(cmd) interrupt */
	/* disable all interrupts except "completed data" and "completed command" */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), 0x03); 
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x03);

	/* write CMD3, crc check enable and response type select */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), 0x031a);

	/* wait for interrupt */
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* get intr and read reg30 */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30);

	if (reg_val & 0x1) {
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01); /* write reg30 to clear interrupt */
		/* read response, card now has RCA = 0x0001 */
		*mmc_rca = MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x12);
	} else 
		return -1; /* wrong interrupt */

	return 0;
}

int send_cmd7(unsigned short mmc_rca)
{
	unsigned long reg_val;
	int i = 0;

	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x01) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* write to argument register */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), (mmc_rca & 0xffff) << 16);

	/* enable complete(dat) and complete(cmd) interrupt */
	/* disable all interrupts except "completed data" and "completed command" */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), 0x03); 
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x03);

	/* write CMD7, crc check enable and response type select */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), 0x071b);

	/* wait for interrupt */
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* get intr and read reg30 */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30);

	if (reg_val & 0x1) {
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01); /* write reg30 to clear interrupt */
	} else 
		return -1; /* wrong interrupt */

	/* read response */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x10);

	/* get intr and read reg30 */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30);

	if (reg_val & 0x2)
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x02); /* write reg30 to clear interrupt */
	else
		return -1; /* wrong interrupt */
		
	return 0;
}

int send_cmd8(void)
{
	unsigned long reg_val;
	int i = 0;

	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x01) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* write 0x1aa to argument register */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), 0x01aa);

	/* enable complete(dat) and complete(cmd) interrupt */
	/* disable all interrupts except  "completed data" and "completed command" */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), 0x03);
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x03);

	/* write CMD8 */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), 0x081a);
	
	/* wait for interrupt */
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* get intr and read reg30 */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30);

	if (reg_val & 0x1) {
		/* Card is version 2 */
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01); /* write reg30 to clear interrupt */
		return 1;
	} else if (reg_val == 0x00018000) {
		/* Card timeout, Card is not version 2 */
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x32), 0x01); /* write reg30 to clear interrupt */
		return 0;
	}
	
	return -1;
}

int send_acmd6(unsigned short mmc_rca)
{
	unsigned long reg_val;
	int i = 0;

	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x01) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* write to argument register */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), (mmc_rca & 0xffff) << 16);

	/* enable complete(dat) and complete(cmd) interrupt */
	/* disable all interrupts except "completed data" and "completed command" */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), 0x03); 
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x03);

	/* write CMD55 */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), 0x371a);

	/* wait for interrupt */
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* get intr and read reg30 */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30);

	if (reg_val & 0x01) {
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01); /* write reg30 to clear interrupt */
	} else {
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), reg_val);
		return -1; /* wrong interrupt */
	}

	/* write ACMD6 */
	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x01) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* 4-bit */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), 0x02);
	
	/* enable complete(dat) and complete(cmd) interrupt */
	/* disable all interrupts except "completed data" and "completed command" */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), 0x03); 
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x03);

	/* write CMD6 */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), 0x061a);
	
	/* wait for interrupt */
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* get intr and read reg30 */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30);

	if (reg_val & 0x01)
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01); /* write reg30 to clear interrupt */
	else {
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), reg_val);
		return -1; /* wrong interrupt */
	}

	return 0;
}

int send_acmd41(void *resp)
{
	unsigned long *response = resp;
	unsigned long reg_val;
	int i = 0;

	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x01) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* write 0 to argument register */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), 0x00);

	/* enable complete(dat) and complete(cmd) interrupt */
	/* disable all interrupts except "completed data" and "completed command" */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), 0x03);
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x03);

	/* write CMD55 first */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), 0x371a);
	
	/* wait for interrupt */
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* get intr and read reg30 */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30);

	if (reg_val & 0x01) {
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01); /* write reg30 to clear interrupt */
	} else {
		return -1;
	}

	i = 0;
	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x01) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* write ACMD41 */
	/* write to argument register */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), 0xff8000 | (1 << 30) | 0x0);

	/* enable complete(dat) and complete(cmd) interrupt */
	/* disable all interrupts except  "completed data" and "completed command" */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), 0x03);
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x03);

	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), 0x291a); /* b'3: Command CRC enable, b'4: Command index enable */	
	//MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), 0x2902);
	
	/* wait for interrupt */
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	udelay(200);
	/* get intr and read reg30 */
	reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30);

	if (reg_val & 0x1) {
		reg_val = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x10); /* read response, should be same as argument sent */
		response[0] = reg_val;
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01); /* write reg30 to clear interrupt */
	} else {
		return -1;
	}
	return 0;
}

static int sd_parse_cid(struct mmc_cid *cid, unsigned long *resp)
{
	cid->mid = (resp[3] >> 16) & 0xff;
	cid->oid = resp[3] & 0xffff;
	cid->pnm[0] = resp[2] >> 24;
	cid->pnm[1] = resp[2] >> 16;
	cid->pnm[2] = resp[2] >> 8;
	cid->pnm[3] = resp[2];
	cid->pnm[4] = resp[1] >> 24;
	cid->pnm[5] = 0;
	cid->pnm[6] = 0;
	cid->prv = resp[1] >> 16;
	cid->psn = (resp[1] & 0xff) | (resp[0] >> 16);
	cid->mdt = resp[0] & 0x0fff;

	return 0;
}

#ifdef MMC_DEBUG
static void mmc_dump_cid(const struct mmc_cid *cid)
{
	printf("Manufacturer ID:       %02X\n", cid->mid);
	printf("OEM/Application ID:    %04X\n", cid->oid);
	printf("Product name:          %s\n", cid->pnm);
	printf("Product Revision:      %u.%u\n", cid->prv >> 4, cid->prv & 0x0f);
	printf("Product Serial Number: %lu\n", cid->psn);
	printf("Manufacturing Date:    %02u/%02u\n", cid->mdt >> 4, cid->mdt & 0x0f);
}
#endif

static int sd_parse_csd(struct mmc_csd *csd, unsigned long *resp)
{
	unsigned long csd_raw[4];

	csd_raw[0] = (resp[3] << 8) | (resp[2] >> 24);
	csd_raw[1] = (resp[2] << 8) | (resp[1] >> 24);
	csd_raw[2] = (resp[1] << 8) | (resp[0] >> 24);
	csd_raw[3] = (resp[0] << 8);

#ifdef MMC_DEBUG
	printf("CSD data: %08lx %08lx %08lx %08lx\n", csd_raw[0], csd_raw[1], csd_raw[2], csd_raw[3]);
#endif

	csd->csd_structure	= UNSTUFF_BITS(csd_raw, 126, 2);
	switch (csd->csd_structure) {
	case 0:
		csd->c_size		= UNSTUFF_BITS(csd_raw, 62, 12);
		csd->vdd_r_curr_min	= UNSTUFF_BITS(csd_raw, 59, 3);
		csd->vdd_r_curr_max	= UNSTUFF_BITS(csd_raw, 56, 3);
		csd->vdd_w_curr_min	= UNSTUFF_BITS(csd_raw, 53, 3);
		csd->vdd_w_curr_max	= UNSTUFF_BITS(csd_raw, 50, 3);
		csd->c_size_mult	= UNSTUFF_BITS(csd_raw, 47, 3);
		break;
	case 1:
		csd->taac		= UNSTUFF_BITS(csd_raw, 112, 8);
		csd->nsac		= UNSTUFF_BITS(csd_raw, 104, 8);
		csd->tran_speed		= UNSTUFF_BITS(csd_raw, 96, 8);
		csd->ccc		= UNSTUFF_BITS(csd_raw, 84, 12);
		csd->read_bl_len	= UNSTUFF_BITS(csd_raw, 80, 4);
		csd->read_bl_partial	= UNSTUFF_BITS(csd_raw, 79, 1);
		csd->write_blk_misalign	= UNSTUFF_BITS(csd_raw, 78, 1);
		csd->read_blk_misalign	= UNSTUFF_BITS(csd_raw, 77, 1);
		csd->dsr_imp		= UNSTUFF_BITS(csd_raw, 76, 1);
		csd->c_size		= UNSTUFF_BITS(csd_raw, 48, 22);
		csd->erase_blk_en	= UNSTUFF_BITS(csd_raw, 46, 1);
		csd->sector_size	= UNSTUFF_BITS(csd_raw, 39, 7);
		csd->wp_grp_size	= UNSTUFF_BITS(csd_raw, 32, 7);
		csd->wp_grp_enable	= UNSTUFF_BITS(csd_raw, 31, 1);
		csd->r2w_factor		= UNSTUFF_BITS(csd_raw, 26, 3);
		csd->write_bl_len	= UNSTUFF_BITS(csd_raw, 22, 4);
		csd->write_bl_partial	= UNSTUFF_BITS(csd_raw, 21, 1);
		csd->file_format_grp	= UNSTUFF_BITS(csd_raw, 15, 1);
		csd->copy		= UNSTUFF_BITS(csd_raw, 14, 1);
		csd->perm_write_protect	= UNSTUFF_BITS(csd_raw, 13, 1);
		csd->tmp_write_protect	= UNSTUFF_BITS(csd_raw, 12, 1);
		csd->file_format	= UNSTUFF_BITS(csd_raw, 10, 2);

#ifdef MMC_DEBUG
		printf("csd_structure = %01x\n", csd->csd_structure);
		printf("tran_speed = %02x\n", csd->tran_speed);
		printf("read_bl_len = %01x\n", csd->read_bl_len);
		printf("read_bl_partial = %01x\n", csd->read_bl_partial);
		printf("write_blk_misalign = %01x\n", csd->write_blk_misalign);
		printf("read_blk_misalign = %01x\n", csd->read_blk_misalign);
		printf("dsr_imp = %01x\n", csd->dsr_imp);
		printf("c_size = %06x\n", csd->c_size);
		printf("erase_blk_en = %01x\n", csd->erase_blk_en);
		printf("sector_size = %02x\n", csd->sector_size);
		printf("wp_grp_size = %02x\n", csd->wp_grp_size);
		printf("wp_grp_enable = %01x\n", csd->wp_grp_enable);
		printf("write_bl_len = %01x\n", csd->write_bl_len);
		printf("write_bl_partial = %01x\n", csd->write_bl_partial);
		printf("copy = %01x\n", csd->copy);
#endif
		break;
	default:
		printf("Unrecognised CSD structure version!\n");
		return -EINVAL;
	}
	return 0;
}

#ifdef MMC_DEBUG
static void mmc_dump_csd(const struct mmc_csd *csd)
{

	printf("CSD structure version:   %u\n", csd->csd_structure);
	printf("Card command classes:    %03x\n", csd->ccc);
	printf("Read block length:       %u\n", 1 << csd->read_bl_len);

	if (csd->read_bl_partial)
		puts("Supports partial reads\n");
	else
		puts("Does not support partial reads\n");

	printf("Write block length:      %u\n", 1 << csd->write_bl_len);

	if (csd->write_bl_partial)
		puts("Supports partial writes\n");
	else
		puts("Does not support partial writes\n");

	if (csd->wp_grp_enable)
		printf("Supports group WP:      %u\n", csd->wp_grp_size + 1);
	else
		puts("Does not support group WP\n");

	if (csd->csd_structure)
		printf("Card capacity:          %u Kbytes\n", ((int) csd->c_size + 1) * 512);

	printf("File format:            %u/%u\n", csd->file_format_grp, csd->file_format);
	puts("Write protection:        ");
	if (csd->perm_write_protect)
		puts(" permanent");
	if (csd->tmp_write_protect)
		puts(" temporary");
	putc('\n');
}
#endif

/* This task is the command completion sequence for simple command */
static int cmd_simple_completion(void *resp)
{
	unsigned long *response = resp;
	unsigned short usreg_val;

	/* enable command completion interrupt */
	usreg_val = MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x34); 
	usreg_val |= 0x01;
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), usreg_val); 
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x01); 
	
	/* wait for command completion interrupt */
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
	}

	/* check for error and clear the command completion status */
	if (MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x32))
		return -1;
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01);

	/* get response */
	response[0] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x10);
	response[1] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x14);
	response[2] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x18);
	response[3] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x1c);

	return 0;
}

/* This task is for issuing commands that does not use the data bus and does not require R1B response type. */
static int issue_simple_cmd(unsigned char cmd_code, unsigned char response_type, unsigned char command_type,
			    unsigned char index_check, unsigned char crc_check, unsigned long arg, void *resp)
{
	unsigned short usreg_val, ustmp;
	int i = 0;

	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x01) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* write to argument register */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), arg);
	
	/* construct the command register */
	usreg_val = 0;				/* bit 15, bit 14, bit 5, and bit 2 are set to 0 */
	ustmp = (unsigned short) (response_type & 0x03);
	usreg_val |= ustmp;			/* bit 0..1 */
	ustmp = (unsigned short) command_type;
	usreg_val |= ((ustmp & 0x03) << 6);;	/* bit 6..7 */
	ustmp = (unsigned short) crc_check;
	usreg_val |= ((ustmp & 0x01) << 3);	/* bit 3 */
	ustmp = (unsigned short) index_check;
        usreg_val |= ((ustmp & 0x01) << 4);	/* bit 4 */
	ustmp = (unsigned short) cmd_code;
        usreg_val |= ((ustmp & 0x3f) << 8);	/* bit 8..13 */

	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), usreg_val);

	if (cmd_simple_completion(resp))
		return -1;
	return 0;
}

#ifdef MMC_DEBUG
/* This task is the command completion sequence for commands that uses the DAT line to check for card busy */
static int cmd_r1b_completion(void *resp)
{
	unsigned long *response = resp;
	unsigned short usreg_val;

	/* enable command completion interrupt */
	usreg_val = MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x34); 
	usreg_val |= 0x01;
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), usreg_val); 
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x01); 
	
	/* wait for command completion interrupt */
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x01)) {
		udelay(100);
	}

	/* check for error and clear the command completion status */
	if (MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x32))
		return -1;
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x01);

	/* */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x02); 

	/* wait for command completion interrupt */
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x02)) {
		udelay(100);
	}
	
	/* check for error and clear the command completion status */
	if (MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x32))
		return -1;
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x02);

	/* get response */
	response[0] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x10);
	response[1] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x14);
	response[2] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x18);
	response[3] = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x1c);

	return 0;
}

/* This task is for issuing commands that uses the DAT line such as commands that require R1B response type. */
static int issue_r1b_cmd(unsigned char cmd_code, unsigned char response_type, unsigned char command_type,
			 unsigned char index_check, unsigned char crc_check, unsigned long arg, void *resp)
{
	unsigned short usreg_val, ustmp;
	int i = 0;

	/* read reg24 and check command inhibit_CMD and command_inhibit_DAT bit until bit is zero */
	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x03) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* write to argument register */
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), arg);
	
	/* construct the command register */
	usreg_val = 0;				/* bit 15, bit 14, bit 5, and bit 2 are set to 0 */
	ustmp = (unsigned short) (response_type & 0x03);
	usreg_val |= ustmp;			/* bit 0..1 */
	ustmp = (unsigned short) command_type;
	usreg_val |= ((ustmp & 0x03) << 6);;	/* bit 6..7 */
	ustmp = (unsigned short) crc_check;
	usreg_val |= ((ustmp & 0x01) << 3);	/* bit 3 */
	ustmp = (unsigned short) index_check;
        usreg_val |= ((ustmp & 0x01) << 4);	/* bit 4 */
	ustmp = (unsigned short) cmd_code;
        usreg_val |= ((ustmp & 0x3f) << 8);	/* bit 8..13 */

	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), usreg_val);

	if (cmd_r1b_completion(resp))
		return -1;

	return 0;
}
#endif

int issue_data_read(unsigned char cmd_code, unsigned char response_type, unsigned char command_type,
		    unsigned char index_check, unsigned char crc_check, unsigned long arg,
		    unsigned short block_size, unsigned short block_count, unsigned short transfer_mode,
		    void *resp, unsigned char *data_buffer)
{
	unsigned short usreg_val, ustmp;
	unsigned char multi_single, block_count_enable;
	unsigned short blk_cnt, blk_size;
	unsigned long *needle;
	int i = 0, j = 0;

	/* read reg24 and check command inhibit_CMD and command_inhibit_DAT bit until bit is zero */
	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x03) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* setup for buffer read ready interrupt and transfer complete interrupt */
	usreg_val = MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x34);
	usreg_val |= 0x22;
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), usreg_val);

	/* write to block_size, block_count, argument and transfer mode registers */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x04), block_size);
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x06), block_count);
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), arg);
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0c), transfer_mode);

	/* construct the command register */
	usreg_val = 0;				/* bit 15, bit 14 and bit 5 are set to 0 */
	ustmp = (unsigned short) (response_type & 0x03);
	usreg_val |= ustmp;			/* bit 0..1 */
	ustmp = (unsigned short) command_type;
	usreg_val |= ((ustmp & 0x03) << 6);;	/* bit 6..7 */
	ustmp = (unsigned short) crc_check;
	usreg_val |= ((ustmp & 0x01) << 3);	/* bit 3 */
	ustmp = (unsigned short) index_check;
        usreg_val |= ((ustmp & 0x01) << 4);	/* bit 4 */
	ustmp = (unsigned short) cmd_code;
	usreg_val |= 0x04;			/* bit 2 */
        usreg_val |= ((ustmp & 0x3f) << 8);	/* bit 8..13 */

	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), usreg_val);
	
	/* wait for command completion and get response */
	if (cmd_simple_completion(resp) != 0)
		return -1;

	/* compute block size in dwords */
	blk_size = block_size;
	if (block_size & 0x03)			/* check whether it is 3 or less */
		blk_size = blk_size + 1;

	if (transfer_mode & 0x20)		/* bit 5 */
		multi_single = 1;
	else
		multi_single = 0;

	if (transfer_mode & 0x02)		/* bit 1 */
		block_count_enable = 1;
	else
		block_count_enable = 0;

	/* start a while-loop to get all the data blocks */
	j = 0;
	blk_cnt = block_count;
	while (blk_cnt) {
		/* wait for buffer read ready interrupt */
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x20);
		i = 0;
		while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & (1 << 5))) {
			udelay(100);
			if (i > MAX_WAIT_TIMEOUT)
				return -1;
			i++;
		}

		/* clear buffer read ready interrupt */
		if (MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x32))
			return -1;
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x20);

		/* get one block of read data */
		needle = (unsigned long *)(data_buffer + j * blk_size);
		for (i = 0; i < blk_size; i += 4) {
			*needle = MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x20);
			needle++;
		}

		/* checks the setting, patch */
		if (multi_single == 0)
			blk_cnt = 0;
		else if (block_count_enable == 0)
			blk_cnt = 1;
		else
			blk_cnt--;

		j++;
	}

	/* wait for command completion interrupt */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x02);
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x02)) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* clear buffer read ready interrupt */
	if (MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x32))
		return -1;
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x02);

	return 0;
}

int issue_data_write(unsigned char cmd_code, unsigned char response_type, unsigned char command_type,
		     unsigned char index_check, unsigned char crc_check, unsigned long arg,
		     unsigned short block_size, unsigned short block_count, unsigned short transfer_mode,
		     void *resp, unsigned char *data_buffer)
{
	unsigned short usreg_val, ustmp;
	unsigned char multi_single, block_count_enable;
	unsigned short blk_cnt, blk_size;
	int i = 0, j = 0;

	/* read reg24 and check command inhibit_CMD and command_inhibit_DAT bit until bit is zero */
	while (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) & 0x03) {
		udelay(100);
		if (i > MAX_WAIT_TIMEOUT)
			return -1;
		i++;
	}

	/* setup for buffer read ready interrupt and transfer complete interrupt */
	usreg_val = MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x34);
	usreg_val |= 0x12;
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x34), usreg_val);

	/* write to block_size, block_count, argument and transfer mode registers */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x04), block_size);
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x06), block_count);
	MMC_WRITE_REGl((CNS3000_VEGA_SDIO_BASE + 0x08), arg);
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0c), transfer_mode);

	/* construct the command register */
	usreg_val = 0;				/* bit 15, bit 14 and bit 5 are set to 0 */
	ustmp = (unsigned short) (response_type & 0x03);
	usreg_val |= ustmp;			/* bit 0..1 */
	ustmp = (unsigned short) command_type;
	usreg_val |= ((ustmp & 0x03) << 6);;	/* bit 6..7 */
	ustmp = (unsigned short) crc_check;
	usreg_val |= ((ustmp & 0x01) << 3);	/* bit 3 */
	ustmp = (unsigned short) index_check;
        usreg_val |= ((ustmp & 0x01) << 4);	/* bit 4 */
	ustmp = (unsigned short) cmd_code;
	usreg_val |= 0x04;			/* bit 2 */
        usreg_val |= ((ustmp & 0x3f) << 8);	/* bit 8..13 */

	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x0e), usreg_val);
	
	/* wait for command completion and get response */
	if (cmd_simple_completion(resp) != 0)
		return -1;

	/* compute block size in dwords */
	blk_size = block_size;
	if (block_size & 0x03)			/* check whether it is 3 or less */
		blk_size = blk_size + 1;

	if (transfer_mode & 0x20)		/* bit 5 */
		multi_single = 1;
	else
		multi_single = 0;

	if (transfer_mode & 0x02)		/* bit 1 */
		block_count_enable = 1;
	else
		block_count_enable = 0;

	/* start a while-loop to get all the data blocks */
	j = 0;
	blk_cnt = block_count;
	while (blk_cnt) {
		/* wait for buffer write ready interrupt */
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x10);
		i = 0;
		while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & (1 << 4))) {
			udelay(100);
			if (i > MAX_WAIT_TIMEOUT)
				return -1;
			i++;
		}

		/* clear buffer read ready interrupt */
		if (MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x32))
			return -1;
		MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x10);

		/* write one block of data */
		for (i = 0; i < blk_size; i += 4) {
			MMC_WRITE_REGl(CNS3000_VEGA_SDIO_BASE + 0x20, *((unsigned long *) data_buffer + (j * blk_size + i / 4)));
		}
		/* checks the setting */
		if (multi_single == 0)
			blk_cnt = 0;
		else if (block_count_enable == 0)
			blk_cnt = 1;
		else
			blk_cnt--;
		j++;
	}

	/* wait for command completion interrupt */
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x38), 0x02);
	i = 0;
	while (! (MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x30) & 0x02)) {
		udelay(1000);
		if (i > MAX_WAIT_TIMEOUT)
			return -5;
		i++;
	}

	/* clear buffer read ready interrupt */
	if (MMC_READ_REGw(CNS3000_VEGA_SDIO_BASE + 0x32))
		return -1;
	MMC_WRITE_REGw((CNS3000_VEGA_SDIO_BASE + 0x30), 0x02);

	return 0;
}

/*
 *
 *
 */
unsigned long mmc_block_read(int dev, unsigned long start, lbaint_t blkcnt, void *buffer)
{
	int ret = 0, i = 0;
	unsigned long resp[4];

#ifdef MMC_DEBUG
	printf("mmc_block_read: start=%lu, blkcnt=%lu\n", start, (unsigned long)blkcnt);
#endif

	/* send CMD16 to set block length */
	ret = issue_simple_cmd(16, 0x02, 0x00, 0x01, 0x01, mmc_dev.blksz, resp);

	for (i = start; i < start + blkcnt; i++, buffer += mmc_dev.blksz) {
		ret = issue_data_read(MMC_CMD_READ_SINGLE_BLOCK, 2, 0, 1, 1, i, mmc_dev.blksz, 1, (1 << 4), resp, buffer); 

		if (ret) {
			printf("block read failed: %d\n", ret);
			return i - start;
		}
	}

	return blkcnt;
}

unsigned long mmc_block_write(int dev, unsigned long start, lbaint_t blkcnt, const void *buffer)
{
	int ret = 0, i = 0;
	unsigned long resp[4];

#ifdef MMC_DEBUG
	printf("mmc_block_write: start=%lu, blkcnt=%lu\n", start, (unsigned long)blkcnt);
#endif

	/* send CMD16 to set block length */
	ret = issue_simple_cmd(16, 0x02, 0x00, 0x01, 0x01, mmc_dev.blksz, resp);

	for (i = start; i < start + blkcnt; i++, buffer += mmc_dev.blksz) {
		ret = issue_data_write(MMC_CMD_WRITE_SINGLE_BLOCK, 2, 0, 1, 1, i, mmc_dev.blksz, 1, (0 << 4), resp, (unsigned char *) buffer);

		if (ret) {
			printf("block write failed: %d\n", ret);
			return i - start;
		}
	}
	if (blkcnt > 1) {
		/* MMC_CMD_STOP_TRANSMISSION */
	}

	return blkcnt;
}

int mmc_init(int verbose)
{
	unsigned long resp[4];
	struct mmc_cid cid;
	struct mmc_csd csd;
	unsigned short mmc_rca;
	int ret = -ENODEV;
	int i = 0, version_2;
	
	/* Reset device interface type */
	mmc_dev.if_type = IF_TYPE_UNKNOWN;

#if 1
{
	int tmp;
	/* there are some share pin with GPIOA, please refer to 0x76000014 for more details */
	MMC_WRITE(0x76000014, 0x1FFF0004);
	
	/* enable SDIO clock */
	tmp = MMC_READ(0x77000000);
	tmp |= (0x1 << 25);
	MMC_WRITE(0x77000000, tmp);

	/* do software reset */
	MMC_WRITE(0x77000004, 0xffffffff);	
	tmp = MMC_READ(0x77000004);
	tmp &= ~(0x1 << 25);
	MMC_WRITE(0x77000004, tmp);
	tmp |= (0x1 << 25);
	MMC_WRITE(0x77000004, tmp);
	udelay(2000);
}
#endif
	if (verbose)
		printf("mmc_init\n");

#if 0 // do not rely on inconsistant card detect signal 
printf("0x%08x=0x%08x\n", CNS3000_VEGA_SDIO_BASE+0x24, MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24));
	if (!((MMC_READ_REGl(CNS3000_VEGA_SDIO_BASE + 0x24) >> 16) & 0x01))
		return ret;
#endif
		
	/* power on SD bus, 3.3V */
	MMC_WRITE_REGb((CNS3000_VEGA_SDIO_BASE + 0x29), 0x0f);

	/* set clock divider, assume base clock is 30Mhz. Base clock. */
	#if 0
	MMC_WRITE_REGb((CNS3000_VEGA_SDIO_BASE + 0x2d), 0x00);
	#else
	/* FIXME: this divider should be determined by HCLK
		the maximum SD clock is 25MHz (normal speed mode)
								50MHz (high speed mode)
		formula => SD clock = (HCLK / divisor) 
 	 */
	MMC_WRITE_REGb((CNS3000_VEGA_SDIO_BASE + 0x2d), 0x04);
	#endif

	/* enable clock */
	MMC_WRITE_REGb((CNS3000_VEGA_SDIO_BASE + 0x2c), 0x05);

	/* 4 bit transfer width */
	MMC_WRITE_REGb((CNS3000_VEGA_SDIO_BASE + 0x28), (0x01 << 1));
	
	/* set timeout to 1024K cycles */
	MMC_WRITE_REGb((CNS3000_VEGA_SDIO_BASE + 0x2e), 0x07);

	/* enable read wait for host */
	MMC_WRITE_REGb((CNS3000_VEGA_SDIO_BASE + 0x2a), 0x04);

	udelay(2000);

	/* send CMD0 to start, reset and initialize all cards */
	if ((ret = send_cmd0()) != 0) 
		return -FAIL_IN_COMMAND0;

	/* send CMD8 to see if card is version 2 */
	if ((ret = send_cmd8()) < 0) 
		return -FAIL_IN_COMMAND8;

	version_2 = ret;
#ifdef MMC_DEBUG
	printf("version_2 = %d \n", version_2);
#endif

	/* send ACMD41 until card is ready */
	for (i = 0; i < 2000; i++) {
		ret = send_acmd41(resp);
		udelay(20000);
		if (ret || (resp[0] & 0x80000000)) {
			mmc_dev.if_type = IF_TYPE_MMC;
			printf("Detected SD Card\n");
			break;
		}
		ret = -ETIMEDOUT;
	}
	if (ret) 
		return ret;

	/* send CMD2 to read CID */
	if ((ret = send_cmd2(resp)) != 0) 
		return -FAIL_IN_COMMAND2;

	ret = sd_parse_cid(&cid, resp);
	if (ret) 
		return ret;

#ifdef MMC_DEBUG
	if (verbose)
		mmc_dump_cid(&cid);
#endif

	/* send CMD3 to publish RCA */
	if ((ret = send_cmd3(&mmc_rca)) != 0) 
		return -FAIL_IN_COMMAND3;

#ifdef MMC_DEBUG
	if (verbose)
		printf("MMC/SD RCA = 0x%04X\n", mmc_rca);
#endif

	/* send CMD9 to read CSD */
	if ((ret = issue_simple_cmd(0x09, 0x01, 0x00, 0x00, 0x01, (mmc_rca & 0xFFFF) << 16, resp)) != 0)
		return -FAIL_IN_COMMAND9;


	ret = sd_parse_csd(&csd, resp);
	if (ret) 
		return ret;

#ifdef MMC_DEBUG
	if (verbose)
		mmc_dump_csd(&csd);
#endif

	/* send CMD7 to put card to trans state */
	if ((ret = send_cmd7(mmc_rca)) != 0)
		return -FAIL_IN_COMMAND7;

	/* send ACMD6 to set bus width */
	if ((ret = send_acmd6(mmc_rca)) != 0) 
		return -FAIL_IN_ACMD6;


	/* Card is now ready for data transfer */

	/* Initialize the block device structure */
	sprintf((char *)mmc_dev.vendor, "Man %02x%04x Snr %08lx", cid.mid, cid.oid, cid.psn);
	strncpy((char *)mmc_dev.product, cid.pnm, sizeof(mmc_dev.product));
	sprintf((char *)mmc_dev.revision, "%x %x", cid.prv >> 4, cid.prv & 0x0f);

	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.lun = 0;
	mmc_dev.type = DEV_TYPE_HARDDISK;
	mmc_dev.removable = 1;
	mmc_dev.blksz = MMC_BLOCK_SIZE;
	mmc_dev.lba = ((int) csd.c_size + 1) * 1024;
	mmc_dev.block_read = mmc_block_read;
	mmc_dev.block_write = mmc_block_write;
	
	/* FAT file system */
	if (fat_register_device(&mmc_dev, 1))
		printf("Could not register MMC/SD fat device\n");

	ret = 0;

	return ret;
}

int mmc_read(ulong src, uchar *dst, int size)
{
	/* Since mmc2info always returns 0 this function will never be called */

	return -ENOSYS;
}

int mmc_write(uchar *src, ulong dst, int size)
{
	/* Since mmc2info always returns 0 this function will never be called */

	return -ENOSYS;
}

int mmc2info(ulong addr)
{
	/* This function is used by cmd_cp to determine if source or destination address resides on MMC/SD card or not. */

	return 0;
}

#endif /* CONFIG_MMC */
