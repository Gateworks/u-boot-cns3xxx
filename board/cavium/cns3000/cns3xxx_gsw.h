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

#ifndef CNS3XXX_GSW_H
#define CNS3XXX_GSW_H

#define TX_SIZE	(16)
#define RX_SIZE	(16)


#define PKT_BUFFER_ALLOC_SIZE		1600
#define PKT_BUFFER_SIZE			1536
#define PKT_MIN_SIZE			60

#define NUM_PKT_BUFFER			128

#define DEFAULT_VLAN_ID			1

#define GSW_VLAN_PORT_0			(1 << 0)
#define GSW_VLAN_PORT_1			(1 << 1)
#define GSW_VLAN_PORT_CPU		(1 << 2)


typedef struct
{
	int32_t sdp; // segment data pointer

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 cown:1;
	u32 eor:1;
	u32 fsd:1;
	u32 lsd:1;
	u32 interrupt:1;
	u32 fr:1;
	u32 fp:1; // force priority
	u32 pri:3;
	u32 rsv_1:3; // reserve
	u32 ico:1;
	u32 uco:1;
	u32 tco:1;
	u32 sdl:16; // segment data length

#else
	u32 sdl:16; // segment data length
	u32 tco:1;
	u32 uco:1;
	u32 ico:1;
	u32 rsv_1:3; // reserve
	u32 pri:3;
	u32 fp:1; // force priority
	u32 fr:1;
	u32 interrupt:1;
	u32 lsd:1;
	u32 fsd:1;
	u32 eor:1;
	u32 cown:1;
#endif

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 rsv_3:5;
	u32 fewan:1;
	u32 ewan:1;
	u32 mark:3;
	u32 pmap:5;
	u32 rsv_2:9;
	u32 dels:1;
	u32 inss:1;
	u32 sid:4;
	u32 stv:1;
	u32 ctv:1;
#else
	u32 ctv:1;
	u32 stv:1;
	u32 sid:4;
	u32 inss:1;
	u32 dels:1;
	u32 rsv_2:9;
	u32 pmap:5;
	u32 mark:3;
	u32 ewan:1;
	u32 fewan:1;
	u32 rsv_3:5;
#endif

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 s_pri:3;
	u32 s_dei:1;
	u32 s_vid:12;
	u32 c_pri:3;
	u32 c_cfs:1;
	u32 c_vid:12;
#else
	u32 c_vid:12;
	u32 c_cfs:1;
	u32 c_pri:3;
	u32 s_vid:12;
	u32 s_dei:1;
	u32 s_pri:3;
#endif

	u8 alignment[16]; // for alignment 32 byte

} __attribute__((packed)) TXDesc;

typedef struct
{
	u32 sdp;

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 cown:1;
	u32 eor:1;
	u32 fsd:1;
	u32 lsd:1;
	u32 hr :6;
	u32 prot:4;
	u32 ipf:1;
	u32 l4f:1;
	u32 sdl:16;
#else
	u32 sdl:16;
	u32 l4f:1;
	u32 ipf:1;
	u32 prot:4;
	u32 hr :6;
	u32 lsd:1;
	u32 fsd:1;
	u32 eor:1;
	u32 cown:1;
#endif

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 rsv_3:11;
	u32 ip_offset:5;
	u32 rsv_2:1;
	u32 tc:2;
	u32 un_eth:1;
	u32 crc_err:1;
	u32 sp:3;
	u32 rsv_1:2;
	u32 e_wan:1;
	u32 exdv:1;
	u32 iwan:1;
	u32 unv:1;
	u32 stv:1;
	u32 ctv:1;
#else
	u32 ctv:1;
	u32 stv:1;
	u32 unv:1;
	u32 iwan:1;
	u32 exdv:1;
	u32 e_wan:1;
	u32 rsv_1:2;
	u32 sp:3;
	u32 crc_err:1;
	u32 un_eth:1;
	u32 tc:2;
	u32 rsv_2:1;
	u32 ip_offset:5;
	u32 rsv_3:11;
#endif

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 s_pri:3;
	u32 s_dei:1;
	u32 s_vid:12;
	u32 c_pri:3;
	u32 c_cfs:1;
	u32 c_vid:12;
#else
	u32 c_vid:12;
	u32 c_cfs:1;
	u32 c_pri:3;
	u32 s_vid:12;
	u32 s_dei:1;
	u32 s_pri:3;
#endif

	u8 alignment[16]; // for alignment 32 byte

} __attribute__((packed)) RXDesc;

#if 0
static inline void rx_index_next(RXRing *ring)
{ 
        ring->cur_index = ((ring->cur_index + 1) % ring->ring_size);
}
static inline void tx_index_next(TXRing *ring)
{
        ring->cur_index = ((ring->cur_index + 1) % ring->ring_size);
}
#endif


typedef struct pkt
{
	struct pkt	*next;
	u32		flags;
	u32		length;
	u8		*pkt_buffer;
} pkt_t;

typedef struct
{
	u16	gid;
	u16	vid;
	u32	port_map;
	u32	tag_port_map;
	u8	mac[6];
} vlan_config_t;

typedef struct
{
	u32	pvid;
	u32	config;
	u32	state;
} port_config_t;

typedef struct 
{
	TXDesc *tx_ring;
	RXDesc *rx_ring;

	pkt_t			*tx_ring_pkt[TX_SIZE];
	pkt_t			*rx_ring_pkt[RX_SIZE];

	u32			cur_tx_desc_idx;
	u32			cur_rx_desc_idx;

	pkt_t			*pkt_pool;
	u8			*pkt_buffer_pool;
	pkt_t			*rx_pkt_pool;
	u8			*rx_pkt_buffer_pool;
	u32			free_pkt_count;
	u32			rx_free_pkt_count;
	pkt_t			*free_pkt_list;
	pkt_t			*rx_free_pkt_list;

	pkt_t			*tx_pkt_q_head;
	pkt_t			*tx_pkt_q_tail;
	u32			tx_pkt_q_count;

	u32			tx_pkt_count;
	u32			rx_pkt_count;

	u8			phy_addr;
	u16			phy_id;

	u8			mac[6];
	port_config_t		port_config[3];
	vlan_config_t		vlan_config[8];
	int 			which_port;
}GswDev;

typedef struct
{
	u8 vlan_index;
	u8 valid;
	u16 vid;
	u8 wan_side;
	u8 etag_pmap;
	u8 mb_pmap;
	u8 *my_mac;

}VLANTableEntry;

typedef struct
{
	u16 vid;
	u8 pmap;
	u8 *mac;
	u8 age_field;
	u8 vlan_mac;
	u8 filter;
}ARLTableEntry;

#endif

