/**
 * @file rotary_switch.h
 * @brief Driver interface for rotary switch position detection using ADC.
 *
 * @details
 * This module provides an interface to read and process analog input values
 * from a rotary switch. The switch output is connected to an ADC channel and
 * converted into discrete position indices (e.g., 0–7 or 0–15).
 *
 * It supports calibration for minimum and maximum raw ADC values, allowing
 * the driver to operate correctly despite component or voltage variations.
 *
 * @note
 * Before using `rotary_GetPosition()`, call `rotary_Init()` and optionally
 * `rotary_SetCalibration(min, max)` to set the input range.
 */
#ifndef ROTARY_SWITCH_H
#define ROTARY_SWITCH_H

#include <stdint.h>
#include <stdbool.h>

/** @def ROTARY_ADC_CHANNEL
 *  @brief Defines the ADC channel assigned to the rotary switch input.
 *  @note Update this value according to the hardware connection.
 */
#define ROTARY_ADC_CHANNEL 12

/**
 * @brief Initializes the rotary switch driver.
 *
 * @details
 * Sets up internal variables and the number of valid positions for the rotary switch.
 * Also initializes default calibration values (0–4095).
 *
 * @param[in] num_positions Total number of discrete positions for the rotary switch.
 */
void rotary_Init(uint8_t num_positions);

/**
 * @brief Reads the current position of the rotary switch.
 *
 * @details
 * Converts the raw ADC value into a discrete position index within the range
 * [0, num_positions - 1]. The conversion is linear and based on the calibration
 * limits set via `rotary_SetCalibration()`.
 *
 * @return uint8_t Current position index.
 */
uint8_t rotary_GetPosition(void);

/**
 * @brief Reads the raw ADC value corresponding to the rotary switch input.
 *
 * @return uint16_t Raw ADC reading (0–4095 depending on resolution).
 */
uint16_t rotary_GetRawValue(void);

/**
 * @brief Sets calibration limits for the rotary switch input.
 *
 * @details
 * Defines the minimum and maximum raw ADC values corresponding to the lowest
 * and highest rotary positions, respectively.
 *
 * @param[in] min Minimum raw ADC value (position 0).
 * @param[in] max Maximum raw ADC value (last position).
 */ 
void rotary_SetCalibration(uint16_t min, uint16_t max);

#endif
