/*
 * Copyright (c) 2008 Cavium Networks 
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo (void)
{
	unsigned long id_code;
	unsigned long cpu_id;

	asm ("mrc p15, 0, %0, c0, c0, 0":"=r" (id_code));
	asm ("mrc p15, 0, %0, c0, c0, 5":"=r" (cpu_id));

	printf("CPU: Cavium Networks CNS3000\n");
	printf("ID Code: %lx ", id_code);

	switch ((id_code & 0x0000fff0) >> 4) {
	case 0xb02:
		printf("(Part number: 0xB02, ");
		break;

	default:
		printf("(Part number: unknown, ");

	}
	printf("Revision number: %lx) \n", (id_code & 0x0000000f));

	printf("CPU ID: %lx \n", cpu_id);

	return 0;
}
#endif

u32 __weak get_board_rev(void)
{
	unsigned long id_code;

	asm ("mrc p15, 0, %0, c0, c0, 0":"=r" (id_code));

	return (id_code & 0xf);
}
