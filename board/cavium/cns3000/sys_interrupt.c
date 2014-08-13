/******************************************************************************
 * MODULE NAME:    star_sys_interrupt.c
 * PROJECT CODE:   Vega
 * DESCRIPTION:    
 * MAINTAINER:     Jacky Hou
 * DATE:           12 Nov 2008
 *
 * SOURCE CONTROL: 
 *
 * LICENSE:
 *     This source code is copyright (c) 2008 Cavium Networks.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 
 *
 *
 * SOURCE:
 * ISSUES:
 * NOTES TO USERS:
 ******************************************************************************/

#include "sys_sysdef.h"
#include "cpu.h"
#define SYS_MPCORE_PRIVATE_MEM_BASE_ADDR       0x90000000

void Sys_Interrupt_Distributor_Init(void)
{
    u_int32 tmp;   

    /* Disable the whole controller */
//    GIC_CONTROL_REG &=~(0x1); //no interrrupt will sent to CPU interface
    tmp = IO_IN_WORD(GIC_CONTROL_REG_ADDR) & ~(0x1);
    IO_OUT_WORD(GIC_CONTROL_REG_ADDR, tmp );

    /* Disable all interrupts */
//    GIC_CLEAR_ENABLE_ID0_ID31_REG        =0xFFFFFFFF;     
//    GIC_CLEAR_ENABLE_ID32_ID63_REG       =0xFFFFFFFF;
//    GIC_CLEAR_ENABLE_ID64_ID95_REG       =0xFFFFFFFF;
//    GIC_CLEAR_ENABLE_ID96_ID127_REG      =0xFFFFFFFF;
//    GIC_CLEAR_ENABLE_ID128_ID159_REG     =0xFFFFFFFF;
//    GIC_CLEAR_ENABLE_ID160_ID191_REG     =0xFFFFFFFF;
//    GIC_CLEAR_ENABLE_ID192_ID223_REG     =0xFFFFFFFF;
//    GIC_CLEAR_ENABLE_ID224_ID255_REG     =0xFFFFFFFF;

    IO_OUT_WORD(GIC_CLEAR_ENABLE_ID0_ID31_REG_ADDR        ,0xFFFFFFFF);     
    IO_OUT_WORD(GIC_CLEAR_ENABLE_ID32_ID63_REG_ADDR       ,0xFFFFFFFF);
    IO_OUT_WORD(GIC_CLEAR_ENABLE_ID64_ID95_REG_ADDR       ,0xFFFFFFFF);
    IO_OUT_WORD(GIC_CLEAR_ENABLE_ID96_ID127_REG_ADDR      ,0xFFFFFFFF);
    IO_OUT_WORD(GIC_CLEAR_ENABLE_ID128_ID159_REG_ADDR     ,0xFFFFFFFF);
    IO_OUT_WORD(GIC_CLEAR_ENABLE_ID160_ID191_REG_ADDR     ,0xFFFFFFFF);
    IO_OUT_WORD(GIC_CLEAR_ENABLE_ID192_ID223_REG_ADDR     ,0xFFFFFFFF);
    IO_OUT_WORD(GIC_CLEAR_ENABLE_ID224_ID255_REG_ADDR     ,0xFFFFFFFF);


    /* Clear all interrupts, interrupts return from pending to "inactive" state */
//    GIC_CLEAR_PENDING_ID0_ID31_REG       =0xFFFFFFFF;       
//    GIC_CLEAR_PENDING_ID32_ID63_REG      =0xFFFFFFFF;       
//    GIC_CLEAR_PENDING_ID64_ID95_REG      =0xFFFFFFFF;       
//    GIC_CLEAR_PENDING_ID96_ID127_REG     =0xFFFFFFFF;       
//    GIC_CLEAR_PENDING_ID128_ID159_REG    =0xFFFFFFFF;       
//    GIC_CLEAR_PENDING_ID160_ID191_REG    =0xFFFFFFFF;       
//    GIC_CLEAR_PENDING_ID192_ID223_REG    =0xFFFFFFFF;       
//    GIC_CLEAR_PENDING_ID224_ID255_REG    =0xFFFFFFFF;       
    IO_OUT_WORD(GIC_CLEAR_PENDING_ID0_ID31_REG_ADDR       ,0xFFFFFFFF);       
    IO_OUT_WORD(GIC_CLEAR_PENDING_ID32_ID63_REG_ADDR      ,0xFFFFFFFF);       
    IO_OUT_WORD(GIC_CLEAR_PENDING_ID64_ID95_REG_ADDR      ,0xFFFFFFFF);       
    IO_OUT_WORD(GIC_CLEAR_PENDING_ID96_ID127_REG_ADDR     ,0xFFFFFFFF);       
    IO_OUT_WORD(GIC_CLEAR_PENDING_ID128_ID159_REG_ADDR    ,0xFFFFFFFF);       
    IO_OUT_WORD(GIC_CLEAR_PENDING_ID160_ID191_REG_ADDR    ,0xFFFFFFFF);       
    IO_OUT_WORD(GIC_CLEAR_PENDING_ID192_ID223_REG_ADDR    ,0xFFFFFFFF);       
    IO_OUT_WORD(GIC_CLEAR_PENDING_ID224_ID255_REG_ADDR    ,0xFFFFFFFF);       
    /* Reset interrupt priorities, 0x0 is the highest priority */
    GIC_PRIORITY_ID0_ID3_REG             =0x0;      
    GIC_PRIORITY_ID4_ID7_REG             =0x0;      
    GIC_PRIORITY_ID8_ID11_REG            =0x0;      
    GIC_PRIORITY_ID12_ID15_REG           =0x0;      
    GIC_PRIORITY_ID16_ID19_REG           =0x0;      
    GIC_PRIORITY_ID20_ID23_REG           =0x0;      
    GIC_PRIORITY_ID24_ID27_REG           =0x0;      
    GIC_PRIORITY_ID28_ID31_REG           =0x0;      
    GIC_PRIORITY_ID32_ID35_REG           =0x0;      
    GIC_PRIORITY_ID36_ID39_REG           =0x0;      
    GIC_PRIORITY_ID40_ID43_REG           =0x0;      
    GIC_PRIORITY_ID44_ID47_REG           =0x0;      
    GIC_PRIORITY_ID48_ID51_REG           =0x0;      
    GIC_PRIORITY_ID52_ID55_REG           =0x0;      
    GIC_PRIORITY_ID56_ID59_REG           =0x0;      
    GIC_PRIORITY_ID60_ID63_REG           =0x0;      
    GIC_PRIORITY_ID64_ID67_REG           =0x0;      
    GIC_PRIORITY_ID68_ID71_REG           =0x0;      
    GIC_PRIORITY_ID72_ID75_REG           =0x0;      
    GIC_PRIORITY_ID76_ID79_REG           =0x0;      
    GIC_PRIORITY_ID80_ID83_REG           =0x0;      
    GIC_PRIORITY_ID84_ID87_REG           =0x0;      
    GIC_PRIORITY_ID88_ID91_REG           =0x0;      
    GIC_PRIORITY_ID92_ID95_REG           =0x0;      
    GIC_PRIORITY_ID96_ID99_REG           =0x0;      
    GIC_PRIORITY_ID100_ID103_REG         =0x0;      
    GIC_PRIORITY_ID104_ID107_REG         =0x0;      
    GIC_PRIORITY_ID108_ID111_REG         =0x0;      
    GIC_PRIORITY_ID112_ID115_REG         =0x0;      
    GIC_PRIORITY_ID116_ID119_REG         =0x0;      
    GIC_PRIORITY_ID120_ID123_REG         =0x0;      
    GIC_PRIORITY_ID124_ID127_REG         =0x0;      
    GIC_PRIORITY_ID128_ID131_REG         =0x0;      
    GIC_PRIORITY_ID132_ID135_REG         =0x0;      
    GIC_PRIORITY_ID136_ID139_REG         =0x0;      
    GIC_PRIORITY_ID140_ID143_REG         =0x0;      
    GIC_PRIORITY_ID144_ID147_REG         =0x0;      
    GIC_PRIORITY_ID148_ID151_REG         =0x0;      
    GIC_PRIORITY_ID152_ID155_REG         =0x0;      
    GIC_PRIORITY_ID156_ID159_REG         =0x0;      
    GIC_PRIORITY_ID160_ID163_REG         =0x0;      
    GIC_PRIORITY_ID164_ID167_REG         =0x0;      
    GIC_PRIORITY_ID168_ID171_REG         =0x0;      
    GIC_PRIORITY_ID172_ID175_REG         =0x0;      
    GIC_PRIORITY_ID176_ID179_REG         =0x0;      
    GIC_PRIORITY_ID180_ID183_REG         =0x0;      
    GIC_PRIORITY_ID184_ID187_REG         =0x0;      
    GIC_PRIORITY_ID188_ID191_REG         =0x0;      
    GIC_PRIORITY_ID192_ID195_REG         =0x0;      
    GIC_PRIORITY_ID196_ID199_REG         =0x0;      
    GIC_PRIORITY_ID200_ID203_REG         =0x0;      
    GIC_PRIORITY_ID204_ID207_REG         =0x0;      
    GIC_PRIORITY_ID208_ID211_REG         =0x0;      
    GIC_PRIORITY_ID212_ID215_REG         =0x0;      
    GIC_PRIORITY_ID216_ID219_REG         =0x0;      
    GIC_PRIORITY_ID220_ID223_REG         =0x0;      
    GIC_PRIORITY_ID224_ID227_REG         =0x0;      
    GIC_PRIORITY_ID228_ID231_REG         =0x0;      
    GIC_PRIORITY_ID232_ID235_REG         =0x0;      
    GIC_PRIORITY_ID236_ID239_REG         =0x0;      
    GIC_PRIORITY_ID240_ID243_REG         =0x0;      
    GIC_PRIORITY_ID244_ID247_REG         =0x0;      
    GIC_PRIORITY_ID248_ID251_REG         =0x0;      
    GIC_PRIORITY_ID252_ID255_REG         =0x0;      

    /* Reset interrupt targets, no CPUs set */
    GIC_CPU_TARGET_ID0_ID3_REG           =0x0;
    GIC_CPU_TARGET_ID4_ID7_REG           =0x0;
    GIC_CPU_TARGET_ID8_ID11_REG          =0x0;
    GIC_CPU_TARGET_ID12_ID15_REG         =0x0;
    GIC_CPU_TARGET_ID16_ID19_REG         =0x0;
    GIC_CPU_TARGET_ID20_ID23_REG         =0x0;
    GIC_CPU_TARGET_ID24_ID27_REG         =0x0;
    GIC_CPU_TARGET_ID28_ID31_REG         =0x0;
    GIC_CPU_TARGET_ID32_ID35_REG         =0x0;
    GIC_CPU_TARGET_ID36_ID39_REG         =0x0;
    GIC_CPU_TARGET_ID40_ID43_REG         =0x0;
    GIC_CPU_TARGET_ID44_ID47_REG         =0x0;
    GIC_CPU_TARGET_ID48_ID51_REG         =0x0;
    GIC_CPU_TARGET_ID52_ID55_REG         =0x0;
    GIC_CPU_TARGET_ID56_ID59_REG         =0x0;
    GIC_CPU_TARGET_ID60_ID63_REG         =0x0;
    GIC_CPU_TARGET_ID64_ID67_REG         =0x0;
    GIC_CPU_TARGET_ID68_ID71_REG         =0x0;
    GIC_CPU_TARGET_ID72_ID75_REG         =0x0;
    GIC_CPU_TARGET_ID76_ID79_REG         =0x0;
    GIC_CPU_TARGET_ID80_ID83_REG         =0x0;
    GIC_CPU_TARGET_ID84_ID87_REG         =0x0;
    GIC_CPU_TARGET_ID88_ID91_REG         =0x0;
    GIC_CPU_TARGET_ID92_ID95_REG         =0x0;
    GIC_CPU_TARGET_ID96_ID99_REG         =0x0;
    GIC_CPU_TARGET_ID100_ID103_REG       =0x0;
    GIC_CPU_TARGET_ID104_ID107_REG       =0x0;
    GIC_CPU_TARGET_ID108_ID111_REG       =0x0;
    GIC_CPU_TARGET_ID112_ID115_REG       =0x0;
    GIC_CPU_TARGET_ID116_ID119_REG       =0x0;
    GIC_CPU_TARGET_ID120_ID123_REG       =0x0;
    GIC_CPU_TARGET_ID124_ID127_REG       =0x0;
    GIC_CPU_TARGET_ID128_ID131_REG       =0x0;
    GIC_CPU_TARGET_ID132_ID135_REG       =0x0;
    GIC_CPU_TARGET_ID136_ID139_REG       =0x0;
    GIC_CPU_TARGET_ID140_ID143_REG       =0x0;
    GIC_CPU_TARGET_ID144_ID147_REG       =0x0;
    GIC_CPU_TARGET_ID148_ID151_REG       =0x0;
    GIC_CPU_TARGET_ID152_ID155_REG       =0x0;
    GIC_CPU_TARGET_ID156_ID159_REG       =0x0;
    GIC_CPU_TARGET_ID160_ID163_REG       =0x0;
    GIC_CPU_TARGET_ID164_ID167_REG       =0x0;
    GIC_CPU_TARGET_ID168_ID171_REG       =0x0;
    GIC_CPU_TARGET_ID172_ID175_REG       =0x0;
    GIC_CPU_TARGET_ID176_ID179_REG       =0x0;
    GIC_CPU_TARGET_ID180_ID183_REG       =0x0;
    GIC_CPU_TARGET_ID184_ID187_REG       =0x0;
    GIC_CPU_TARGET_ID188_ID191_REG       =0x0;
    GIC_CPU_TARGET_ID192_ID195_REG       =0x0;
    GIC_CPU_TARGET_ID196_ID199_REG       =0x0;
    GIC_CPU_TARGET_ID200_ID203_REG       =0x0;
    GIC_CPU_TARGET_ID204_ID207_REG       =0x0;
    GIC_CPU_TARGET_ID208_ID211_REG       =0x0;
    GIC_CPU_TARGET_ID212_ID215_REG       =0x0;
    GIC_CPU_TARGET_ID216_ID219_REG       =0x0;
    GIC_CPU_TARGET_ID220_ID223_REG       =0x0;
    GIC_CPU_TARGET_ID224_ID227_REG       =0x0;
    GIC_CPU_TARGET_ID228_ID231_REG       =0x0;
    GIC_CPU_TARGET_ID232_ID235_REG       =0x0;
    GIC_CPU_TARGET_ID236_ID239_REG       =0x0;
    GIC_CPU_TARGET_ID240_ID243_REG       =0x0;
    GIC_CPU_TARGET_ID244_ID247_REG       =0x0;
    GIC_CPU_TARGET_ID248_ID251_REG       =0x0;
    GIC_CPU_TARGET_ID252_ID255_REG       =0x0;

   
    /* Set interrupt configuration (level high sensitive, 1-N) */
    GIC_CONFIG_ID0_ID15_REG              =0x55555555;    
    GIC_CONFIG_ID16_ID31_REG             =0x55555555;  
    GIC_CONFIG_ID32_ID47_REG             =0x55555555;  
    GIC_CONFIG_ID48_ID64_REG             =0x55555555;  
    GIC_CONFIG_ID64_ID79_REG             =0x55555555;  
    GIC_CONFIG_ID80_ID95_REG             =0x55555555;  
    GIC_CONFIG_ID96_ID111_REG            =0x55555555;  
    GIC_CONFIG_ID112_ID127_REG           =0x55555555;  
    GIC_CONFIG_ID128_ID143_REG           =0x55555555;  
    GIC_CONFIG_ID144_ID159_REG           =0x55555555;  
    GIC_CONFIG_ID160_ID175_REG           =0x55555555;  
    GIC_CONFIG_ID176_ID191_REG           =0x55555555;  
    GIC_CONFIG_ID192_ID207_REG           =0x55555555;  
    GIC_CONFIG_ID208_ID223_REG           =0x55555555;  
    GIC_CONFIG_ID224_ID239_REG           =0x55555555;  
    GIC_CONFIG_ID240_ID255_REG           =0x55555555;  

    /* enable the interrupt controller */
//    GIC_CONTROL_REG |= 0x00000001;
    tmp =  IO_IN_WORD(GIC_CONTROL_REG_ADDR)|0x1;
    IO_OUT_WORD(GIC_CONTROL_REG_ADDR    ,tmp);                     
    return ;
}   
    
    
void Sys_Interrupt_Interface_Init(void)
{
    u_int32 ii,temp;

    /* ID0~31 are aliased for each MP11 !
     * Clear up the bits of the distributor which are actually CPU-specific, all IT are "Inactive" 
     */
    GIC_CLEAR_PENDING_ID0_ID31_REG          =0xFFFFFFFF;   


    /* Reset interrupt priorities */
    GIC_PRIORITY_ID0_ID3_REG             =0x0;      
    GIC_PRIORITY_ID4_ID7_REG             =0x0;      
    GIC_PRIORITY_ID8_ID11_REG            =0x0;      
    GIC_PRIORITY_ID12_ID15_REG           =0x0;      
    GIC_PRIORITY_ID16_ID19_REG           =0x0;      
    GIC_PRIORITY_ID20_ID23_REG           =0x0;      
    GIC_PRIORITY_ID24_ID27_REG           =0x0;      
    GIC_PRIORITY_ID28_ID31_REG           =0x0;      

    
    /* Set interrupt configuration (rising edge, N-N) for ID0~31, ID0~ID15 always rising edge */
    GIC_CONFIG_ID0_ID15_REG              =0xAAAAAAAA;    
    GIC_CONFIG_ID16_ID31_REG             =0xAAAAAAAA;  


    /* Disable the CPU Interface */
    CPU_INTERFACE_CONTROL_REG = 0x00000000;

    /* Allow interrupts with higher priority (i.e. lower number) than 0xF */
//    CPU_INTERFACE_PRIORITY_MASK_REG = 0x000000F0;
    IO_OUT_WORD(CPU_INTERFACE_PRIORITY_MASK_REG_ADDR,0x000000F0);                     


    /* All priority bits are compared for pre-emption */
//    CPU_INTERFACE_BIN_POINT_REG = 0x00000003;
    IO_OUT_WORD(CPU_INTERFACE_BIN_POINT_REG_ADDR ,0x00000003);                     


    /* Clear any pending interrupts */
    ii = 0;
    do
    {
//    	temp = CPU_INTERFACE_INT_ACK_REG;
        temp = IO_IN_WORD(CPU_INTERFACE_INT_ACK_REG_ADDR);                     
        if ((temp & 0x3FF) == 1023 ) //this is SPURIOUS_INTERRUPT, no interrupt is Pending
        {
            break;
        }

//        CPU_INTERFACE_EOI_REG = temp;
        IO_OUT_WORD(CPU_INTERFACE_EOI_REG_ADDR, temp);                     

        ii++;

        /*to prevent deadlock if the CPU_INTERFACE_INT_ACK_REG never be 1023*/
        if (ii>64)
           break;

    } while (1);

    /* Enable the CPU Interface */
//    CPU_INTERFACE_CONTROL_REG = 0x00000001;
        IO_OUT_WORD(CPU_INTERFACE_CONTROL_REG_ADDR, 0x00000001);                     
}    

u_int32 Sys_Interrupt_Priority_Set(u_int32 interrupt, u_int32 priority)
{
    u_int32 word, bit_shift, temp, old_priority;

    priority &= 0xF;

    /* There are 4 interrupt priority registers per word */
    word = interrupt ; /* Register index where this interrupt priority is */


    bit_shift = (interrupt % 4) * 8 + 4; /* Offset of this interrupt's priority within the register */

    temp = old_priority = IO_IN_WORD(GIC_PRIORITY_WORD_REG_ADDR(word)); /* Get priority register */

    temp &= ~((u_int32)0xF << bit_shift); /* Reset the priority for this interrupt to 0 */

    temp |= (priority << bit_shift); /* Set the new priority */

    IO_OUT_WORD(GIC_PRIORITY_WORD_REG_ADDR(word),temp);

    return ((old_priority >> bit_shift) & 0xF); /* Return original priority */
}

void Sys_Interrupt_Target_Set(u_int32 interrupt, u_int32 cpu, u_int32 set)
{
    u_int32 word, bit_shift, temp;

    /* There are 4 interrupt target registers per word */
    word = interrupt ;
    bit_shift = (interrupt % 4) * 8;
    cpu = (1 << cpu) << bit_shift;

    temp = IO_IN_WORD(GIC_CPU_TARGET_WORD_REG_ADDR(word));
    if (set)
    {
        temp |= cpu;
    }
    else
    {
        temp &= ~cpu;
    }
    IO_OUT_WORD(GIC_CPU_TARGET_WORD_REG_ADDR(word),temp);
}


void Sys_Interrupt_Enable(u_int32 interrupt, u_int32 enable)
{
    u_int32 word;

    word = interrupt ;
    interrupt %= 32;
    interrupt = 1 << interrupt;

    if (enable) {
        IO_OUT_WORD(GIC_SET_ENABLE_WORD_REG_ADDR(word),IO_IN_WORD(GIC_SET_ENABLE_WORD_REG_ADDR(word))|interrupt );
    } else {
        IO_OUT_WORD(GIC_CLEAR_ENABLE_WORD_REG_ADDR(word),IO_IN_WORD(GIC_CLEAR_ENABLE_WORD_REG_ADDR(word))|interrupt );
    }
}


#define INTC_LEVEL_HIGH_ACTIVE                (0)
#define INTC_RISING_EDGE_SENSITIVE            (1)

#define INTC_SOFTWARE_MODEL_N_N                (0)
#define INTC_SOFTWARE_MODEL_1_N                (1)



u_int32 Hal_Gic_Register_Interrupt(unsigned int interrupt_id, 
									unsigned int trigger_mode,
									unsigned int interrupt_software_model,
									unsigned int priority_level,
									unsigned int interrupt_target_cpu)
{
	u_int32             bit_shift;

    /* configure trigger mode */
    if (trigger_mode == INTC_LEVEL_HIGH_ACTIVE)
    {
        bit_shift = ((interrupt_id % 16) << 1) + 1 ;
        /* level high trigger mode */
        IO_OUT_WORD(GIC_CONFIG_WORD_REG_ADDR(interrupt_id), IO_IN_WORD(GIC_CONFIG_WORD_REG_ADDR(interrupt_id))& (~(1 << bit_shift)));


    }
    else
    {
        bit_shift = ((interrupt_id % 16) << 1) + 1 ;
        /* rising edge trigger mode */
        IO_OUT_WORD(GIC_CONFIG_WORD_REG_ADDR(interrupt_id), IO_IN_WORD(GIC_CONFIG_WORD_REG_ADDR(interrupt_id)) |(1 << bit_shift));

    }

    /* configure software model mode */
    if (interrupt_software_model == INTC_SOFTWARE_MODEL_N_N)
    {
        bit_shift = ((interrupt_id % 16) << 1);
        /* N-N */
        IO_OUT_WORD(GIC_CONFIG_WORD_REG_ADDR(interrupt_id), IO_IN_WORD(GIC_CONFIG_WORD_REG_ADDR(interrupt_id)) & (~(1 << bit_shift)));
    }
    else
    {
        bit_shift = ((interrupt_id % 16) << 1);
        /* 1-N */

        IO_OUT_WORD(GIC_CONFIG_WORD_REG_ADDR(interrupt_id), IO_IN_WORD(GIC_CONFIG_WORD_REG_ADDR(interrupt_id)) | (1 << bit_shift));
    }

    /* configure priority level */
    Sys_Interrupt_Priority_Set(interrupt_id, priority_level);

    /* configure CPU target list */
    Sys_Interrupt_Target_Set(interrupt_id, interrupt_target_cpu, 1);

    /* Enable this specific interrupt */
    Sys_Interrupt_Enable(interrupt_id, 1);
    return 0;


}
