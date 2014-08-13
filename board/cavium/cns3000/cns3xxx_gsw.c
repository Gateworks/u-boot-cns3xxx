/*******************************************************************************
 *
 *   Copyright (c) 2009 Cavium Networks 
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
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

#include <common.h>
#include <net.h> // for NetReceive()

#include "cns3xxx_gsw.h"
#include "cns3xxx_phy.h"
#include "cns3xxx_symbol.h"
#include "cns3xxx_switch_type.h"
#include "cns3xxx_tool.h"

#define PORT0_PVID 1
#define PORT1_PVID 2
#define PORT2_PVID 3
#define CPU_PVID 5

#define TIMEOUT 50000

#define CPU_CACHE_64BYTES		64
#define CPU_CACHE_64ALIGN(X)	(((X) + (CPU_CACHE_64BYTES-1)) & ~(CPU_CACHE_64BYTES-1))

#define CPU_CACHE_BYTES		32
#define CPU_CACHE_ALIGN(X)	(((X) + (CPU_CACHE_BYTES-1)) & ~(CPU_CACHE_BYTES-1))
//#define PRINT(args...) printf(args)
#define PRINT(args...) 

// Prototypes
static int cns3xxx_eth_load(int port, char *name, char *enet_addr);

u16 tx_pmap[3];
u8 linkup_port[3];
static GswDev gsw_dev;

static RXDesc *rx_desc_ring;
static TXDesc *tx_desc_ring;
static pkt_t *pkt_pool;
static u8 *pkt_buffer_pool;
static u8 mem_alloc_ok;

typedef struct _cns3xxx_eth_priv
{
	int port;
	int dev_init;
} cns3xxx_eth_priv;

static void cns3xxx_gsw_tx_pkt_enqueue(GswDev *dev, pkt_t *pkt)
{
	if (dev->tx_pkt_q_tail) {
		dev->tx_pkt_q_tail->next = pkt;
	}
	dev->tx_pkt_q_tail = pkt;

	if (dev->tx_pkt_q_head == 0) {
		dev->tx_pkt_q_head = pkt;
	}
	dev->tx_pkt_q_count++;
}

static pkt_t *cns3xxx_gsw_tx_pkt_dequeue(GswDev *dev)
{
	pkt_t *pkt;

	pkt = dev->tx_pkt_q_head;
	if (pkt) {
		dev->tx_pkt_q_head = pkt->next;
		pkt->next = 0;
		if (dev->tx_pkt_q_head == 0) {
			dev->tx_pkt_q_tail = 0;
		}
		dev->tx_pkt_q_count--;
	}

	return pkt;
}

static void cns3xxx_gsw_tx_pkt_requeue(GswDev *dev, pkt_t *pkt)
{
	pkt->next = dev->tx_pkt_q_head;
	dev->tx_pkt_q_head = pkt;
	if (dev->tx_pkt_q_tail == 0) {
		dev->tx_pkt_q_tail = pkt;
	}
	dev->tx_pkt_q_count++;
}

static inline pkt_t *alloc_pkt(GswDev *dev)
{
	pkt_t *pkt=0;

	pkt = dev->free_pkt_list;
	if (pkt) {
		dev->free_pkt_list = pkt->next;
		pkt->next = 0;
		dev->free_pkt_count--;
	}

	return pkt;
}

static inline void free_pkt(GswDev *dev, pkt_t *pkt)
{
	pkt->next = dev->free_pkt_list;
	dev->free_pkt_list = pkt;
	dev->free_pkt_count++;
}

static inline pkt_t *rx_alloc_pkt(GswDev *dev)
{
	pkt_t *pkt=0;

	pkt = dev->rx_free_pkt_list;
	if (pkt) {
		dev->rx_free_pkt_list = pkt->next;
		pkt->next = 0;
		dev->rx_free_pkt_count--;
	}

	return pkt;
}

static inline void rx_free_pkt(GswDev *dev, pkt_t *pkt)
{
	pkt->next = dev->rx_free_pkt_list;
	dev->rx_free_pkt_list = pkt;
	dev->rx_free_pkt_count++;
}

static void free_mem(void)
{
	if (tx_desc_ring)
		free(tx_desc_ring);
	if (rx_desc_ring)
		free(rx_desc_ring);
	if (pkt_pool)
		free(pkt_pool);
	if (pkt_buffer_pool)
		free(pkt_buffer_pool);
}

int setup_pkt_pool(void)
{
	pkt_pool = (pkt_t *)malloc(sizeof(pkt_t) * NUM_PKT_BUFFER);
	if (pkt_pool == 0) {
		goto err_out;
	}

	pkt_buffer_pool = (u8 *)malloc(PKT_BUFFER_ALLOC_SIZE * NUM_PKT_BUFFER);
	if (pkt_buffer_pool == 0) {
		goto err_out;
	}

	return CAVM_OK;
err_out:
	free_mem();
	return CAVM_ERR;
}


static int cns3xxx_alloc_mem(GswDev *dev)
{
	
	tx_desc_ring = (TXDesc *)malloc(sizeof(TXDesc) * TX_SIZE + CPU_CACHE_BYTES);
	if (tx_desc_ring == 0) {
		goto err_out;
	}

	rx_desc_ring = (RXDesc *)malloc(sizeof(RXDesc) * RX_SIZE + CPU_CACHE_BYTES);
	if (rx_desc_ring == 0) {
		goto err_out;
	}

	pkt_pool = (pkt_t *)malloc(sizeof(pkt_t) * NUM_PKT_BUFFER);
	if (pkt_pool == 0) {
		goto err_out;
	}

	pkt_buffer_pool = (u8 *)malloc(PKT_BUFFER_ALLOC_SIZE * NUM_PKT_BUFFER);
	if (pkt_buffer_pool == 0) {
		goto err_out;
	}

	dev->rx_pkt_pool = (pkt_t *)malloc(sizeof(pkt_t) * NUM_PKT_BUFFER);
	if (dev->rx_pkt_pool == 0) {
		goto err_out;
	}

	dev->rx_pkt_buffer_pool = (u8 *)malloc(PKT_BUFFER_ALLOC_SIZE * NUM_PKT_BUFFER);
	dev->rx_pkt_buffer_pool = (u8 *)CPU_CACHE_64ALIGN((u32)dev->rx_pkt_buffer_pool);

	if (dev->rx_pkt_buffer_pool == 0) {
		goto err_out;
	}



	mem_alloc_ok = 1;

	return CAVM_OK;

err_out:
	printf("alloc memory fail\n");
	free_mem();

	return CAVM_ERR;
}

int cns3xxx_init_mem(GswDev *dev)
{
	int i;

	dev->tx_ring = (TXDesc *)CPU_CACHE_ALIGN((u32)tx_desc_ring);
	dev->rx_ring = (RXDesc *)CPU_CACHE_ALIGN((u32)rx_desc_ring);
	memset(dev->tx_ring, 0, sizeof(TXDesc) * TX_SIZE);
	memset(dev->rx_ring, 0, sizeof(RXDesc) * RX_SIZE);
	dev->tx_ring[TX_SIZE - 1].eor = 1;
	dev->rx_ring[RX_SIZE - 1].eor = 1;

	memset(dev->tx_ring_pkt, 0, sizeof(pkt_t *) * TX_SIZE);
	memset(dev->rx_ring_pkt, 0, sizeof(pkt_t *) * RX_SIZE);

	dev->pkt_pool = pkt_pool;
	memset(dev->pkt_pool, 0, sizeof(pkt_t) * NUM_PKT_BUFFER);

	dev->pkt_buffer_pool = pkt_buffer_pool;


	dev->free_pkt_list = &dev->pkt_pool[0];

	dev->rx_free_pkt_list = &dev->rx_pkt_pool[0];

	for (i = 0; i < (NUM_PKT_BUFFER - 1); i++) {
		dev->pkt_pool[i].next = &dev->pkt_pool[i + 1];
		dev->pkt_pool[i].pkt_buffer = dev->pkt_buffer_pool + (i * PKT_BUFFER_ALLOC_SIZE);

		dev->rx_pkt_pool[i].next = &dev->rx_pkt_pool[i + 1];
		dev->rx_pkt_pool[i].pkt_buffer = dev->rx_pkt_buffer_pool + (i * PKT_BUFFER_ALLOC_SIZE);
	}

	dev->pkt_pool[i].next = 0;
	dev->pkt_pool[i].pkt_buffer = dev->pkt_buffer_pool + (i * PKT_BUFFER_ALLOC_SIZE);

	dev->rx_pkt_pool[i].next = 0;
	dev->rx_pkt_pool[i].pkt_buffer = dev->rx_pkt_buffer_pool + (i * PKT_BUFFER_ALLOC_SIZE);

	for (i = 0; i < TX_SIZE; i++) {
		dev->tx_ring[i].cown = 1;
		dev->tx_ring[i].ico = 0;
		dev->tx_ring[i].uco = 0;
		dev->tx_ring[i].tco = 0;
	}
	
	for (i = 0; i < RX_SIZE; i++) {
		dev->rx_ring_pkt[i] = rx_alloc_pkt(dev);
		dev->rx_ring[i].sdp = (u32)dev->rx_ring_pkt[i]->pkt_buffer;
		dev->rx_ring[i].sdl = PKT_BUFFER_SIZE;
		dev->rx_ring[i].cown = 0;
	}

	dev->tx_pkt_q_head = 0;
	dev->tx_pkt_q_tail = 0;

	dev->cur_tx_desc_idx	= 0;
	dev->cur_rx_desc_idx	= 0;
	dev->tx_pkt_count	= 0;
	dev->rx_pkt_count	= 0;
	dev->free_pkt_count	= NUM_PKT_BUFFER;
	dev->tx_pkt_q_count	= 0;

	return 0;
}

static int cns3xxx_hw_init(GswDev *dev)
{
	u32 reg_config;
	int i;

	static u8 my_vlan0_mac[6] = {0x00, 0x01, 0x22, 0x33, 0x44, 0x99};
	static u8 my_vlan1_mac[6] = {0x00, 0x01, 0xbb, 0xcc, 0xdd, 0x60};
	static u8 my_vlan2_mac[6] = {0x00, 0x01, 0xbb, 0xcc, 0xdd, 0x22};
	static u8 my_vlan3_mac[6] = {0x00, 0x01, 0xbb, 0xcc, 0xdd, 0x55};

	static VLANTableEntry cpu_vlan_table_entry = {0, 1, CPU_PVID, 0, 0, MAC_PORT0_PMAP | MAC_PORT1_PMAP | MAC_PORT2_PMAP | CPU_PORT_PMAP, my_vlan3_mac}; // for cpu

	static VLANTableEntry vlan_table_entry[] =
	{
		// vlan_index; valid; vid; wan_side; etag_pmap; mb_pmap; *my_mac;
		{1, 1, PORT0_PVID, 0, 0, MAC_PORT0_PMAP | MAC_PORT2_PMAP | CPU_PORT_PMAP, my_vlan0_mac},
		{2, 1, PORT1_PVID, 1, 0, MAC_PORT1_PMAP | MAC_PORT2_PMAP  | CPU_PORT_PMAP, my_vlan1_mac},
		{3, 1, PORT2_PVID, 1, 0, MAC_PORT2_PMAP  | CPU_PORT_PMAP, my_vlan2_mac},
	};

	GPIOB_PIN_EN_REG |= (1 << 20); //enable MDC
	GPIOB_PIN_EN_REG |= (1 << 21); //enable MDIO

	/* enable the gsw */
	cns3xxx_gsw_power_enable();

	/* software reset the gsw */
	cns3xxx_gsw_software_reset();

	while (1) {
		if (((SRAM_TEST_REG >> 20) & 1) == 1) {
			break;
		}
	}

	cns3xxx_vlan_table_add(&cpu_vlan_table_entry);
	for (i=0 ; i < (sizeof(vlan_table_entry)/sizeof(VLANTableEntry)) ; ++i) {
		cns3xxx_vlan_table_add(&vlan_table_entry[i]);
	}

	cns3xxx_set_pvid(0, PORT0_PVID);
	cns3xxx_set_pvid(1, PORT1_PVID);
	cns3xxx_set_pvid(2, PORT2_PVID);
	cns3xxx_set_pvid(3, CPU_PVID);

	/* configure DMA descriptors */
	FS_DESC_PTR0_REG = (u32)dev->rx_ring;
	FS_DESC_BASE_ADDR0_REG = (u32)dev->rx_ring;

	TS_DESC_PTR0_REG = (u32)dev->tx_ring;
	TS_DESC_BASE_ADDR0_REG = (u32)dev->tx_ring;

	// configure MAC global
	reg_config = MAC_GLOB_CFG_REG;

	/* disable aging */
	reg_config &= ~0xf;


	// disable IVL, use SVL
	reg_config &= ~(1 << 7);

	// use IVL
	reg_config |= (1 << 7);

	// disable HNAT
	reg_config &= ~(1 << 26);

	// receive CRC error frame
	reg_config |= (1 << 21);

	MAC_GLOB_CFG_REG = reg_config;
	
	// max packet len 1536
	reg_config = PHY_AUTO_ADDR_REG;
	reg_config &= ~(3 << 30);
	reg_config |= (2 << 30);
	PHY_AUTO_ADDR_REG = reg_config;

	/* configure cpu port */
	reg_config = CPU_CFG_REG;

	// disable CPU port SA learning
	reg_config |= (1 << 19);

	// DMA 4N mode
	reg_config |= (1 << 30);

	CPU_CFG_REG = reg_config;

	return CAVM_OK;
}

static int cns3xxx_gsw_init(GswDev *dev)
{
	int err;


	// init tx pmap array
	tx_pmap[0] = 1; // port 0
	tx_pmap[1] = (1 << 1); // port 1
	tx_pmap[2] = (1 << 4); // port 2

	// stop ring0, ring1 TX/RX DMA
	enable_rx_dma(0, 0);
	enable_tx_dma(0, 0);
	enable_rx_dma(1, 0);
	enable_tx_dma(1, 0);

	if (!mem_alloc_ok) {
		err = cns3xxx_alloc_mem(dev);
		if (err) {
			return err;
		}
	}

	cns3xxx_init_mem(dev);
	
	err = cns3xxx_hw_init(dev);

	// GIGA mode off
	MAC0_CFG_REG &= (~(1<<16));
	MAC1_CFG_REG &= (~(1<<16));
	MAC2_CFG_REG &= (~(1<<16));

	// GIGA mode off
	MAC0_CFG_REG |= (1<<16);
	MAC1_CFG_REG |= (1<<16);
	MAC2_CFG_REG |= (1<<16);

	if (err != CAVM_OK) {
		memset(dev, 0, sizeof(GswDev));
		return err;
	}

	return CAVM_OK;
}


static void cns3xxx_gsw_tx(GswDev *dev)
{
	TXDesc volatile *tx_desc = 0;
	pkt_t *pkt = 0;
	u32 txcount = 0;
	int i=0;

	while ((pkt = cns3xxx_gsw_tx_pkt_dequeue(dev))) {
		tx_desc = &dev->tx_ring[dev->cur_tx_desc_idx];
		if (!tx_desc->cown) {
			cns3xxx_gsw_tx_pkt_requeue(dev, pkt);
			break;
		} else {
			if (dev->tx_ring_pkt[dev->cur_tx_desc_idx]) {
				free_pkt(dev, dev->tx_ring_pkt[dev->cur_tx_desc_idx]);
			}
		}
		dev->tx_ring_pkt[dev->cur_tx_desc_idx] = pkt;

		tx_desc->sdp = (u32)pkt->pkt_buffer;

		tx_desc->sdl = pkt->length;

		tx_desc->cown = 0;
		tx_desc->fsd = 1;
		tx_desc->lsd = 1;
		tx_desc->fr = 1;

		if (dev->which_port == -1) {
			for (i=0 ; i < 3 ; ++i) {
				if (linkup_port[i] == 1) {
					tx_desc->pmap = tx_pmap[i];
					break;
				}
			}
		} else {
			tx_desc->pmap = tx_pmap[dev->which_port];
		}

		// point to next index

		dev->cur_tx_desc_idx = ((dev->cur_tx_desc_idx + 1) % TX_SIZE);


		txcount++;
	}

	dev->tx_pkt_count += txcount;

	enable_tx_dma(0, 1);
}

static int cns3xxx_eth_send(struct eth_device *edev, volatile void *packet, s32 length)
{
	GswDev *dev = &gsw_dev;
	pkt_t *pkt;

	pkt = alloc_pkt(dev);
	if (!pkt) {
		printf("Allocate pkt failed on TX...\n");
		return 0;
	}

	memcpy(pkt->pkt_buffer, (void *)packet, length);
	if (length < PKT_MIN_SIZE) {
		pkt->length = PKT_MIN_SIZE;
		memset(pkt->pkt_buffer + length, 0x00, PKT_MIN_SIZE - length);
	} else {
		pkt->length = length;
	}
	cns3xxx_gsw_tx_pkt_enqueue(dev, pkt);
	cns3xxx_gsw_tx(dev);

	return 0;
}

u32 get_rx_hw_index(void)
{
	return (FS_DESC_PTR0_REG - FS_DESC_BASE_ADDR0_REG)/32;
}

// hw_index, sw_index begin from 0
int get_rx_count(u32 hw_index, u32 sw_index, u8 cown)
{
	if (hw_index > sw_index)
		return (hw_index - sw_index);
	else if (hw_index < sw_index)
		return ((RX_SIZE) - sw_index + hw_index + 1);
	else 
		if (cown == 0) { // this should not happen
			return -1;
		} else {
			return (RX_SIZE);
		}
}

static void cns3xxx_gsw_rx(GswDev *dev)
{
	RXDesc volatile *rx_desc=0;
	pkt_t *rcvpkt;
	pkt_t *newpkt;
	u32 rxcount = 0;

	while (1) {
		u32 hw_index = get_rx_hw_index();
		int rx_count=0;

		rx_desc = &dev->rx_ring[dev->cur_rx_desc_idx];

		rx_count = get_rx_count(hw_index, dev->cur_rx_desc_idx, rx_desc->cown);

		if (rx_desc->cown == 0) {
			break;
		}
		rcvpkt = dev->rx_ring_pkt[dev->cur_rx_desc_idx];

		rcvpkt->length = rx_desc->sdl;
		newpkt = rx_alloc_pkt(dev);
		if (newpkt == 0) {
			printf("Allocate pkt failed on RX...\n");
		}
		dev->rx_ring_pkt[dev->cur_rx_desc_idx] = newpkt;
		rx_desc->sdp = (u32)newpkt->pkt_buffer;
		rx_desc->sdl = PKT_BUFFER_SIZE;
		rx_desc->cown = 0;

		NetReceive(rcvpkt->pkt_buffer, rcvpkt->length);
		rx_free_pkt(dev, rcvpkt);

		// point to next index
		
		//dev->cur_rx_desc_idx = ((dev->cur_rx_desc_idx + 2) % RX_SIZE);
		dev->cur_rx_desc_idx = ((dev->cur_rx_desc_idx + 1) % RX_SIZE);

		rxcount++;
		if (rxcount == RX_SIZE) {
			break;
		}
	}

	dev->rx_pkt_count += rxcount;
	enable_rx_dma(0, 1);
}

static int cns3xxx_eth_rx(struct eth_device *edev)
{
	GswDev *dev = &gsw_dev;
	cns3xxx_gsw_rx(dev);

	return 0;
}

static int cns3xxx_eth_halt(struct eth_device *dev)
{
	cns3xxx_eth_priv *priv = dev->priv;
	GswDev *cns_dev = &gsw_dev;

	if (priv->dev_init) {
		enable_port(priv->port, 0);
		priv->dev_init = 0;
		cns_dev->which_port = -1;
		enable_rx_dma(0, 0);
		enable_port(3, 0);
	}
	return 0;
}

static int cns3xxx_eth_init(struct eth_device *dev, bd_t *p)
{
	cns3xxx_eth_priv *priv = dev->priv;
	GswDev *cns_dev = &gsw_dev;
	u16 phy_data;
	u32 timeout = 0;

	if (priv->dev_init) {
		return 0;
	}

	enable_port(priv->port, 1);

	enable_rx_dma(0, 1);
	enable_port(3, 1);

	priv->dev_init = 1;
	cns_dev->which_port = priv->port;

	// wait for link or timeout
	do {
		cns3xxx_read_phy(priv->port, 1, &phy_data);
		++timeout;
		if (timeout > TIMEOUT)
			break;
	} while (((phy_data >> 2) & 1) == 0);

	return 0;
}


int cns3xxx_eth_initialize( bd_t *bis)
{
	int port;
	char name[4];
	char enetvar[9];
	char *enet_addr;
	GswDev *dev = &gsw_dev;

	cns3xxx_gsw_init(dev);

	// config the phy
	INIT_PORT0_PHY 
	INIT_PORT1_PHY
	INIT_PORT2_PHY

	//INIT_PORT2_MAC
	
	PORT0_LINK_UP
	PORT1_LINK_UP
	PORT2_LINK_UP

	// start tx/rx DMA operation
	//enable_tx_dma(0, 1);
	//enable_rx_dma(0, 1);

	// Enable the CPU Port
	//enable_port(3, 1);

	for (port = 0; port < 3; port++)
	{
		sprintf(name, "eth%d", port);
		sprintf(enetvar, port ? "eth%daddr" : "ethaddr", port);
		enet_addr = getenv( enetvar);
		
		cns3xxx_eth_load( port, name, enet_addr);
	}
	return 0;
}

static int cns3xxx_eth_load(int port, char *name, char *enet_addr)
{
	int i;
	unsigned char env_enetaddr[6];
	char *end;
	struct eth_device *dev = NULL;
	cns3xxx_eth_priv *priv = NULL;

	dev = malloc( sizeof(struct eth_device) );
	priv = malloc( sizeof(cns3xxx_eth_priv) );

	memset( priv, 0, sizeof(cns3xxx_eth_priv) );

	memcpy( dev->name, name, NAMESIZE );
	for (i=0; i<6; i++) {
		env_enetaddr[i] = enet_addr ? simple_strtoul(enet_addr, &end, 16) : 0;
		if (enet_addr)
			enet_addr = (*end) ? end+1 : end;
	}
	memcpy( dev->enetaddr, env_enetaddr, 6);

  dev->init = (void *)cns3xxx_eth_init;
  dev->halt = (void *)cns3xxx_eth_halt;
  dev->send = (void *)cns3xxx_eth_send;
  dev->recv = (void *)cns3xxx_eth_rx;
  dev->priv = priv;
  dev->iobase = 0;
  priv->port = port;

	eth_register(dev);
	
	return 0;
}
