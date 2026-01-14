
/**
 * @file adc_test.c
 * @brief Functional test for the ADC-based input drivers (Clutch and Rotary Switch).
 *
 * @details
 * This test validates the correct operation of the analog input subsystem, which includes:
 * - ADC hardware initialization through the HAL layer.
 * - Calibration and percentage computation of the clutch pedal input.
 * - Position detection and raw value acquisition from the rotary switch.
 *
 * The test continuously reads both sensors, converts the raw ADC values into
 * physical units (percentage and discrete positions), and prints them on the console.
 *
 * This test is primarily used during bench validation or PC simulation to verify:
 * - ADC signal scaling and stability.
 * - Calibration accuracy.
 * - Correct mapping between analog signals and digital representations.
 *
 * @note
 * To step through the readings, press **Enter** after each iteration.
 * For continuous automatic testing, replace `getchar()` with a timed delay (e.g., `usleep()`).
 */

#include "../hal/hal_adc.h"
#include "../drivers/clutch.h"
#include "../drivers/rotary_switch.h"
#include "adc_test.h"
#include "hal_delay.h"

#include <stdio.h>


/**
 * @brief Executes the ADC functional test.
 *
 * @details
 * Initializes the ADC hardware and both analog drivers (clutch and rotary switch),
 * applies optional calibration settings, and repeatedly measures and prints
 * their current values to the console.
 *
 * @return void
 */
void adc_test(void) {

    /* ----------------------- Initialization Section ----------------------- */

    hal_adc_init(); // Initialize the ADC peripheral (HAL layer)
    
    clutch_Init(); // Initialize the clutch input driver
    clutch_SetCalibration(400,4000); /// Optional calibration (min, max ADC values)

    rotary_Init(10); // Initialize the rotary switch with 10 discrete positions 
    //rotary_SetCalibration(400,400) // Optional calibration for rotary input

    /* ------------------------- Test Execution Loop ------------------------ */
    while(1) {

        /* --- Clutch measurement --- */
        uint16_t raw_c=clutch_GetRawValue(); // Read raw ADC value from clutch input
        float clutch=clutch_GetPercentage(); // Get calibrated clutch position (0–100%)

        //uint16_t raw_c = hal_adc_read(0);
        printf("Clutch: %.1f%% | Raw value: %u\n", clutch, raw_c);

        /* --- Rotary switch measurement --- */
        uint16_t raw_r=rotary_GetRawValue();    // Read raw ADC value from rotary switch
        uint8_t position=rotary_GetPosition();  // Determine the current position index

        //uint16_t raw_r = hal_adc_read(1);
        printf("Position: %u | Raw value: %u\n", position, raw_r);

         /* --- Pause for user input --- */
        //getchar(); // Wait for Enter key (manual step mode)
        HAL_DelayMs(50);
    }
}