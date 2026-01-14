/**
 * @file hal_adc.c
 * @brief Host PC simulation of the Analog-to-Digital Converter (ADC).
 *
 * @details
 * This file implements the ADC HAL for the host PC simulation.
 * It provides ADC values either by reading them from a CSV file or
 * by generating random numbers if the CSV file is not available.
 * 
 * This allows testing application logic without real hardware.
 */

#define _POSIX_C_SOURCE 200809L

#include "../hal_adc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

/*--------------------------DEFINES-----------------------------------*/
#define MAX_CHANNELS 3           /**< Number of ADC channels simulated */
#define ADC_UPDATE_PERIOD_MS 1000 /**< Minimum time between CSV updates */

/*--------------------------PRIVATE VARIABLES-----------------------------------*/
static FILE *adc_file = NULL;                   /**< File pointer for CSV simulation data */
static uint8_t initialized = 0;                /**< Flag to indicate initialization */
static uint16_t last_values[MAX_CHANNELS] = {0}; /**< Last read values for each channel */
static uint64_t last_update_time_ms = 0;       /**< Timestamp of the last CSV read */

/*--------------------------PRIVATE HELPER FUNCTIONS-----------------------------------*/
/**
 * @brief Returns current time in milliseconds.
 *
 * @details
 * Used to determine when to update ADC values from the CSV file.
 */
static uint64_t get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL);
}

/*--------------------------PUBLIC API FUNCTIONS-----------------------------------*/
/**
 * @brief Initializes the ADC HAL for simulation.
 *
 * @details
 * Opens the CSV file for simulation data, or falls back to random
 * values if the file is not found. Should be called before
 * any calls to `hal_adc_read`.
 */
void hal_adc_init(void) {
    if (initialized) return;

    adc_file = fopen("test/adc_data.csv", "r");
    if (adc_file)
        printf("[Sim ADC] Reading from adc_data.csv\n");
    else
        printf("[Sim ADC] File not found, using random values\n");

    initialized = 1;
    last_update_time_ms = get_time_ms();
}

/**
 * @brief Reads the simulated ADC value for a specific channel.
 *
 * @param[in] channel ADC channel index (0 to MAX_CHANNELS-1)
 * @return uint16_t Last read or generated ADC value
 *
 * @details
 * Updates values from the CSV if enough time has passed.
 * If the CSV is not available, generates pseudo-random values.
 */
uint16_t hal_adc_read(uint8_t channel) {
    if (!initialized) hal_adc_init();
    if (channel >= MAX_CHANNELS) return 0;

    uint64_t now = get_time_ms();

    // Update values from CSV if enough time has passed
    if (adc_file && (now - last_update_time_ms >= ADC_UPDATE_PERIOD_MS)) {
        char line[128];
        if (fgets(line, sizeof(line), adc_file)) {
            char *token = strtok(line, ",");
            for (uint8_t i = 0; i < MAX_CHANNELS && token; i++) {
                last_values[i] = (uint16_t)atoi(token);
                token = strtok(NULL, ",");
            }
        } else {
            rewind(adc_file);
            return hal_adc_read(channel);
        }
        last_update_time_ms = now;
    }
    // Fallback: generate random value if no CSV
    else if (!adc_file) {
        last_values[channel] = rand() % 4096;
    }

    return last_values[channel];
}

/**
 * @brief Shuts down the ADC HAL simulation.
 *
 * @details
 * Closes the CSV file if opened and resets internal state.
 */
void hal_adc_shutdown(void) {
    if (adc_file) {
        fclose(adc_file);
        adc_file = NULL;
    }
    initialized = 0;
}
