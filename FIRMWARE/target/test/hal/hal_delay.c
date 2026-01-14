#include "S32K118.h"
#include "hal_delay.h"

#define LPIT_CLK_HZ 48000000u   /* FIRCDIV2 = 48 MHz */

/* Factor empírico medido con osciloscopio */
#define NOP_LOOP_FACTOR 12u

extern uint32_t SystemCoreClock;  // defined in system_S32K118.c (normally 48MHz)

void HAL_LPIT0_Init(void)
{
    /* 1) PCC: configurar clock LPIT = FIRCDIV2 (PCS=3) */
    IP_PCC->PCCn[PCC_LPIT_INDEX] &= ~PCC_PCCn_CGC_MASK;     // CGC = 0
    IP_PCC->PCCn[PCC_LPIT_INDEX] =
          PCC_PCCn_PCS(3)          // FIRCDIV2
        | PCC_PCCn_PCD(0)          // divide by 1
        | PCC_PCCn_CGC_MASK;       // CGC = 1

    /* 2) Habilitar módulo LPIT */
    IP_LPIT0->MCR =
          LPIT_MCR_M_CEN(1)        // enable timers
        | LPIT_MCR_DBG_EN(1)       // freeze in debug (MUY IMPORTANTE)
        | LPIT_MCR_DOZE_EN(0);

    /* 3) Software reset (opcional pero limpio) */
    //IP_LPIT0->MCR |= LPIT_MCR_SW_RST(1);
    //for (volatile int i = 0; i < 64; i++) { __asm("nop"); }
    //IP_LPIT0->MCR &= ~LPIT_MCR_SW_RST_MASK;

    /* Limpia flags */
    IP_LPIT0->MSR = 0xF;
}

int HAL_LPIT_DelayUs(uint32_t us)
{
    uint64_t ticks64 = ((uint64_t)LPIT_CLK_HZ * us) / 1000000ULL;
    if (ticks64 == 0 || ticks64 > 0xFFFFFFFFULL) return -1;

    uint32_t ticks = (uint32_t)ticks64;

    /* Asegura canal apagado */
    IP_LPIT0->TMR[0].TCTRL = 0;
    IP_LPIT0->MSR = LPIT_MSR_TIF0_MASK;

    IP_LPIT0->TMR[0].TVAL = ticks - 1u;

    /* MODE=32bit periodic, TSOI=1 (one-shot) */
    IP_LPIT0->TMR[0].TCTRL =
          LPIT_TMR_TCTRL_MODE(0)
        | LPIT_TMR_TCTRL_TSOI(1)
        | LPIT_TMR_TCTRL_T_EN(1);

    /* Espera con guard para no colgar MCU */
    uint32_t guard = 0xFFFFFFu;
    while (((IP_LPIT0->MSR & LPIT_MSR_TIF0_MASK) == 0u) && guard--) {}

    IP_LPIT0->TMR[0].TCTRL = 0;
    IP_LPIT0->MSR = LPIT_MSR_TIF0_MASK;

    return (guard == 0u) ? -2 : 0;
}

int HAL_LPIT_DelayMs(uint32_t ms)
{
    while (ms--)
    {
        int r = HAL_LPIT_DelayUs(1000u);
        if (r != 0) return r;
    }
    return 0;
}



void HAL_DelayMs(uint32_t ms) {

	 while (ms--)
	    {
	        HAL_DelayUs(1000u);
	    }
}

/**
 * @brief Blocking delay in microseconds using CPU cycles.
 *
 * Formula:
 *   cycles_per_us = SystemCoreClock / 1,000,000
 */
void HAL_DelayUs(uint32_t us) {
	uint32_t cycles_per_us = SystemCoreClock / 1000000u;

	/* Correct the error from the loop */
	uint32_t total_cycles = (cycles_per_us * us) / NOP_LOOP_FACTOR;

	    if (total_cycles == 0u){
	    	total_cycles = 1u;
	    }

	    while (total_cycles--)
	    {
	        __asm volatile ("nop");
	    }
}
