/**
 * @file hal_delay.h
 * @brief Hardware Abstraction Layer (HAL) interface for blocking delay functions.
 *
 * @details
 * This module provides platform-independent delay routines that pause program
 * execution for a defined duration in either milliseconds or microseconds.
 * 
 * The implementation differs depending on the build target:
 * - **Target MCU**: Implements precise time delays using calibrated busy-wait loops
 *   or hardware timer peripherals.
 * - **Host (Linux/PC)**: Uses system calls such as `usleep()` from <unistd.h> to
 *   emulate timing delays for simulation or testing.
 *
 * Typical use cases:
 * - Waiting for hardware stabilization (e.g., sensor warm-up).
 * - Creating short software-based timing intervals.
 * - Debugging time-dependent firmware behavior.
 *
 * @note
 * These functions perform *blocking* delays — they halt CPU execution until the
 * specified interval elapses. They should be avoided in time-critical or
 * multitasking environments where non-blocking timing mechanisms are preferred.
 */

#ifndef HAL_DELAY_H
#define HAL_DELAY_H

// --- INCLUDES ---
#include <stdint.h> /**< Provides fixed-width integer types like uint32_t. */

/*--------------------------PUBLIC API FUNCTIONS-----------------------------------*/

void HAL_LPIT0_Init(void);

int HAL_LPIT_DelayUs(uint32_t us);

int HAL_LPIT_DelayMs(uint32_t ms);

/**
 * @brief Delays program execution for a specified number of milliseconds.
 *
 * @details
 * Suspends execution for the given duration in milliseconds.  
 * On MCU targets, the delay is typically achieved via calibrated loop counts
 * or hardware timer-based wait routines.  
 * On host systems, it uses standard OS sleep functions for approximate timing.
 *
 * @param[in] ms Duration of the delay, in milliseconds.
 */
void HAL_DelayMs(uint32_t ms);

/**
 * @brief Delays program execution for a specified number of microseconds.
 *
 * @details
 * Suspends execution for the given duration in microseconds.  
 * Used for precise timing requirements such as peripheral initialization
 * sequences or bit-banging communication protocols.  
 * Implementation is platform-dependent and may have limited accuracy on host systems.
 *
 * @param[in] us Duration of the delay, in microseconds.
 */
void HAL_DelayUs(uint32_t us);

#endif /* HAL_DELAY_H */
