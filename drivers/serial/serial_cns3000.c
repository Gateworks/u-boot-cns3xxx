/*******************************************************************************
 *
 * Copyright (c) 2008 Cavium Networks 
 * Scott Shu, <scott.shu@caviumnetworks.com>
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
 ******************************************************************************/

#include <common.h>

#include "serial_cns3000.h"

#define IO_WRITE(addr, val) (*(volatile unsigned int *)(addr) = (val))
#define IO_READ(addr) (*(volatile unsigned int *)(addr))

static volatile unsigned char *const port[] = CONFIG_CNS3000_PORTS;
#define NUM_PORTS (sizeof(port)/sizeof(port[0]))

#ifdef CONFIG_CNS3000_SERIAL
int serial_init (void)
{
	unsigned int temp;
	unsigned int divisor;

	/* Hardware Initialization. */
#if 1
	/* Enable clock for UART 0,1,2 */
#define PMU_CLK_GATE_REG 0x77000000
#define PMU_SOFT_RST	 0x77000004
	temp = IO_READ(PMU_CLK_GATE_REG);
	temp |= (1 << 6) | (1 << 7) | (1 << 8);
	IO_WRITE(PMU_CLK_GATE_REG, temp);
	temp = IO_READ(PMU_SOFT_RST);
	temp &= ~(1 << 6) | (1 << 7) | (1 << 8);
	IO_WRITE(PMU_SOFT_RST, temp);
	temp |= (1 << 6) | (1 << 7) | (1 << 8);
	IO_WRITE(PMU_SOFT_RST, temp);

#define GPIOA_PIN_EN 0x76000014
#define GPIOB_PIN_EN 0x76000018
	temp = IO_READ(GPIOA_PIN_EN);
	temp |= (1 << 29) | (1 << 30) | (1 << 31);
	IO_WRITE(GPIOA_PIN_EN, temp);

	temp = IO_READ(GPIOB_PIN_EN);
	temp |= (1 << 0) | (1 << 1) | (1 << 2);
	IO_WRITE(GPIOB_PIN_EN, temp);
#endif

        /* Enter DLAB mode */
	IO_WRITE (port[0] + UART_CNS3000_LCR,
		IO_READ (port[0] + UART_CNS3000_LCR) | UART_CNS3000_LCR_DLAB_ENABLE);
	IO_WRITE (port[1] + UART_CNS3000_LCR,
		IO_READ (port[1] + UART_CNS3000_LCR) | UART_CNS3000_LCR_DLAB_ENABLE);
	IO_WRITE (port[2] + UART_CNS3000_LCR,
		IO_READ (port[2] + UART_CNS3000_LCR) | UART_CNS3000_LCR_DLAB_ENABLE);


	/* Set baud rate */
	IO_WRITE (port[0] + UART_CNS3000_PSR, (0x01 & 0x03));
	IO_WRITE (port[1] + UART_CNS3000_PSR, (0x01 & 0x03));
	IO_WRITE (port[2] + UART_CNS3000_PSR, (0x01 & 0x03));

	temp = 16 * CONFIG_BAUDRATE;
	divisor = CONFIG_CNS3000_CLOCK / temp;

	IO_WRITE (port[0] + UART_CNS3000_DLL, (divisor & 0xff));
	IO_WRITE (port[0] + UART_CNS3000_DLM, ((divisor & 0xff00) >> 8));
	IO_WRITE (port[1] + UART_CNS3000_DLL, (divisor & 0xff));
	IO_WRITE (port[1] + UART_CNS3000_DLM, ((divisor & 0xff00) >> 8));
	IO_WRITE (port[2] + UART_CNS3000_DLL, (divisor & 0xff));
	IO_WRITE (port[2] + UART_CNS3000_DLM, ((divisor & 0xff00) >> 8));

	/* Exit DLAB mode */
	IO_WRITE (port[0] + UART_CNS3000_LCR,
		IO_READ (port[0] + UART_CNS3000_LCR) & ~(UART_CNS3000_LCR_DLAB_ENABLE));
	IO_WRITE (port[1] + UART_CNS3000_LCR,
		IO_READ (port[1] + UART_CNS3000_LCR) & ~(UART_CNS3000_LCR_DLAB_ENABLE));
	IO_WRITE (port[2] + UART_CNS3000_LCR,
		IO_READ (port[2] + UART_CNS3000_LCR) & ~(UART_CNS3000_LCR_DLAB_ENABLE));

	/* Set the UART to be 8 bits, 1 stop bit, no parity. */
	IO_WRITE (port[0] + UART_CNS3000_LCR,
		(UART_CNS3000_LCR_WLEN_8 | UART_CNS3000_LCR_STOP_BIT_1 | UART_CNS3000_LCR_PARITY_NONE));
	IO_WRITE (port[1] + UART_CNS3000_LCR,
		(UART_CNS3000_LCR_WLEN_8 | UART_CNS3000_LCR_STOP_BIT_1 | UART_CNS3000_LCR_PARITY_NONE));
	IO_WRITE (port[2] + UART_CNS3000_LCR,
		(UART_CNS3000_LCR_WLEN_8 | UART_CNS3000_LCR_STOP_BIT_1 | UART_CNS3000_LCR_PARITY_NONE));

	/* Set the UART to fifo enabled. */
	IO_WRITE (port[0] + UART_CNS3000_FCR,
		(UART_CNS3000_FCR_FIFO_ENABLE | UART_CNS3000_FCR_RXFIFO_TRIGGER_LEVEL_8 |
		UART_CNS3000_FCR_RXFIFO_RESET | UART_CNS3000_FCR_TXFIFO_RESET));
	IO_WRITE (port[1] + UART_CNS3000_FCR,
		(UART_CNS3000_FCR_FIFO_ENABLE | UART_CNS3000_FCR_RXFIFO_TRIGGER_LEVEL_8 |
		UART_CNS3000_FCR_RXFIFO_RESET | UART_CNS3000_FCR_TXFIFO_RESET));
	IO_WRITE (port[2] + UART_CNS3000_FCR,
		(UART_CNS3000_FCR_FIFO_ENABLE | UART_CNS3000_FCR_RXFIFO_TRIGGER_LEVEL_8 |
		UART_CNS3000_FCR_RXFIFO_RESET | UART_CNS3000_FCR_TXFIFO_RESET));

	/* Finally, enable the UART */

	return 0;
}
#endif /* CONFIG_CNS3000_SERIAL */

int serial_getc (void)
{
	unsigned int data;

	/* Wait until there is data in the FIFO */
	while ((IO_READ (port[CONFIG_CONS_INDEX] + UART_CNS3000_LSR) & UART_CNS3000_LSR_DATA_READY) != UART_CNS3000_LSR_DATA_READY)
		; /* wait */

	data = IO_READ (port[CONFIG_CONS_INDEX] + UART_CNS3000_RBR);

	return (int) data;
}

static void cns3000_putc (char c)
{
	/* Wait until there is space in the FIFO */
	while ((IO_READ (port[CONFIG_CONS_INDEX] + UART_CNS3000_LSR) & UART_CNS3000_LSR_THR_EMPTY) != UART_CNS3000_LSR_THR_EMPTY)
		; /* wait */

	/* Send the character */
	IO_WRITE (port[CONFIG_CONS_INDEX] + UART_CNS3000_THR, c);
}

void serial_putc (const char c)
{
	if (c == '\n')
		cns3000_putc ('\r');

	cns3000_putc (c);
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int serial_tstc (void)
{
	return ((IO_READ (port[CONFIG_CONS_INDEX] + UART_CNS3000_LSR) & UART_CNS3000_LSR_DATA_READY) == UART_CNS3000_LSR_DATA_READY);
}

void serial_setbrg (void)
{
}

