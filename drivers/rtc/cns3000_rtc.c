/*
 * Copyright (c) 2008 Cavium Networks 
 * 
 * Scott Shu
 *
 * Real Time Clock on CNS3000 SOCs
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
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>

#if (defined(CONFIG_CMD_DATE))

#include <rtc.h>
#include <cns3000.h>

#undef RTC_TEST
#undef DEBUG

#define RTC_WRITE_REGl(reg, value)	(*(volatile unsigned long*)(reg) = (value))
#define RTC_READ_REGl(reg)		(*(volatile unsigned long*)(reg))

/*
 * RTC Control Register
 */
#define RTC_ENABLE_BIT			(1 << 0)
#define RTC_AUTO_ALARM_SEC_ENABLE_BIT	(1 << 1)
#define RTC_AUTO_ALARM_MIN_ENABLE_BIT	(1 << 2)
#define RTC_AUTO_ALARM_HRS_ENABLE_BIT	(1 << 3)
#define RTC_AUTO_ALARM_DAY_ENABLE_BIT	(1 << 4)
#define RTC_MATCH_ALARM_INTC_ENABLE_BIT	(1 << 5)
#define RTC_SYSTEM_CLK_BIT		(1 << 6)

static void cns3000_rtc_enable(void)
{
#ifdef RTC_TEST	/* select system clock as the RTC reference clock */
	RTC_WRITE_REGl(CNS3000_VEGA_RTC_BASE + RTC_CTRL_OFFSET, RTC_ENABLE_BIT | RTC_SYSTEM_CLK_BIT);
#else
	RTC_WRITE_REGl(CNS3000_VEGA_RTC_BASE + RTC_CTRL_OFFSET, RTC_ENABLE_BIT);
#endif
}

/*
 * return current time in seconds
 */
static unsigned long cns3000_rtc_time(void)
{
	unsigned long second, minute, hour, day;
	unsigned long second2;

	do {
		second = RTC_READ_REGl(CNS3000_VEGA_RTC_BASE + RTC_SEC_OFFSET);
		minute = RTC_READ_REGl(CNS3000_VEGA_RTC_BASE + RTC_MIN_OFFSET);
		hour = RTC_READ_REGl(CNS3000_VEGA_RTC_BASE + RTC_HOUR_OFFSET);
		day = RTC_READ_REGl(CNS3000_VEGA_RTC_BASE + RTC_DAY_OFFSET);
		second2 = RTC_READ_REGl(CNS3000_VEGA_RTC_BASE + RTC_SEC_OFFSET);
	} while (second != second2);

	return day * 24 * 60 * 60 + hour * 60 * 60 + minute * 60 + second;
}

/*
 * Get the current time from the RTC
 */
int rtc_get(struct rtc_time *tmp)
{
	unsigned long now;

	debug("rtc_get(): record register: %x\n", RTC_READ_REGl(CNS3000_VEGA_RTC_BASE + RTC_REC_OFFSET));

	now = cns3000_rtc_time() + RTC_READ_REGl(CNS3000_VEGA_RTC_BASE + RTC_REC_OFFSET);

	to_tm(now, tmp);

	return 0;
}

/*
 * Set the RTC
 */
int rtc_set(struct rtc_time *tmp)
{
	unsigned long new;
	unsigned long now;

	debug("rtc_set(): DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	new = mktime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	now = cns3000_rtc_time();

	debug("rtc_set(): write %x to record register\n", new - now);

	RTC_WRITE_REGl((CNS3000_VEGA_RTC_BASE + RTC_REC_OFFSET), new - now);

	return 0;
}

void rtc_reset(void)
{
	debug("rtc_reset()\n");
	cns3000_rtc_enable();
}

#endif /* CONFIG_CMD_DATE */
