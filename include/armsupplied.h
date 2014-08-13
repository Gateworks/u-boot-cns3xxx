/*
 * (C) Copyright 2006
 * ARM Ltd.
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



#ifndef __ARMSUPPLIED_
/* 
 *MACROS common to all ARM supplied development boards 
 */
/*
 *  Obtain board revision info from board register
 */
#define ARM_SUPPLIED_GET_BOARD_REV			\
u32 get_board_rev(void){				\
	u32 * rev_reg = (u32*)ARM_SUPPLIED_REVISION_REGISTER; \
	return(*rev_reg);				\
}
#define CONFIG_REVISION_TAG
#define __ARMSUPPLIED_
#endif /* __ARMSUPPLIED_ */

