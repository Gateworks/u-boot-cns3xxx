/*
 * Copyright (c) 2008 Cavium Networks 
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

#ifndef __MMC_CNS3000_P_H__
#define __MMC_CNS3000_P_H__

#define MMC_BLOCK_SIZE		512

#define MMC_RSP_PRESENT		(1 << 0)
#define MMC_RSP_136		(1 << 1)		/* 136 bit response */
#define MMC_RSP_CRC		(1 << 2)		/* expect valid crc */
#define MMC_RSP_BUSY		(1 << 3)		/* card may send busy */
#define MMC_RSP_OPCODE		(1 << 4)		/* response contains opcode */

#define MMC_RSP_NONE		(0)
#define MMC_RSP_R1		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE|MMC_RSP_BUSY)
#define MMC_RSP_R2		(MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3		(MMC_RSP_PRESENT)
#define MMC_RSP_R4		(MMC_RSP_PRESENT)
#define MMC_RSP_R5		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

struct mmc_cid {
	unsigned char	mid;
	unsigned short	oid;
	char		pnm[7];
	unsigned char	prv;
	unsigned char	mdt;
	unsigned long	psn;
};

struct mmc_csd {
	unsigned char	csd_structure;
	unsigned char	taac;
	unsigned char	nsac;
	unsigned char	tran_speed;
	unsigned short	ccc;
	unsigned char	read_bl_len;
	unsigned char	read_bl_partial;
	unsigned char	write_blk_misalign;
	unsigned char	read_blk_misalign;
	unsigned char	dsr_imp;
	unsigned short	c_size;
	unsigned char	vdd_r_curr_min;		/* v1 only */
	unsigned char	vdd_r_curr_max;		/* v1 only */
	unsigned char	vdd_w_curr_min;		/* v1 only */
	unsigned char	vdd_w_curr_max;		/* v1 only */
	unsigned char	c_size_mult;		/* v1 only */
	unsigned char	erase_blk_en;
	unsigned char	sector_size;
	unsigned char	wp_grp_size;
	unsigned char	wp_grp_enable;
	unsigned char	default_ecc;		/* v1 only */
	unsigned char	r2w_factor;
	unsigned char	write_bl_len;
	unsigned char	write_bl_partial;
	unsigned char	content_prot_app;	/* v1 only */
	unsigned char	file_format_grp;
	unsigned char	copy;
	unsigned char	perm_write_protect;
	unsigned char	tmp_write_protect;
	unsigned char	file_format;
	unsigned char	ecc;			/* v1 only */
};

struct mmc_cmd {
	unsigned short	cmdidx;
	unsigned int	resp_type;
	unsigned int	cmdarg;
	char		response[18];
	unsigned int	flags;
};

struct mmc_data {
	union {
		char *dest;
		const char *src;		/* src buffers don't get written to */
	};
	unsigned int flags;
	unsigned int blocks;
	unsigned int blocksize;
};

#endif /* __MMC_CNS3000_P_H__ */

