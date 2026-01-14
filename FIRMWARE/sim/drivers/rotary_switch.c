
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
    /* Perform ADC read if no valid value available */
    if (!raw_valid) { 
        rotary_GetRawValue(); 
    }

    /* Reset validation flag for next cycle */
    raw_valid = false; 

    /* Prevent invalid calibration */
    if (max_raw <= min_raw) return 0;

    /* Calculate the range of ADC values for each discrete position */
    float step_size = (float)(max_raw - min_raw) / num_positions;

    /* Compute position index based on raw ADC input */
    int position = (int)((raw - min_raw) / step_size);

    /* Clamp position within valid limits */
    if (position < 0) position = 0;
    if (position >= num_positions) position = num_positions - 1;

    return (uint8_t)position;
}