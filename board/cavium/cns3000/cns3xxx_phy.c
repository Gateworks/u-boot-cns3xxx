/*******************************************************************************
 *
 *
 *   Copyright (c) 2009 Cavium Networks 
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
1*   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *   more details.
 *
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59
 *   Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *   The full GNU General Public License is included in this distribution in the
 *   file called LICENSE.
 *
 ********************************************************************************/

#include "cns3xxx_phy.h"
#include "cns3xxx_symbol.h"

#include <common.h>
#include "cns3xxx_switch_type.h"

// mac_port: 0, 1, 2
int cns3xxx_enable_mac_clock(u8 mac_port, u8 en)
{
	switch (mac_port) 
	{
		case 0:
		{
			(en==1)?(PHY_AUTO_ADDR_REG |= 1 << 7) :(PHY_AUTO_ADDR_REG &= (~(1 << 7)) );
			break;
		}
		case 1:
		{
			(en==1)?(PHY_AUTO_ADDR_REG |= (1 << 15)) :(PHY_AUTO_ADDR_REG &= (~(1 << 15)) );
			break;
		}
		case 2:
		{
			(en==1)?(PHY_AUTO_ADDR_REG |= (1 << 23)) :(PHY_AUTO_ADDR_REG &= (~(1 << 23)) );
			break;
		}
	}

	return CAVM_OK;
}

// dis: 1 disable
// dis: 0 enable
int cns3xxx_phy_auto_polling_enable(u8 port, u8 en)
{
	u8 phy_addr[]={5, 13, 21};

	PHY_AUTO_ADDR_REG &= (~(1 << phy_addr[port]));
	if (en) {
		PHY_AUTO_ADDR_REG |= (1 << phy_addr[port]);
	}
	return CAVM_OK;
}

// dis: 1 disable
// dis: 0 enable
int cns3xxx_mdc_mdio_disable(u8 dis)
{

	PHY_CTRL_REG &= (~(1 << 7));
	if (dis) {
		PHY_CTRL_REG |= (1 << 7);
	}
	return CAVM_OK;
}


static int cns3xxx_phy_auto_polling_conf(int mac_port, u8 phy_addr)
{
	if ( (mac_port < 0) || (mac_port > 2) ) {
		return CAVM_ERR;
	}

	switch (mac_port) 
	{
		case 0:
		{
			PHY_AUTO_ADDR_REG &= (~0x1f);
			PHY_AUTO_ADDR_REG |= phy_addr;
			break;
		}
		case 1:
		{
			PHY_AUTO_ADDR_REG &= (~(0x1f << 8));
			PHY_AUTO_ADDR_REG |= (phy_addr << 8);
			break;
		}
		case 2:
		{
			PHY_AUTO_ADDR_REG &= (~(0x1f << 16));
			PHY_AUTO_ADDR_REG |= (phy_addr << 16);
			break;
		}
	}
	cns3xxx_phy_auto_polling_enable(mac_port, 1);
	return CAVM_OK;
}


int cns3xxx_read_phy(u8 phy_addr, u8 phy_reg, u16 *read_data)
{
	int delay=0;
	u32 volatile tmp = PHY_CTRL_REG;

	PHY_CTRL_REG |= (1 << 15); // clear "command completed" bit
	// delay
	for (delay=0; delay<10; delay++);

	tmp &= (~0x1f);
	tmp |= phy_addr;

	tmp &= (~(0x1f << 8));
	tmp |= (phy_reg << 8);

	tmp |= (1 << 14); // read command
	PHY_CTRL_REG = tmp;

	// wait command complete
	while ( ((PHY_CTRL_REG >> 15) & 1) == 0);

	*read_data = (PHY_CTRL_REG >> 16);

	PHY_CTRL_REG |= (1 << 15); // clear "command completed" bit
//	printf("read_phy(%d, 0x%0x)=%x\n", phy_addr, phy_reg, *read_data);

	return CAVM_OK;
}

int phy_read_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u16 phyReg;

	cns3xxx_read_phy(simple_strtoul(argv[1], NULL, 16), 
										simple_strtoul(argv[2], NULL, 16), &phyReg);
	printf ("0x%x\n", phyReg);

	return 1;
}

U_BOOT_CMD(
	phyRead,      3,     3,      phy_read_cmd,
	"phyRead	- Read Phy register\n",
	" Phy_address Phy_offset. \n"
	"\tRead the Phy register. \n"
);

int cns3xxx_write_phy(u8 phy_addr, u8 phy_reg, u16 write_data)
{
	int delay=0;
	u32 tmp = PHY_CTRL_REG;

	PHY_CTRL_REG |= (1 << 15); // clear "command completed" bit
	// delay
	for (delay=0; delay<10; delay++);

	tmp &= (~(0xffff << 16));
	tmp |= (write_data << 16);

	tmp &= (~0x1f);
	tmp |= phy_addr;

	tmp &= (~(0x1f << 8));
	tmp |= (phy_reg << 8);

	tmp |= (1 << 13); // write command

	PHY_CTRL_REG = tmp;

	// wait command complete
	while ( ((PHY_CTRL_REG >> 15) & 1) == 0);


	return CAVM_OK;
}

int phy_write_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

	cns3xxx_write_phy(simple_strtoul( argv[1], NULL, 16 ),
					 simple_strtoul( argv[2], NULL, 16 ),
					 simple_strtoul( argv[3], NULL, 16 ));

	return 1;
}

U_BOOT_CMD(
	phyWrite,      4,     4,      phy_write_cmd,
	"phyWrite	- Write Phy register\n",
	" Phy_address Phy_offset value.\n"
	"\tWrite to the Phy register.\n"
);


int cns3xxx_std_phy_power_down(int phy_addr, int y)
{
        u16 phy_data = 0;
        // power-down or up the PHY
        cns3xxx_read_phy(phy_addr, 0, &phy_data);
        if (y==1) // down
                phy_data |= (0x1 << 11);
        if (y==0) // up
                phy_data &= (~(0x1 << 11));
        cns3xxx_write_phy(phy_addr, 0, phy_data);

	phy_data=0;
        cns3xxx_read_phy(phy_addr, 0, &phy_data);

        return 0;
}

u32 get_phy_id(u8 phy_addr)
{
	u32 id;
	u16 read_data;

	cns3xxx_read_phy(phy_addr, 2, &read_data);
	id = read_data << 16;
	cns3xxx_read_phy(phy_addr, 1, &read_data);
	id |= read_data;

	return id;
}

int cns3xxx_config_macphy(u8 mac_port, u8 phy_addr)
{
        u16 phy_data=0;
	u32 mac_port_config=0;
	u32 phy_id=0;
	
	cns3xxx_mdc_mdio_disable(0);

	// software reset
	cns3xxx_read_phy(phy_addr, 0, &phy_data);
	phy_data |= (0x1 << 15); 
	cns3xxx_write_phy(phy_addr, 0, phy_data);
	udelay(10);

	phy_id = get_phy_id(phy_addr);
	printf ("phy%d:%d=0x%0x ", mac_port, phy_addr, phy_id);

	// get mac config for port
	switch (mac_port)
	{
		case 0:
		{
	        	mac_port_config = MAC0_CFG_REG;
			break;
		}
		case 1:
		{
	        	mac_port_config = MAC1_CFG_REG;
			break;
		}
		case 2:
		{
	        	mac_port_config = MAC2_CFG_REG;
			break;
		}
	}
		
	cns3xxx_enable_mac_clock(mac_port, 1);

	// If mac AN turns on, auto polling needs to turn on.
	mac_port_config |= (0x1 << 7); // enable PHY's AN
	cns3xxx_phy_auto_polling_conf(mac_port, phy_addr); 
	
	// enable GSW MAC port 0
	mac_port_config &= ~(0x1 << 18);

	// normal MII
	mac_port_config &= ~(0x1 << 14);

	switch(phy_id >> 16) {
	case 0x0143: // Broadcom BCM5481
		mac_port_config |= (0x1 << 15); // RGMII-PHY mode
		// config tx/rx delays and LED's
		cns3xxx_write_phy(phy_addr, 0x18, 0xf1e7);
		//cns3xxx_write_phy(phy_addr, 0x18, 0x4400);
		cns3xxx_write_phy(phy_addr, 0x1c, 0x8e00);
		//cns3xxx_write_phy(phy_addr, 0x10, 0x21);
		cns3xxx_write_phy(phy_addr, 0x10, 0x20);
		cns3xxx_write_phy(phy_addr, 0x1c, 0xa41f);
		cns3xxx_write_phy(phy_addr, 0x1c, 0xb41a);
		cns3xxx_write_phy(phy_addr, 0x1c, 0xb863);
		cns3xxx_write_phy(phy_addr, 0x17, 0xf04);
		cns3xxx_write_phy(phy_addr, 0x15, 0x1);
		cns3xxx_write_phy(phy_addr, 0x17, 0x0);
		break;

	case 0x2000: // National DP83848
		mac_port_config &= ~(0x1 << 16); // 10/100 Mbps
		mac_port_config &= ~(0x1 << 15); // MII-PHY mode
		break;

	default:
		printf ("no supported phys detected ");
		return CAVM_ERR;
	}
	//printf ("MAC%d_CFG=0x%04x\n", mac_port, mac_port_config);

	// set mac config for port
	switch (mac_port)
	{
		case 0:
		{
			MAC0_CFG_REG = mac_port_config;
			break;
		}
		case 1:
		{
			MAC1_CFG_REG = mac_port_config;
			break;
		}
		case 2:
		{
			MAC2_CFG_REG = mac_port_config;
			break;
		}
	}
}

