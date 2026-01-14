/**
 * @file hal_wdog.h
 * @brief HAL interface for Watchdog (WDOG) on NXP S32K118.
 *
 * Provides initialization, enabling, disabling, and refresh functions
 * for the hardware watchdog module.
 */


#ifndef HAL_WDOG_H_
#define HAL_WDOG_H_

#include <stdint.h>

/**
 * @brief Disable the hardware watchdog.
 *
 * This should be used ONLY in development/debug mode.
 */
void HAL_WDOG_Disable(void);

/**
 * @brief Enable the watchdog with a given timeout in milliseconds.
 *
 * @param timeout_ms Timeout before watchdog reset (1ms–32768ms).
 */
void HAL_WDOG_Enable(uint32_t timeout_ms);

/**
 * @brief Refresh (kick) the watchdog timer.
 *
 * Must be called periodically before timeout expires.
 */
void HAL_WDOG_Refresh(void);

#endif /* HAL_WDOG_H_ */
