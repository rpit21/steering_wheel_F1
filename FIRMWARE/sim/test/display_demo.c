/**
 * @file display_demo.c
 * @brief Functional demonstration for the OLED display driver.
 *
 * @details
 * This test validates the initialization and rendering functions of the
 * display module, using the corresponding HAL for low-level hardware access.
 *
 * The demonstration performs the following sequence:
 * - Initializes the display with specified resolution and scale factor.
 * - Enters a rendering loop that updates the screen at ~60 FPS.
 * - Draws a static numeric value for visualization.
 * - Polls for system or user events (to exit the loop).
 *
 * This test is useful for verifying:
 * - Correct display initialization and refresh.
 * - Communication between driver and HAL.
 * - Frame buffer flushing and synchronization.
 *
 * @note
 * The demo assumes that the HAL provides event polling and delay functions
 * (`hal_poll_events()` and `hal_delay_ms()`).  
 * It can be adapted for both PC simulation (SDL or similar) and target MCU display drivers.
 */

#include "../drivers/display.h"
#include "../hal/hal_display.h"
#include "display_demo.h"

/**
 * @brief Executes a demonstration of the display driver.
 *
 * @details
 * Initializes the display and continuously refreshes it at approximately
 * 60 frames per second. The demo draws a fixed number on the screen as a
 * visual indicator of correct rendering operation.
 *
 * The loop continues running until `hal_poll_events()` sets the `running`
 * flag to zero (e.g., user closes the window or exits the simulation).
 *
 * @return void
 */
void display_demo(void)
{
    /* Initialize display with width=128, height=64, and 4× scaling */
    if (display_init(128, 64, 4) != 0) 
        return; //Exit if initialization fails 

    int running = 1;
    int t = 0;

    /* --- Main rendering loop --- */
    while (running) {
        hal_poll_events(&running); /* Handle input or window events */

        /* Draw test content (e.g., number at coordinates x=100, y=20) */
        display_draw_number(100,20,1,0);

        /* Update (flush) the display buffer to visible screen */
        display_flush();

        /* Maintain ~60 FPS refresh rate */
        hal_delay_ms(16); 
    }

    /* Shutdown and free display resources */
    display_shutdown();
}


