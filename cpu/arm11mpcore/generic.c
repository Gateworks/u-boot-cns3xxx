/*
 * Copyright (c) 2008 Cavium Networks 
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
	printf("Revision number: %x) \n", (id_code & 0x0000000f));

	printf("CPU ID: %lx \n", cpu_id);

	return 0;
}
#endif
