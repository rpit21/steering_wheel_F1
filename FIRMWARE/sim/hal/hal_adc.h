/**
 * @file hal_adc.h
 * @brief Hardware Abstraction Layer (HAL) interface for Analog-to-Digital Converter (ADC) operations.
 *
 * @details
 * This module defines a generic ADC interface for both real hardware (e.g., S32K microcontrollers)
 * and host-based simulation environments. It provides initialization and read functions used by
 * higher-level modules (e.g., drivers such as ::clutch.c or ::rotary.c).
 *
 * The abstraction allows upper software layers to perform analog reads without depending
 * on the specific hardware implementation.
 *
 * Typical usage:
 * - Call ::hal_adc_init() once during system initialization.
 * - Call ::hal_adc_read() to acquire raw ADC samples from a specific channel.
 *
 * @note
 * The behavior and resolution of ADC readings depend on the underlying platform:
 * - Real hardware: Reads physical voltage levels on analog input pins.
 * - Simulation: Returns mock or randomized values to emulate sensor inputs.
 */

#ifndef HAL_ADC_H
#define HAL_ADC_H

// --- INCLUDES ---
#include <stdint.h> /**< Provides fixed-width integer types (e.g., uint8_t, uint16_t). */

/*--------------------------PUBLIC API FUNCTIONS-----------------------------------*/

/**
 * @brief Initializes the ADC subsystem (hardware or simulation).
 *
 * @details
 * Prepares the ADC for operation by configuring internal parameters such as
 * reference voltage, resolution, and sampling time.
 * On host-based simulations, this may instead set up file-based or random data sources.
 *
 * Must be called once before performing any ADC reads.
 */
void hal_adc_init(void);

/**
 * @brief Reads a raw analog-to-digital conversion value from a specified ADC channel.
 *
 * @details
 * Initiates a conversion on the selected input channel and returns the corresponding
 * digital representation of the analog input signal.
 *
 * @param[in] channel Index of the ADC channel to read (e.g., 0, 1, 2...).
 *
 * @return uint16_t Raw ADC conversion result.
 * @retval 0–1023  For 10-bit ADCs.
 * @retval 0–4095  For 12-bit ADCs (depends on platform configuration).
 *
 * @note
 * The function may perform blocking reads or use simulated values depending on the
 * HAL backend (hardware vs. host PC).
 */
uint16_t hal_adc_read(uint8_t channel);

#endif /* HAL_ADC_H */
