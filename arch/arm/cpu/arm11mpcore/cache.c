/*
 * (C) Copyright 2011
 * Ilya Yanok, EmCraft Systems
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <linux/types.h>
#include <common.h>

#ifndef CONFIG_SYS_DCACHE_OFF

#ifndef CONFIG_SYS_CACHELINE_SIZE
#define CONFIG_SYS_CACHELINE_SIZE	32
#endif

void invalidate_dcache_all(void)
{
	/* Invalidate all of dcache */
	asm volatile("mcr p15, 0, %0, c7, c6, 0\n" : : "r"(0));
	/* Memory barrier to ensure all transactions complete */
	asm volatile("mcr p15, 0, %0, c7, c10, 4\n" : : "r"(0));
}

void flush_dcache_all(void)
{
	/* Flush all of dcache */
	asm volatile("mcr p15, 0, %0, c7, c14, 0\n" : : "r"(0));
	/* Memory barrier to ensure all transactions complete */
	asm volatile("mcr p15, 0, %0, c7, c10, 4\n" : : "r"(0));
}

static int check_cache_range(unsigned long start, unsigned long stop)
{
	int ok = 1;

	if (start & (CONFIG_SYS_CACHELINE_SIZE - 1))
		ok = 0;

	if (stop & (CONFIG_SYS_CACHELINE_SIZE - 1))
		ok = 0;

	if (!ok)
		debug("CACHE: Misaligned operation at range [%08lx, %08lx]\n",
			start, stop);

	return ok;
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	if (start & (CONFIG_SYS_CACHELINE_SIZE - 1))
		start = (start & ~(CONFIG_SYS_CACHELINE_SIZE - 1));
	if (stop & (CONFIG_SYS_CACHELINE_SIZE - 1))
		stop = (stop + CONFIG_SYS_CACHELINE_SIZE - 1) & ~(CONFIG_SYS_CACHELINE_SIZE - 1);

	if (!check_cache_range(start, stop))
		return;

	while (start < stop) {
		asm volatile("mcr p15, 0, %0, c7, c6, 1\n" : : "r"(start));
		start += CONFIG_SYS_CACHELINE_SIZE;
	}
	/* Memory barrier to ensure all transactions complete */
	asm volatile("mcr p15, 0, %0, c7, c10, 4\n" : : "r"(0));
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	if (start & (CONFIG_SYS_CACHELINE_SIZE - 1))
		start = (start & ~(CONFIG_SYS_CACHELINE_SIZE - 1));
	if (stop & (CONFIG_SYS_CACHELINE_SIZE - 1))
		stop = (stop + CONFIG_SYS_CACHELINE_SIZE - 1) & ~(CONFIG_SYS_CACHELINE_SIZE - 1);

	if (!check_cache_range(start, stop))
		return;

	while (start < stop) {
		asm volatile("mcr p15, 0, %0, c7, c14, 1\n" : : "r"(start));
		start += CONFIG_SYS_CACHELINE_SIZE;
	}

	asm volatile("mcr p15, 0, %0, c7, c10, 4\n" : : "r"(0));
}

void flush_cache(unsigned long start, unsigned long size)
{
	flush_dcache_range(start, start + size);
}
#else /* #ifndef CONFIG_SYS_DCACHE_OFF */
void invalidate_dcache_all(void)
{
}

void flush_dcache_all(void)
{
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
}

void flush_cache(unsigned long start, unsigned long size)
{
}
#endif /* #ifndef CONFIG_SYS_DCACHE_OFF */

/*
 * Stub implementations for l2 cache operations
 */
void __l2_cache_disable(void) {}

void l2_cache_disable(void)
	__attribute__((weak, alias("__l2_cache_disable")));

#if !defined(CONFIG_SYS_ICACHE_OFF) || !defined(CONFIG_SYS_DCACHE_OFF)
void enable_caches(void)
{
#ifndef CONFIG_SYS_ICACHE_OFF
	icache_enable();
#endif
#ifndef CONFIG_SYS_DCACHE_OFF
	dcache_enable();
#endif
}
#endif
