/*
 *	LiMon - BOOTP/TFTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 */

#ifndef __TFTP_H__
#define __TFTP_H__

#ifdef CONFIG_UDP_FRAGMENT
#include <net.h>

/*
 * Maximum TFTP block size bound to max size of fragmented IP/UDP
 * packets minus TFTP and UDP/IP overhead. TFTP overhead is 2 byte
 * opcode and 2 byte block-number.
 */
#define TFTP_BLOCK_SIZE_MAX     (NET_FRAG_BUF_SIZE - sizeof(IP_t) - 4)

#endif /* CONFIG_UDP_FRAGMENT */

/**********************************************************************/
/*
 *	Global functions and variables.
 */

/* tftp.c */
extern void	TftpStart (void);	/* Begin TFTP get */

/**********************************************************************/

#endif /* __TFTP_H__ */
