#ifndef _STAR_SYS_SYSDEF_H_
#define _STAR_SYS_SYSDEF_H_

/******************************************************************************
 * MODULE NAME:    star_sys_sysdef.h
 * PROJECT CODE:   Vega
 * DESCRIPTION:    
 * MAINTAINER:     Jacky
 * DATE:           09 March 2009
 *
 * SOURCE CONTROL: 
 *
 * LICENSE:
 *     This source code is copyright (c) 2008 Cavium Networks Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 *
 * SOURCE:
 * ISSUES:
 * NOTES TO USERS:
 ******************************************************************************/
//#include <stdio.h>


/*
 * define data type macros
 */
typedef int                        s_int32;
typedef short                      s_int16;
typedef char                       s_int8; 
typedef unsigned int               u_int32;
typedef unsigned short             u_int16;
typedef unsigned char              u_int8;
typedef unsigned int               bool;


#ifndef TRUE
#define TRUE                       (bool)1
#endif

#ifndef FALSE
#define FALSE                      (bool)0
#endif


#ifndef NULL
#define NULL                       ((void *)0)
#endif


/*
 *  For Byte reverse within 32-bit data: 0x11223344-->0x44332211
 */
#define REV(x) __rev(x)


/*
 *  For Byte reverse within 16-bit data : 0x1122-->0x2211
 */
#define REV16(x)  ((((u_int16)x&0xff)<<8)| (((u_int16)x&0xff00)>>8) )

/*
 * define 8-bit IO access macros
 */ 
#define IO_OUT_BYTE(reg, data)     ((*((volatile u_int8 *)(reg))) = (u_int8)(data))
#define IO_IN_BYTE(reg)            (*((volatile u_int8 *)(reg)))



#ifdef __BIG_ENDIAN


#define IO_OUT_WORD(reg, data)     ((*((volatile u_int32 *)(reg))) = REV((u_int32)(data)))
#define IO_IN_WORD(reg)            REV((*((volatile u_int32 *)(reg))))


#define IO_OUT_HWORD(reg, data)    ((*((volatile u_int16 *)(reg))) = REV16((u_int16)(data)))
#define IO_IN_HWORD(reg)           REV16((*((volatile u_int16 *)(reg))))

#else

#define IO_OUT_WORD(reg, data)     ((*((volatile u_int32 *)(reg))) = (u_int32)(data))
#define IO_IN_WORD(reg)            (*((volatile u_int32 *)(reg)))

#define IO_OUT_HWORD(reg, data)    ((*((volatile u_int16 *)(reg))) = (u_int16)(data))
#define IO_IN_HWORD(reg)           (*((volatile u_int16 *)(reg)))

#endif




/*
 * define build type macros and endian mode macros
 */
#define RAM_BUILD                  (0)

#define FLASH_BUILD                (1)

#define ROM_BUILD                  (2)


#define LITTLE_ENDIAN              (0)

#define BIG_ENDIAN                 (1)

#ifndef NULL
#define NULL                       ((void *)0)
#endif


/*
 * define system-related constant macros
 */


/*
 * define function macros
 */
#if (((BUILD_TYPE == FLASH_BUILD) || (BUILD_TYPE == ROM_BUILD)) && !defined(USE_UART_PORT_DEBUG_SUPPORT))
#define printf(...)                NULL
#endif


#endif // end of #ifndef _STAR_SYS_SYSDEF_H_
