/**
 * @file clutch.h
 * @brief Driver interface for clutch pedal position sensing.
 *
 * @details
 * This module provides functions (API) to initialize, calibrate, and read
 * the clutch pedal position using an analog-to-digital converter (ADC).
 *
 * The raw ADC readings are linearly converted into a percentage (0–100%)
 * based on calibration limits set by the user. The driver supports both
 * raw and processed (percentage) data retrieval.
 *
 * @note
 * Before using `clutch_GetPercentage()`, the driver must be initialized
 * with `clutch_Init()` and optionally calibrated with
 * `clutch_SetCalibration(min, max)`.
 */

#ifndef CLUTCH_H
#define CLUTCH_H

/** @def CLUTCH_ADC_CHANNEL
 *  @brief Defines the ADC channel used for clutch pedal input.
 *  @note Change this value according to the actual hardware connection.
 */
#define CLUTCH_ADC_CHANNEL 13


/**
 * @brief Initializes the clutch driver.
 *
 * @details
 * Sets default internal variables and calibration values.
 * This function must be called once before reading clutch data.
 */
void clutch_Init(void);

/**
 * @brief Sets calibration limits for clutch pedal readings.
 *
 * @param[in] min Minimum raw ADC value corresponding to 0% clutch.
 * @param[in] max Maximum raw ADC value corresponding to 100% clutch.
 */
void clutch_SetCalibration(uint16_t min, uint16_t max);

/**
 * @brief Gets the current clutch position as a percentage (0–100%).
 *
 * @details
 * Reads the ADC channel (if not already updated), applies calibration,
 * and returns a linearly scaled value between 0 and 100.
 *
 * @return float Clutch position in percentage.
 */
float clutch_GetPercentage(void);

/**
 * @brief Reads the raw ADC value from the clutch input channel.
 *
 * @return uint16_t Raw ADC reading (0–4095 depending on resolution).
 */
uint16_t clutch_GetRawValue(void);

#endif /* CLUTCH_H */
