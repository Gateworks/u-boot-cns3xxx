/*
 * Basic I2C functions
 *
 * Copyright (c) 2003 Texas Instruments
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author: Jian Zhang jzhang@ti.com, Texas Instruments
 *
 * Copyright (c) 2003 Wolfgang Denk, wd@denx.de
 * Rewritten to fit into the current U-Boot framework
 *
 */

#include <common.h>

#define IO_I2C_WRITE(addr, val) (*(volatile unsigned int *)(addr) = (val))
#define IO_I2C_READ(addr) (*(volatile unsigned int *)(addr))

#define MISC_MEM_MAP_VALUE(reg_offset)  (*(volatile ulong *)(0x76000000 + reg_offset))
#define MISC_IOCDB_CTRL       MISC_MEM_MAP_VALUE(0x020)

#define I2C_MEM_MAP_VALUE(x)        (*(volatile ulong *)(0x71000000 + x))

#define I2C_CONTROLLER_REG                    I2C_MEM_MAP_VALUE(0x20)
#define I2C_TIME_OUT_REG                      I2C_MEM_MAP_VALUE(0x24)
#define I2C_SLAVE_ADDRESS_REG                 I2C_MEM_MAP_VALUE(0x28)
#define I2C_WRITE_DATA_REG                    I2C_MEM_MAP_VALUE(0x2C)
#define I2C_READ_DATA_REG                     I2C_MEM_MAP_VALUE(0x30)
#define I2C_INTERRUPT_STATUS_REG              I2C_MEM_MAP_VALUE(0x34)
#define I2C_INTERRUPT_ENABLE_REG              I2C_MEM_MAP_VALUE(0x38)
#define I2C_TWI_OUT_DLY_REG                   I2C_MEM_MAP_VALUE(0x3C)

#define I2C_BUS_ERROR_FLAG     (0x1)
#define I2C_ACTION_DONE_FLAG   (0x2)

#define CNS3xxx_I2C_ENABLE()          (I2C_CONTROLLER_REG) |= ((unsigned int)0x1 << 31)
#define CNS3xxx_I2C_DISABLE()         (I2C_CONTROLLER_REG) &= ~((unsigned int)0x1 << 31)
#define CNS3xxx_I2C_ENABLE_INTR()     (I2C_INTERRUPT_ENABLE_REG) |= 0x03
#define CNS3xxx_I2C_DISABLE_INTR()    (I2C_INTERRUPT_ENABLE_REG) &= 0xfc

#define TWI_TIMEOUT         (10*HZ)
#define I2C_50KHZ          	 50000
#define I2C_100KHZ          100000
#define I2C_200KHZ          200000
#define I2C_300KHZ          300000
#define I2C_400KHZ          400000

#define CNS3xxx_I2C_CLK     I2C_100KHZ


static int wait_for_status (void);

void i2c_init (int speed, int slaveadd)
{
	unsigned int temp;

  /* Enable clock for I2C */
#define PMU_CLK_GATE_REG 0x77000000
#define PMU_SOFT_RST   0x77000004
#if 0
  temp = IO_I2C_READ(PMU_CLK_GATE_REG);
  temp |= (1 << 3);
  IO_I2C_WRITE(PMU_CLK_GATE_REG, temp);
  temp = IO_I2C_READ(PMU_SOFT_RST);
  temp &= ~(1 << 3);
  IO_I2C_WRITE(PMU_SOFT_RST, temp);
  temp |= (1 << 3);
  IO_I2C_WRITE(PMU_SOFT_RST, temp);
#endif

  I2C_CONTROLLER_REG = 0; /* Disabled the I2C */

	// Disable GPIOB function of i2c pins
	(*(volatile ulong *)(0x76000018)) |= ((1<<12)|(1<<13));

	// Set Drive Strength
  MISC_IOCDB_CTRL &= ~0x300;
  MISC_IOCDB_CTRL |= 0x300; //21mA...

  /* Check the Reg Dump when testing */
  I2C_TIME_OUT_REG =
      ((((((600*(1000000/8)) / (2 * CNS3xxx_I2C_CLK)) -
    1) & 0x3FF) << 8) | (1 << 7) | 0x7F);
  I2C_TWI_OUT_DLY_REG |= 0x3;

  /* Clear Interrupt Status (0x2 | 0x1) */
  I2C_INTERRUPT_STATUS_REG |= (I2C_ACTION_DONE_FLAG | I2C_BUS_ERROR_FLAG);

  /* Enable the I2C Controller */
  CNS3xxx_I2C_ENABLE();
}

static int i2c_read_byte (u8 devaddr, u8 regoffset, u8 * value)
{
	int ret;

	I2C_CONTROLLER_REG = 0x80000000;
	I2C_SLAVE_ADDRESS_REG = (devaddr << 1);

	// Write then Read operation
	I2C_CONTROLLER_REG |= (2 << 4);
	// regoffset in write operation
	I2C_WRITE_DATA_REG = regoffset;

	/* Clear Interrupt */
	I2C_INTERRUPT_STATUS_REG |= 0x3;

	I2C_CONTROLLER_REG |= (1 << 6);

	ret = wait_for_status();

	if (!ret) {
		*value = I2C_READ_DATA_REG & 0xff;
	}

	return ret;
}

static int i2c_write_byte (u8 devaddr, u8 regoffset, u8 value)
{
	int ret;

	I2C_CONTROLLER_REG = 0x80000000;
	I2C_SLAVE_ADDRESS_REG = (devaddr << 1);

	// Write operation, 2 bytes
	I2C_CONTROLLER_REG |= (1 << 4) | (1 << 2);

	// regoffset in first byte of write operation
	// value in second byte
	I2C_WRITE_DATA_REG = (regoffset | (value << 8));

	/* Clear Interrupt */
	I2C_INTERRUPT_STATUS_REG |= 0x3;

	I2C_CONTROLLER_REG |= (1 << 6);

	ret = wait_for_status();

	return ret;
}

int i2c_probe (uchar chip)
{
	int ret;

	I2C_CONTROLLER_REG = 0x80000000;
	I2C_SLAVE_ADDRESS_REG = (chip << 1);

	// Write operation
	I2C_CONTROLLER_REG |= (1 << 4);

	// chip in first byte of write operation
	I2C_WRITE_DATA_REG = 0;

	/* Clear Interrupt */
	I2C_INTERRUPT_STATUS_REG |= 0x3;

	I2C_CONTROLLER_REG |= (1 << 6);

	ret = wait_for_status();

	return ret;
}

int i2c_read (uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	int i;
	int retry;

	if (alen > 1) {
		printf ("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	while (addr >= 256) {
		chip++;
		addr -= 256;
	}

	if (addr + len > 256) {
		printf ("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		retry = 0;
		while (i2c_read_byte (chip, addr + i, &buffer[i])) {
			if (retry++ == 10) {
				printf ("I2C read: I/O error\n");
				return 1;
			}
			udelay(1000);
		}
	}

	return 0;
}

int i2c_write (uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	int i;
	int retry;

	if (alen > 1) {
		printf ("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		printf ("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		retry = 0;
		while (i2c_write_byte (chip, addr + i, buffer[i])) {
			if (retry++ == 10) {
				printf ("I2C read: I/O error\n");
				return 1;
			}
			udelay(1000);
		}
	}

	return 0;
}

static int wait_for_status(void)
{
	while ((I2C_INTERRUPT_STATUS_REG & 0x3) == 0);
	udelay(100);
	if ((I2C_INTERRUPT_STATUS_REG & 0x3) == 2) {
		return 0;
	} else {
		return 1;
	}
}
