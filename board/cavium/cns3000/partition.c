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
 *
 */
#include <common.h>
#include <config.h>
#include <asm/hardware.h>
#include <dataflash.h>

AT91S_DATAFLASH_INFO dataflash_info[CFG_MAX_DATAFLASH_BANKS];

struct dataflash_addr cs[CFG_MAX_DATAFLASH_BANKS] = {
	{CFG_DATAFLASH_LOGIC_ADDR_CS0, 0},	/* Logical adress, CS */
};

/*define the area offsets*/
dataflash_protect_t area_list[NB_DATAFLASH_AREA] = {
	{0x00000000, 0x0003FFFF, FLAG_PROTECT_CLEAR, 0, "U-boot"},
	{0x00040000, 0x0007FFFF, FLAG_PROTECT_CLEAR, 0, "Environment"},
	{0x00080000, 0x001fFFFF, FLAG_PROTECT_CLEAR, 0,	"linux"},
	{0x00200000, 0x00FFFFFF, FLAG_PROTECT_CLEAR, 0, "rootfs"},
};
