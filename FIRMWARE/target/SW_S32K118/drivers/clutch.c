/**
 * @file clutch.c
 * @brief Implementation of the clutch pedal position driver.
 *
 * @details
 * This module handles clutch pedal input through the ADC peripheral.
 * It provides functionality for:
 * - Reading the raw ADC value from the assigned channel.
 * - Calibrating min/max raw limits.
 * - Converting readings into a percentage (0–100%) using linear scaling.
 *
 * The driver is designed to work with the HAL ADC interface, ensuring
 * hardware abstraction between simulation and target MCU.
 */

#include "../hal/hal_adc.h"
#include "clutch.h"
#include <stdbool.h>

/** @brief Last raw ADC reading from clutch input. */
static uint16_t raw;

/** @brief Flag indicating whether the raw ADC value is valid. */
static bool raw_valid;

/** @brief Minimum calibrated raw ADC value (0% clutch). */
static uint16_t min_raw;

/** @brief Maximum calibrated raw ADC value (100% clutch). */
static uint16_t max_raw;


void clutch_SetCalibration(uint16_t min, uint16_t max){

    //* Update calibration boundaries */
    min_raw=min;
    max_raw=max; 
}

void clutch_Init(void)
{
    /* Initialize internal state variables */
    raw=0;
    raw_valid=false;

    /* Default calibration range (full ADC scale) */ 
    clutch_SetCalibration(0, 4095);
}

float clutch_GetPercentage(void)
{
    /* If no valid raw reading is available, perform a new one */
    if (!raw_valid){ 
        clutch_GetRawValue(); //Perform ADC read once
    }

    /* Reset flag for next read */
    raw_valid=false;

     /* Avoid division by zero */
    if (max_raw == min_raw) return 0.0f;

    /* Linear mapping: percent = m*raw - m*min_raw, m -> Slope of the line */
    float percent = (raw-min_raw)* (100.0f / (max_raw-min_raw)); 

    /* Clamp result between 0% and 100% */
    if (percent > 100.0f) percent = 100.0f; 
    if (percent < 0.0f) percent = 0.0f; 
    

    return percent; // return porcentage
}

uint16_t clutch_GetRawValue(void) {

    /* Read raw ADC value from the configured clutch channel */
    raw = hal_adc_read(CLUTCH_ADC_CHANNEL); 

    /* Mark data as valid */
    raw_valid = true; //set the flag

    return raw;
}