// hal_delay.c
// Host PC (simulation) version of the Hardware Abstraction Layer (HAL) for delays.
// Provides blocking delays in milliseconds and microseconds using `usleep()`.

// --- INCLUDES ---
#include "hal_delay.h" // HAL function prototypes
#include <unistd.h>    // usleep() function

// --- PUBLIC FUNCTIONS ---

/**
 * @brief Blocks execution for a specified number of milliseconds.
 * @param ms Number of milliseconds to delay.
 *
 * @details
 * Converts milliseconds to microseconds and calls usleep() for blocking delay.
 * On embedded targets, this function would typically use a hardware timer or busy-wait loop.
 */
void HAL_DelayMs(uint32_t ms) { 
    usleep(ms * 1000); 
}

/**
 * @brief Blocks execution for a specified number of microseconds.
 * @param us Number of microseconds to delay.
 *
 * @details
 * Directly calls usleep() for fine-grained delay. Resolution depends on the
 * host OS scheduler; on embedded targets, a more precise timer may be used.
 */
void HAL_DelayUs(uint32_t us) { 
    usleep(us); 
}
