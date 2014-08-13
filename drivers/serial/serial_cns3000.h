/*******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#define	UART_CNS3000_RBR	0x00
#define	UART_CNS3000_THR	0x00
#define	UART_CNS3000_DLL	0x00
#define	UART_CNS3000_IER	0x04
#define	UART_CNS3000_DLM	0x04
#define	UART_CNS3000_IIR	0x08
#define	UART_CNS3000_FCR	0x08
#define	UART_CNS3000_PSR	0x08
#define	UART_CNS3000_LCR	0x0C
#define	UART_CNS3000_UCR	0x10
#define	UART_CNS3000_LSR	0x14
#define	UART_CNS3000_SPR	0x1C

/* FCR Register	*/
#define	UART_CNS3000_FCR_FIFO_ENABLE			(1 << 0)
#define	UART_CNS3000_FCR_RXFIFO_RESET			(1 << 1)
#define	UART_CNS3000_FCR_TXFIFO_RESET			(1 << 2)
#define	UART_CNS3000_FCR_RXFIFO_TRIGGER_LEVEL_1		(0 << 6)
#define	UART_CNS3000_FCR_RXFIFO_TRIGGER_LEVEL_4		(1 << 6)
#define	UART_CNS3000_FCR_RXFIFO_TRIGGER_LEVEL_8		(2 << 6)
#define	UART_CNS3000_FCR_RXFIFO_TRIGGER_LEVEL_14	(3 << 6)

/* LCR Register	*/
#define	UART_CNS3000_LCR_WLEN_5				(0 << 0)
#define	UART_CNS3000_LCR_WLEN_6				(1 << 0)
#define	UART_CNS3000_LCR_WLEN_7				(2 << 0)
#define	UART_CNS3000_LCR_WLEN_8				(3 << 0)
#define	UART_CNS3000_LCR_STOP_BIT_1			(0 << 2)
#define	UART_CNS3000_LCR_STOP_BIT_2			(1 << 2)
#define	UART_CNS3000_LCR_PARITY_NONE			(0 << 3)
#define	UART_CNS3000_LCR_PARITY_ODD			(1 << 3)
#define	UART_CNS3000_LCR_PARITY_EVEN			(3 << 3)
#define	UART_CNS3000_LCR_PARITY_STICK_1			(5 << 3)
#define	UART_CNS3000_LCR_PARITY_STICK_0			(7 << 3)
#define	UART_CNS3000_LCR_SET_BREAK			(1 << 6)
#define	UART_CNS3000_LCR_DLAB_ENABLE			(1 << 7)

/* LSR Register */
#define UART_CNS3000_LSR_DATA_READY			(1 << 0)
#define UART_CNS3000_LSR_OVERRUN_ERROR			(1 << 1)
#define UART_CNS3000_LSR_PARITY_ERROR			(1 << 2)
#define UART_CNS3000_LSR_FRAMING_ERROR			(1 << 3)
#define UART_CNS3000_LSR_BREAK_INTERRUPT		(1 << 4)
#define UART_CNS3000_LSR_THR_EMPTY			(1 << 5)
#define UART_CNS3000_LSR_TRANSMITTER_EMPTY		(1 << 6)
#define UART_CNS3000_LSR_FIFO_DATA_ERROR		(1 << 7)

