/**
 * @file hal_wdog.c
 * @brief HAL implementation for Watchdog (WDOG) on NXP S32K118.
 */

#include "hal_wdog.h"
#include "device_registers.h"
#include "hal_uart.h"

/* Unlock key values required by WDOG hardware */
#define WDOG_UNLOCK_WORD_1   (0xD928C520U)
#define WDOG_UNLOCK_WORD_2   (0xB480A602U)

/*======================================================================
 *  INTERNAL UTILITIES
 *====================================================================*/

/**
 * @brief Unlocks the WDOG so its registers can be modified.
 */
static void WDOG_Unlock(void)
{
    IP_WDOG->CNT = WDOG_UNLOCK_WORD_1;  /* First key */
    IP_WDOG->CNT = WDOG_UNLOCK_WORD_2;  /* Second key */
}

/*======================================================================
 *  PUBLIC API
 *====================================================================*/

void HAL_WDOG_Disable(void)
{
    WDOG_Unlock();

    /* Disable watchdog:
     * - CS[EN] = 0  (disable)
     * - Update allowed
     * - Clock: LPO
     */
    IP_WDOG->CS =
            WDOG_CS_CMD32EN_MASK |     /* Allow 32-bit command write */
            WDOG_CS_UPDATE_MASK;       /* Allow reconfiguration */
    //HAL_UART_Printf("[WDOG] Disabled: CS=0x%08lX\n", (uint32_t)IP_WDOG->CS);
}

void HAL_WDOG_Enable(uint32_t timeout_ms)
{
    if (timeout_ms == 0)
        timeout_ms = 1;

    /* Timeout register counts in LPO cycles (1 kHz). */
    uint32_t toval = timeout_ms;

    if (toval > 0xFFFF)
        toval = 0xFFFF;

    WDOG_Unlock();

    /* Set timeout */
    IP_WDOG->TOVAL = WDOG_TOVAL_TOVALLOW(toval);

    /* Enable watchdog:
     * - EN = 1
     * - UPDATE = 1   -> allow future reconfigurations
     * - CLK = LPO (1kHz)
     * - INT = 0 -> reset only
     * - WIN disabled
     */
    IP_WDOG->CS =
            WDOG_CS_CMD32EN_MASK  |    /* Enable 32-bit commands */
            WDOG_CS_UPDATE_MASK   |    /* Allow future changes */
			WDOG_CS_CLK(0b01)	  |		/* Low-power 1kHz oscillator */
            WDOG_CS_EN_MASK;           /* Enable watchdog */
    /*HAL_UART_Printf("[WDOG] Enabled timeout=%lums: CS=0x%08lX TOVAL=0x%08lX\n",
                    (unsigned long)timeout_ms,
                    (uint32_t)IP_WDOG->CS,
                    (uint32_t)IP_WDOG->TOVAL);*/
}

void HAL_WDOG_Refresh(void)
{
    /* Refresh sequence: write unlock words to CNT */
    IP_WDOG->CNT = WDOG_UNLOCK_WORD_1;
    IP_WDOG->CNT = WDOG_UNLOCK_WORD_2;
}


