
#include <cns3000.h>
#include "misc.h"

#define PMU_WRITE_REGl(reg, value)      (*(volatile unsigned long*)(reg) = (value))
#define PMU_READ_REGl(reg)              (*(volatile unsigned long*)(reg))

#define CNS3XXX_PM_BASE             0x77000000  /* Power Management Control */
#define PMU_REG_VALUE(offset) (*((volatile unsigned int *)(CNS3XXX_PM_BASE+offset)))

#define CNS3XXX_TC11MP_SCU_BASE				0x90000000  /* IRQ, Test chip */
#define CPU_INT_REG_VALUE(offset) (*((volatile unsigned int *)(CNS3XXX_TC11MP_SCU_BASE+offset)))
#define CNS3XXX_TC11MP_GIC_DIST_BASE        0x90001000  /* Test chip interrupt controller distributor */
#define GIC_REG_VALUE(offset) (*((volatile unsigned int *)(CNS3XXX_TC11MP_GIC_DIST_BASE+offset)))

#define PM_CLK_GATE_REG             PMU_REG_VALUE(0x000)
#define PM_SOFT_RST_REG             PMU_REG_VALUE(0x004)
#define PM_HS_CFG_REG               PMU_REG_VALUE(0x008)
#define PM_CACTIVE_STA_REG          PMU_REG_VALUE(0x00C)
#define PM_PWR_STA_REG              PMU_REG_VALUE(0x010)
#define PM_CLK_CTRL_REG             PMU_REG_VALUE(0x014)
#define PM_PLL_LCD_I2S_CTRL_REG     PMU_REG_VALUE(0x018)
#define PM_PLL_HM_PD_CTRL_REG       PMU_REG_VALUE(0x01C)
#define PM_REGULAT_CTRL_REG         PMU_REG_VALUE(0x020)
#define PM_WDT_CTRL_REG             PMU_REG_VALUE(0x024)
#define PM_WU_CTRL0_REG             PMU_REG_VALUE(0x028)
#define PM_WU_CTRL1_REG             PMU_REG_VALUE(0x02C)
#define PM_CSR_REG                  PMU_REG_VALUE(0x030)

/* Change CPU frequency and divider */
#define CNS3XXX_PWR_PLL_CPU_300MHZ      (0)
#define CNS3XXX_PWR_PLL_CPU_333MHZ      (1)
#define CNS3XXX_PWR_PLL_CPU_366MHZ      (2)
#define CNS3XXX_PWR_PLL_CPU_400MHZ      (3)
#define CNS3XXX_PWR_PLL_CPU_433MHZ      (4)
#define CNS3XXX_PWR_PLL_CPU_466MHZ      (5)
#define CNS3XXX_PWR_PLL_CPU_500MHZ      (6)
#define CNS3XXX_PWR_PLL_CPU_533MHZ      (7)
#define CNS3XXX_PWR_PLL_CPU_566MHZ      (8)
#define CNS3XXX_PWR_PLL_CPU_600MHZ      (9)
#define CNS3XXX_PWR_PLL_CPU_633MHZ      (10)
#define CNS3XXX_PWR_PLL_CPU_666MHZ      (11)
#define CNS3XXX_PWR_PLL_CPU_700MHZ      (12)

#define CNS3XXX_PWR_CPU_CLK_DIV_BY1     (0)
#define CNS3XXX_PWR_CPU_CLK_DIV_BY2     (1)
#define CNS3XXX_PWR_CPU_CLK_DIV_BY4     (2)

#define PM_PLL_CPU_SEL(CPU) { \
    PM_CLK_CTRL_REG &= ~((0xF) << PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL); \
    PM_CLK_CTRL_REG |= (((CPU)&0xF) << PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL); \
}

#define PM_CPU_CLK_DIV(DIV) { \
    PM_CLK_CTRL_REG &= ~((0x3) << PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV); \
    PM_CLK_CTRL_REG |= (((DIV)&0x3) << PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV); \
}

/* PM_CLK_CTRL_REG */
#define PM_CLK_CTRL_REG_OFFSET_I2S_MCLK         (31)
#define PM_CLK_CTRL_REG_OFFSET_DDR2_CHG_EN      (30)
#define PM_CLK_CTRL_REG_OFFSET_PCIE_REF1_EN     (29)
#define PM_CLK_CTRL_REG_OFFSET_PCIE_REF0_EN     (28)
#define PM_CLK_CTRL_REG_OFFSET_TIMER_SIM_MODE   (27)
#define PM_CLK_CTRL_REG_OFFSET_I2SCLK_DIV       (24)
#define PM_CLK_CTRL_REG_OFFSET_I2SCLK_SEL       (22)
#define PM_CLK_CTRL_REG_OFFSET_CLKOUT_DIV       (20)
#define PM_CLK_CTRL_REG_OFFSET_CLKOUT_SEL       (16)
#define PM_CLK_CTRL_REG_OFFSET_MDC_DIV          (14)
#define PM_CLK_CTRL_REG_OFFSET_CRYPTO_CLK_SEL   (12)
#define PM_CLK_CTRL_REG_OFFSET_CPU_PWR_MODE     (9)
#define PM_CLK_CTRL_REG_OFFSET_PLL_DDR2_SEL     (7)
#define PM_CLK_CTRL_REG_OFFSET_DIV_IMMEDIATE    (6)
#define PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV      (4)
#define PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL      (0)

#define CNS3XXX_PWR_CPU_MODE_DFS        (0)
#define CNS3XXX_PWR_CPU_MODE_IDLE       (1)
#define CNS3XXX_PWR_CPU_MODE_HALT       (2)
#define CNS3XXX_PWR_CPU_MODE_DOZE       (3)
#define CNS3XXX_PWR_CPU_MODE_SLEEP      (4)
#define CNS3XXX_PWR_CPU_MODE_HIBERNATE  (5)


static void cns3xxx_pwr_mode(unsigned int pwr_mode)
{
    if (CNS3XXX_PWR_CPU_MODE_HIBERNATE < pwr_mode) {
        return;
    }

    PM_CLK_CTRL_REG &=
            ~(0x7<<PM_CLK_CTRL_REG_OFFSET_CPU_PWR_MODE);
    PM_CLK_CTRL_REG |=
            ((pwr_mode&0x7)<<PM_CLK_CTRL_REG_OFFSET_CPU_PWR_MODE);
};

static inline cns3xxx_wfi(void)
{
#if 0
    __asm__ __volatile__(
			"nop\n"
			"nop\n"
            "mov r0, #0\n"
            "mcr p15, 0, r0, c7, c0, 4\n"
			"nop\n"
			"nop\n"
			"nop\n"
            );
#else
	unsigned int zero = 0;
	__asm__ __volatile__("nop\n" "mcr   p15, 0, %0, c7, c0, 4" : : "r" (zero) : "cc");
#endif


}

/* Change CPU frequency and divider */
/*
 * cns3xxx_pwr_change_pll_cpu - change PLL CPU frequency
 * @cpu_sel: PLL CPU frequency
 * @div_sel: divider
 */
void cns3xxx_pwr_change_cpu_clock(unsigned int cpu_sel, unsigned int div_sel)
{
	
	disable_interrupts();
	Sys_Interrupt_Distributor_Init();
    Sys_Interrupt_Interface_Init();
#define INTC_LEVEL_HIGH_ACTIVE                (0)
#define INTC_RISING_EDGE_SENSITIVE            (1)

#define INTC_SOFTWARE_MODEL_N_N                (0)
#define INTC_SOFTWARE_MODEL_1_N                (1)

	Hal_Gic_Register_Interrupt(32, 
								INTC_LEVEL_HIGH_ACTIVE,
                                INTC_SOFTWARE_MODEL_N_N,
                                0,
                                0);
	Hal_Gic_Register_Interrupt(95, 
								INTC_RISING_EDGE_SENSITIVE,
                                INTC_SOFTWARE_MODEL_N_N,
                                0,
                                0);
	/* 1. Set PLL_CPU_SEL */
	if ((CNS3XXX_PWR_PLL_CPU_700MHZ < cpu_sel)
            || (CNS3XXX_PWR_CPU_CLK_DIV_BY4 < div_sel)) {
        return;
    }

    PM_PLL_CPU_SEL(cpu_sel);
    PM_CPU_CLK_DIV(div_sel);

	/* 2. Set in DFS mode */
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DFS);

	/* 3. disable all interrupt except interrupt ID-32 (clkscale_intr) */
	{
		int i;
		for (i=32; i<96;i++) {
			Sys_Interrupt_Enable(i, 0);
		}
	}
	Sys_Interrupt_Enable(32, 1);
	Sys_Interrupt_Enable(95, 1);
    

	/* set DMC to low power hand shake */
	PM_HS_CFG_REG |= (0x1 << 2);
	/* disable DMC */
	PM_CLK_GATE_REG &= ~(0x1<<2);

	enable_interrupts();

	/* 4. Let CPU enter into WFI state */
	CPU_INT_REG_VALUE(0x100) = 0x1;
	CPU_INT_REG_VALUE(0x104) = 0xF0;
	//printf("enter into WFI\n");
	cns3xxx_wfi();
	//printf("leave WFI\n");

#if 0
	{
		/* Add this section for SMC clock */
		int real_cpu , aclk;
		int temp = PM_PLL_LCD_I2S_CTRL_REG;
		real_cpu = 300 + ((cpu_sel&0xf) *33);
		real_cpu = (real_cpu >> div_sel);
		#if 0
		printf("cpu_sel %d, div_sel %d\n", cpu_sel, div_sel);
		printf("CPU is running at %d MHz\n", real_cpu);
		#endif

		aclk = real_cpu/2;
	
		temp &= ~(0x3<<22);
		/* the max. SMC clock is 100Mhz*/	
		if ((aclk/2) <= 100) {
			temp |= (0<<22);	
			/* printf("SMC clk is divided by 2\n"); */
		} else if ((aclk/3) <= 100) {
			temp |= (1<<22);	
			//printf("SMC clk is divided by 3\n");
		} else if ((aclk/4) <= 100) {
			temp |= (2<<22);	
			//printf("SMC clk is divided by 4\n");
		} else if ((aclk/6) <= 100) {
			temp |= (3<<22);	
			//printf("SMC clk is divided by 6\n");
		}
		PM_PLL_LCD_I2S_CTRL_REG = temp;
	
	}
#endif
}

void cns3xxx_cpu_clock_scale_start(void) 
{
	unsigned long smc_div;

	{
		int temp = PM_CLK_CTRL_REG;
		
		/* share pin configuration */
		//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 26);
		//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 27);
		//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 28);
		//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 29);
		/* clock out select */
		temp &=~(0x3 << 20);
		temp &=~(0xf << 16);
		temp |= (0x3 << 20);
		temp |= (0x1 << 16);
		PM_CLK_CTRL_REG = temp;
	}
	
	{
#ifndef NULL
#define NULL    0
#endif

		char *s;
		unsigned int env_pll_clock, env_pll_div, cpu_sel, div_sel;

		if ((s = getenv("cpu_clock")) != NULL) {
			env_pll_clock = simple_strtoul(s, NULL, 10);
		} else {
			env_pll_clock = 300;	
    	}
		
		if ((s = getenv("cpu_div")) != NULL) {
			env_pll_div = simple_strtoul(s, NULL, 10);
		} else {
			env_pll_div = 1;	
    	}

		switch (env_pll_div) {
			case 1: div_sel = 0; break;
			case 2: div_sel = 1; break;
			case 4: div_sel = 2; break;
			default: div_sel = 0;
		}

		switch (env_pll_clock) {
			case 300: cpu_sel = 0; break;
			case 333: cpu_sel = 1; break;
			case 366: cpu_sel = 2; break;
			case 400: cpu_sel = 3; break;
			case 433: cpu_sel = 4; break;
			case 466: cpu_sel = 5; break;
			case 500: cpu_sel = 6; break;
			case 533: cpu_sel = 7; break;
			case 566: cpu_sel = 8; break;
			case 600: cpu_sel = 9; break;
			case 633: cpu_sel = 10; break;
			case 666: cpu_sel = 11; break;
			case 700: cpu_sel = 12; break;
			default:
				cpu_sel = 0; 
		}

		
		if (((PM_CLK_CTRL_REG >> PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL) & 0xf) == cpu_sel) {
			if (((PM_CLK_CTRL_REG >>PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV) & 0x3) == div_sel) {
				printf("no change to clock\n");
				return; /* no change*/
			}
		}
		
		cns3xxx_pwr_change_cpu_clock(cpu_sel, div_sel);

		/* change SMC clock for different CPU clock */
		smc_div = PMU_READ_REGl(CNS3000_VEGA_PMU_BASE + PM_PLL_LCD_I2S_CTRL_OFFSET);
		switch (env_pll_clock) {
			case 300: smc_div |= (0x0 << 22); break;	/* aclk/2 */
			case 333: smc_div |= (0x0 << 22); break;	/* aclk/2 */
			case 366: smc_div |= (0x0 << 22); break;	/* aclk/2 */
			case 400: smc_div |= (0x0 << 22); break;	/* aclk/2 */
			case 433: smc_div |= (0x0 << 22); break;	/* aclk/2 */
			case 466: smc_div |= (0x0 << 22); break;	/* aclk/2 */
			case 500: smc_div |= (0x1 << 22); break;	/* aclk/3 */
			case 533: smc_div |= (0x1 << 22); break;	/* aclk/3 */
			case 566: smc_div |= (0x1 << 22); break;	/* aclk/3 */
			case 600: smc_div |= (0x1 << 22); break;	/* aclk/3 */
			default:
				smc_div |= (0x0 << 22);
		}
		PMU_WRITE_REGl(CNS3000_VEGA_PMU_BASE + PM_PLL_LCD_I2S_CTRL_OFFSET, smc_div);
	}
}

void cns3xxx_cpu_clock_show(void)
{

#if 0
	printf("PLL CPU Frequency: ");
    switch (PM_CLK_CTRL_REG&0xf) {
    case 0: printf("300MHz\n"); break;
    case 1: printf("333MHz\n"); break;
    case 2: printf("366MHz\n"); break;
    case 3: printf("400MHz\n"); break;
    case 4: printf("433MHz\n"); break;
    case 5: printf("466MHz\n"); break;
    case 6: printf("500MHz\n"); break;
    case 7: printf("533MHz\n"); break;
    case 8: printf("566MHz\n"); break;
    case 9: printf("600MHz\n"); break;
    default:
        printf("!!!!!\n");
    }

    printf("CPU clock divider: %d\n", (0x1 << ((PM_CLK_CTRL_REG>>4)&0x3)));
#endif
#define CPU_BASE 300
    int cpu, cpu_sel, div_sel;

    cpu_sel = (PM_CLK_CTRL_REG >> PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL) & 0xf;
    div_sel = (PM_CLK_CTRL_REG >> PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV) & 0x3;

    cpu = (CPU_BASE + ((cpu_sel/3) * 100) + ((cpu_sel %3) *33)) >> div_sel;
	printf("Change CPU frequence to %d MHz (%d/%d)\n", cpu, cpu<<div_sel, 0x1<<div_sel);


    return ;
}

