/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * CPU specific code for a core tile with an unknown cpu
 * - hence fairly empty......
 */

#include <common.h>
#include <command.h>

int cpu_init (void)
{
	/*
	 * setup up stacks if necessary
	 */
#ifdef CONFIG_USE_IRQ
	DECLARE_GLOBAL_DATA_PTR;

	IRQ_STACK_START = _armboot_start - CFG_MALLOC_LEN - CFG_GBL_DATA_SIZE - 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;
#endif
	return 0;
}

static void cache_flush(void);

int cleanup_before_linux (void)
{
  /*
   * this function is called just before we call linux
   * it prepares the processor for linux
   *
   * we turn off caches etc ...
   */

  disable_interrupts ();

  /* turn off I/D-cache */
  icache_disable();
  dcache_disable();
  l2_cache_disable();

  /* flush I/D-cache */
  cache_flush();

  return 0;
}

/* flush I/D-cache */
static void cache_flush (void)
{
	unsigned long i = 0;
	/* clean entire data cache */
	asm volatile("mcr p15, 0, %0, c7, c10, 0" : : "r" (i));
	/* invalidate both caches and flush btb */
	asm volatile("mcr p15, 0, %0, c7, c7, 0" : : "r" (i));
	/* mem barrier to sync things */
	asm volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (i));
}
