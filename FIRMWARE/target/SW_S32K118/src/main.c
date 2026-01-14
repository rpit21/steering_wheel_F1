/*
 * main.c - Entry point for F1 Steering Wheel firmware on S32K118.
 *
 * This file contains the minimal startup code for the embedded target:
 *  - performs optional low-level MCU initialization (if required),
 *  - calls app_main(), which runs the steering wheel application.
 *
 * All hardware-specific initialization (GPIO, ADC, SPI, CAN, Display, etc.)
 * is performed inside app_main() and the underlying HAL modules.
 */

#include "system_S32K118.h"
#include "app_main.h"
#include "../hal/hal_wdog.h"
#include "../hal/hal_clocks.h"
#include "../hal/hal_uart.h"
#include <stdio.h>   /* Optional: for debug prints, may be redirected to UART or semihosting */

extern uint32_t SystemCoreClock;  // defined in system_S32K118.c (normally 48MHz)

/**
 * @brief Main entry point for the S32K118 target firmware.
 */
int main(void)
{

	/* Optional: You can disable the WatchDog for test and debugging
	 * It is necessary to reactive for the final Firmware
	 */
	HAL_WDOG_Disable();
	SOSC_init_20MHz();
	RUN_mode_48MHz();
	SystemCoreClockUpdate(); // (Optional) evaluates the clock register settings and calculates the current core clock.
	HAL_UART_Init(115200); // Inizialization of the uart
	HAL_UART_Printf("[BOOT] Watchdog disabled\r\n");



    /* Optional: if you have a dedicated clocks HAL, you can call it here.
     * Example:
     *     HAL_Clocks_Init(); --> RUN_mode_48MHz();
     *
     * Otherwise, we assume startup code has configured system clocks.
     */

	HAL_UART_Printf("[BOOT] Clocks set: Core=%lu Hz, Bus=%lu Hz, Flash=%lu Hz\r\n",
	                    SystemCoreClock, SystemCoreClock, SystemCoreClock/2);




	HAL_UART_Printf("[BOOT] MCU: S32K118 OK\r\n");

    /* Run the main steering wheel application.
     * app_main() is expected to:
     *  - initialize GPIO, ADC, SPI, CAN, Display, buttons, etc.
     *  - enter the main control loop and never return.
     */
    HAL_UART_Printf("[BOOT] Entering app_main()\r\n");
    app_main();

    /* If app_main() ever returns, stay in a safe infinite loop. */
    for (;;)
    {
        /* Optional: low-power wait, error indication, etc. */
    }

    /* This return is never reached, but kept to satisfy the compiler. */
    return 0;
}
