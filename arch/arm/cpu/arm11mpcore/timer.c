/*
 * Timer implementation
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <cns3000.h>
#include <asm/io.h>

#define PMU_REG_VALUE(addr) (*((volatile unsigned int *)(CNS3000_VEGA_PMU_BASE+addr)))
#define CLK_GATE_REG PMU_REG_VALUE(0)
#define SOFT_RST_REG PMU_REG_VALUE(4)

#define TIMER_LOAD_VAL	0xffffffff

#define READ_TIMER1 (*(volatile ulong *)(CNS3000_VEGA_TIMER_BASE))
#define READ_TIMER2 (*(volatile ulong *)(CNS3000_VEGA_TIMER_BASE + 0x10))

/* initialize timer */
int timer_init(void)
{
	/* Setup timer for 1khz */
	CLK_GATE_REG |= (1 << 14);
	SOFT_RST_REG &= (~(1 << 14));
	SOFT_RST_REG |= (1 << 14);
	*(volatile ulong *)(CNS3000_VEGA_TIMER_BASE + 0x00) = TIMER_LOAD_VAL;
	*(volatile ulong *)(CNS3000_VEGA_TIMER_BASE + 0x04) = TIMER_LOAD_VAL;
	/* Enabled, down counter, no interrupt, 32-bit */
	*(volatile ulong *)(CNS3000_VEGA_TIMER_BASE + 0x30) |= 0x0203;

	return 0;
}

/* get relative tick count */
ulong get_timer(ulong base)
{
	return TIMER_LOAD_VAL - READ_TIMER1 - base;
}

/* get current tick count */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/* get number of timer ticks per second */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
