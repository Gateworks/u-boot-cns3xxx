/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

//#define DEBUG
#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <asm/hardware.h>

/* define access macros */
#define SPI_MEM_MAP_VALUE(reg_offset)       (*((u32 volatile *)(0x71000000 + reg_offset)))

#define SPI_CONFIGURATION_REG           SPI_MEM_MAP_VALUE(0x40)
#define SPI_SERVICE_STATUS_REG          SPI_MEM_MAP_VALUE(0x44)
#define SPI_BIT_RATE_CONTROL_REG        SPI_MEM_MAP_VALUE(0x48)
#define SPI_TRANSMIT_CONTROL_REG        SPI_MEM_MAP_VALUE(0x4C)
#define SPI_TRANSMIT_BUFFER_REG         SPI_MEM_MAP_VALUE(0x50)
#define SPI_RECEIVE_CONTROL_REG         SPI_MEM_MAP_VALUE(0x54)
#define SPI_RECEIVE_BUFFER_REG          SPI_MEM_MAP_VALUE(0x58)
#define SPI_FIFO_TRANSMIT_CONFIG_REG        SPI_MEM_MAP_VALUE(0x5C)
#define SPI_FIFO_TRANSMIT_CONTROL_REG       SPI_MEM_MAP_VALUE(0x60)
#define SPI_FIFO_RECEIVE_CONFIG_REG     SPI_MEM_MAP_VALUE(0x64)
#define SPI_INTERRUPT_STATUS_REG        SPI_MEM_MAP_VALUE(0x68)
#define SPI_INTERRUPT_ENABLE_REG        SPI_MEM_MAP_VALUE(0x6C)

#define SPI_TRANSMIT_BUFFER_REG_ADDR        (CNS3XXX_SSP_BASE +0x50)
#define SPI_RECEIVE_BUFFER_REG_ADDR     (CNS3XXX_SSP_BASE +0x58)

#define SPI0_BUS 0
#define SPI0_NUM_CS 1


//Spi_Flash_Initialize(u8 spi_flash_channel)
struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	struct spi_slave *slave;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	slave = spi_alloc_slave_base(bus, cs);
	if (!slave)
		return NULL;

	return slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	free(slave);
}

void spi_init()
{
	// Do Nothing
}

int spi_claim_bus(struct spi_slave *slave)
{
	u32 volatile receive_data;

	/* Enable SPI pins */
	HAL_MISC_ENABLE_SPI_PINS();

	/* Disable SPI serial flash access through 0x30000000 region */
	HAL_MISC_DISABLE_SPI_SERIAL_FLASH_BANK_ACCESS();

	/*
	 * Note SPI is NOT enabled after this function is invoked!!
	 */
	SPI_CONFIGURATION_REG =
		(((0x0 & 0x3) << 0) | /* 8bits shift length */
		 (0x0 << 9) | /* general SPI mode */
		 (0x0 << 10) | /* disable FIFO */
		 (0x1 << 11) | /* SPI master mode */
		 (0x0 << 12) | /* disable SPI loopback mode */
		 (0x1 << 13) |
		 (0x1 << 14) |
		 (0x0 << 24) | /* Disable SPI Data Swap */
		 (0x0 << 30) | /* Disable SPI High Speed Read for BootUp */
		 (0x0 << 31)); /* Disable SPI */

	SPI_BIT_RATE_CONTROL_REG = 0x1 & 0x07; // PCLK/8

	/* Configure SPI's Tx channel */
	SPI_TRANSMIT_CONTROL_REG &= ~(0x03);
	SPI_TRANSMIT_CONTROL_REG |= slave->cs & 0x03;

	/* Configure Tx FIFO Threshold */
	SPI_FIFO_TRANSMIT_CONFIG_REG &= ~(0x03 << 4);
	SPI_FIFO_TRANSMIT_CONFIG_REG |= ((0x0 & 0x03) << 4);

	// Configure Rx FIFO Threshold
	SPI_FIFO_RECEIVE_CONFIG_REG &= ~(0x03 << 4);
	SPI_FIFO_RECEIVE_CONFIG_REG |= ((0x0 & 0x03) << 4);

	SPI_INTERRUPT_ENABLE_REG = 0;

	/* Clear spurious interrupt sources */
	SPI_INTERRUPT_STATUS_REG = (0xF << 4);

	receive_data = SPI_RECEIVE_BUFFER_REG;

	/* Enable SPI */
	SPI_CONFIGURATION_REG |= (0x1 << 31);

	// quiet compiler warning
	receive_data = 0;
	return receive_data;
}

void spi_release_bus(struct spi_slave *slave)
{
	// Do Nothing
}

static inline u32
cns3xxx_spi_bus_idle(void)
{
	/*
	 * Return value :
	 *    1 : Bus Idle
	 *    0 : Bus Busy
	 */
	return ((SPI_SERVICE_STATUS_REG & 0x1) ? 0 : 1);
}

static inline u32
cns3xxx_spi_tx_buf_empty(void)
{
	/*
	 * Return value :
	 *    1 : SPI Tx Buffer Empty
	 *    0 : SPI Tx Buffer Not Empty
	 */
	return ((SPI_INTERRUPT_STATUS_REG & (0x1 << 3)) ? 1 : 0);
}

static inline u32
cns3xxx_spi_rx_buf_full(void)
{
	/*
	 * Return value :
	 *    1 : SPI Rx Buffer Full
	 *    0 : SPI Rx Buffer Not Full
	 */
	return ((SPI_INTERRUPT_STATUS_REG & (0x1 << 2)) ? 1 : 0);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Buffer_Transmit_Receive
 * PURPOSE:
 *
 ******************************************************************************/
static u32
cns3xxx_spi_buf_txrx(u32 tx_channel, u32 tx_eof_flag, unsigned int len, u8 * tx_data, u8 * rx_data)
{
	u32 volatile temp;

	/*
	 * 1. Wait until SPI Bus is idle, and Tx Buffer is empty
	 * 2. Configure Tx channel and Back-to-Back transmit EOF setting
	 * 3. Write Tx Data
	 * 4. Wait until Rx Buffer is full
	 * 5. Get Rx channel and Back-to-Back receive EOF setting
	 * 6. Get Rx Data
	 */

	while (len--) {
		while (!cns3xxx_spi_bus_idle()) ;

		while (!cns3xxx_spi_tx_buf_empty()) ;

		SPI_TRANSMIT_CONTROL_REG &= ~(0x7);
		if (len == 0) {
			SPI_TRANSMIT_CONTROL_REG |= (tx_channel & 0x3) | ((tx_eof_flag & 0x1) << 2);
		} else {
			SPI_TRANSMIT_CONTROL_REG |= (tx_channel & 0x3);
		}

		if (tx_data) {
			SPI_TRANSMIT_BUFFER_REG = *tx_data++;
		} else {
			SPI_TRANSMIT_BUFFER_REG = 0;
		}
		while (!cns3xxx_spi_rx_buf_full()) ;

		temp = (SPI_RECEIVE_CONTROL_REG & 0x3);

		temp = (SPI_RECEIVE_CONTROL_REG & (0x1 << 2)) ? 1 : 0;

		if (rx_data) {
			*rx_data++ = SPI_RECEIVE_BUFFER_REG;
		} else {
			temp = SPI_RECEIVE_BUFFER_REG;
		}
	}
	// quiet compiler warning
	temp = 0;
	return temp;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
       const void *dout, void *din, unsigned long flags)
{
	int num_blks = DIV_ROUND_UP(bitlen, 8);
	int tx_eof_flag = 0;

	if (flags & SPI_XFER_END)
		tx_eof_flag = 1;

	if (bitlen == 0)
		goto out;

	cns3xxx_spi_buf_txrx(slave->cs, tx_eof_flag, num_blks, (u8 *)dout, (u8 *)din);

	return 0;
out:
	if (flags & SPI_XFER_END) {
		u8 dummy = 0;
		cns3xxx_spi_buf_txrx(slave->cs, 1, 1, (u8 *) &dummy, (u8 *)0);
	}
	return 0;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{

	if (bus > SPI0_BUS)
		return 0;

	if (cs >= SPI0_NUM_CS)
		return 0;

	return 1;
}
