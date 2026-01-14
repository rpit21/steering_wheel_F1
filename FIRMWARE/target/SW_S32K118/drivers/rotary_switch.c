
/**
 * @file rotary_switch.c
 * @brief Implementation of the rotary switch driver.
 *
 * @details
 * This file contains the logic for reading, calibrating, and interpreting
 * the analog signal from a rotary switch connected to an ADC channel.
 *
 * The driver performs a linear conversion between raw ADC values and
 * discrete switch positions, providing a stable and calibrated interface
 * for upper application layers.
 *
 * The implementation depends on the HAL ADC module for raw value acquisition.
 */

#include "../hal/hal_adc.h"
#include "rotary_switch.h"
#include <stdlib.h> // Required for abs() function

/** @brief Stores the most recent raw ADC reading. */
static uint16_t raw;

/** @brief Minimum calibrated ADC value (corresponds to position 0). */
static uint16_t min_raw;

/** @brief Maximum calibrated ADC value (corresponds to the highest position). */
static uint16_t max_raw;

/** @brief Total number of rotary switch positions. */
static uint8_t num_positions;

/** @brief Flag indicating whether a valid raw value has been read. */
static bool raw_valid;

/** * @brief Lookup Table of expected ADC values for each position.
 * * @note Values correspond to Pos 1 through Pos 10.
 * @warning Pos 1 and Pos 2 are identical (4095). Logic will default to Pos 1.
 */
static const uint16_t LUT_POSITIONS[] = {
    4095, // Pos 1 (Index 0)
    //4095, // Pos 2 (Index 1) - Duplicate value!
    3751, // Pos 3 (Index 2)
    3279, // Pos 4 (Index 3)
    2813, // Pos 5 (Index 4)
    2344, // Pos 6 (Index 5)
    1876, // Pos 7 (Index 6)
    1404, // Pos 8 (Index 7)
    938,  // Pos 9 (Index 8)
    470   // Pos 10 (Index 9)
};

/** @brief Total number of defined positions in the LUT. */
#define NUM_DEFINED_POSITIONS (sizeof(LUT_POSITIONS) / sizeof(LUT_POSITIONS[0]))



void rotary_SetCalibration(uint16_t min, uint16_t max)
{
    /* Update calibration boundaries */
    min_raw = min;
    max_raw = max;
}

void rotary_Init(uint8_t positions)
{
    /* Initialize internal variables */
    num_positions = positions;
    raw_valid = false;

    /* Default calibration for full ADC range */
    rotary_SetCalibration(0, 4095);
}

uint16_t rotary_GetRawValue(void)
{
    /* Read raw ADC value from configured channel */
    raw = hal_adc_read(ROTARY_ADC_CHANNEL); 
    
    /* Mark value as valid for later use */
    raw_valid = true;

    return raw;
}

uint8_t rotary_GetPosition(void)
{
    /* 1. Acquire Data */
    if (!raw_valid) { 
        rotary_GetRawValue(); 
    }
    /* Reset validation flag for next cycle */
    raw_valid = false; 

    /* 2. Find the Nearest Neighbor */

    uint8_t best_index = 0;
    uint16_t min_error = 0xFFFF; // Initialize with max possible value

    for (uint8_t i = 0; i < NUM_DEFINED_POSITIONS; i++) {

        // Calculate the absolute difference between read value and expected value
        // We use abs() because the error could be positive or negative
        uint16_t current_error = abs(raw - LUT_POSITIONS[i]);

        // If this position is closer to the raw value than the previous best, update it
        if (current_error < min_error) {
            min_error = current_error;
            best_index = i+1;
        }
    }

    /* * 3. Return the Position
     * Indices are 0-9, but positions are usually referred to as 1-10.
     * We assume the application expects a 0-based index (0 to 9).
     * If you need 1 to 10, change this to: return best_index + 1;
     */
    return best_index;
}
