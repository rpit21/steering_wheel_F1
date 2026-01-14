/**
 * @file main.c
 * @brief Entry point of the F1 Steering Wheel Simulator application.
 *
 * @details
 * This file defines the main entry point of the program. Its primary responsibility
 * is to initialize the system and transfer control either to:
 * - The **main application logic** (`app_main()`), or
 * - Specific **test modules** (e.g., button, ADC, CAN, or TFT tests),
 *   depending on which function is uncommented.
 *
 * The file acts as a flexible launcher for both development and production modes.
 *
 * **Usage example:**
 * - Uncomment `button_test()` to run only the button testing routine.
 * - Keep `app_main()` active to run the full system simulation.
 *
 * @note
 * Only one of the test calls or the main application call should be active at a time.
 * The test functions are useful for debugging individual modules in isolation.
 */

/*----------------------------------INCLUDES---------------------------------------------*/
#include "app/app_main.h"       /**< Declaration for the main application entry function `app_main()`. */
#include "test/button_test.h"   /**< Declaration for the button testing routine. */
#include "test/adc_test.h"      /**< Declaration for the ADC testing routine. */
#include "test/can_test.h"      /**< Declaration for the CAN testing routine. */
#include "test/tft_test.h"      /**< Declaration for the TFT display testing routine. */
#include <stdio.h>              /**< Standard I/O library (used for debugging output). */

/*----------------------------FORWARD DECLARATION----------------------------------------*/
/**
 * @brief Main application entry point implemented in `app/app_main.c`.
 *
 * @details
 * Handles initialization of all modules, starts the main control loop,
 * and manages CAN communication and display updates.
 */
void app_main(void);

/*-------------------------------------MAIN--------------------------------------------*/

/**
 * @brief Program entry point.
 *
 * @details
 * The first function executed when the program starts.
 * It can either run the main application or specific test routines
 * depending on which function calls are enabled.
 *
 * @param[in] argc Number of command-line arguments.
 * @param[in] argv Array of argument strings.
 * @return Returns 0 if the program terminated successfully.
 */
int main(int argc, char** argv) {

    // The code below allows you to easily switch between running the main application
    // and running isolated test functions by commenting/uncommenting the relevant lines.
    // This is a common and useful practice for debugging individual modules.

    //watchdog disable
    //initialization of uart
    printf("[BOOT] Watchdog disabled\n");

    //Clocks configurations

    printf("[BOOT] Clocks set: Core=%u Hz, Bus=%u Hz, Flash=%u Hz\n",
	                    48000000u, 48000000u, 48000000u/2);


    printf("[BOOT] MCU: S32K118 OK (SIM)\n");

    /*----------------------TEST FUNCTION CALLS (Commented Out)---------------------------*/
    /* Uncomment one of the following lines to run a specific module test: */

    // By uncommenting this line, the program would only run the button test routine.
    //button_test();
    
    // By uncommenting this line, the program would only run the ADC test routine.
    //adc_test();

    // By uncommenting this line, the program would only run the CAN test routine.
    //can_test();

    // By uncommenting this line, the program would only run the SPI test routine.
    //spi_test();

    // By uncommenting this line, the program would only run the LCD test routine.
    //lcd_test();

    // By uncommenting this line, the program would only run the LCD test routine.
    //tft_test();

    /*---------------------MAIN APPLICATION CALL (Active)------------------------------*/
    
    /** Transfers control to the full application logic implemented in app_main.c. */
    printf("[BOOT] Entering app_main()\n");
    app_main();

    /** Return 0 to indicate successful termination. */
    return 0;
}
