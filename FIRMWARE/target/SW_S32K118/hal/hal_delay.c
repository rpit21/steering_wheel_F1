#include "hal_delay.h"
#include "device_registers.h"

extern uint32_t SystemCoreClock;  // defined in system_S32K118.c (normally 48MHz)

/* Factor empírico medido con osciloscopio */
#define NOP_LOOP_FACTOR 12u


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
