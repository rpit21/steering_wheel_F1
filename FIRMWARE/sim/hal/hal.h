/**
 * @file hal.h
 * @brief Generic Hardware Abstraction Layer (HAL) interface for platform-independent hardware access.
 *
 * @details
 * This module defines a unified abstraction layer for interacting with various
 * hardware peripherals, including GPIO, ADC, SPI (for display communication), and CAN.
 * 
 * It serves as a high-level interface for drivers and application code, allowing
 * the same firmware to operate seamlessly across multiple environments:
 * - **Target MCU**: Interacts with real hardware peripherals.
 * - **Host (PC Simulation)**: Emulates peripheral behavior via console I/O or SDL.
 *
 * @note
 * This generic header may overlap with more specific peripheral HALs such as
 * ::hal_gpio.h, ::hal_adc.h, or ::hal_spi.h.  
 * It is typically used when a unified interface is required across different hardware layers.
 */

#ifndef HAL_H
#define HAL_H

// --- INCLUDES ---
#include <stdint.h>  /**< Provides fixed-width integer types for consistent cross-platform behavior. */

/*--------------------------GPIO FUNCTIONS-----------------------------------*/

/**
 * @brief Reads the logical state of a button or digital input pin.
 *
 * @details
 * Retrieves the current state of the specified button input.
 * Implementation depends on platform:
 * - On MCU: Reads directly from the GPIO input register.
 * - On host: Reads from emulated input (e.g., SDL key events).
 *
 * @param[in] id_button Identifier of the button to read.
 * @return int Button state (0 = released, 1 = pressed).
 */
int HAL_ButtonRead(int id_button);

/**
 * @brief Reads the current position or state of a rotary switch.
 *
 * @details
 * Returns a code representing the current rotary switch position or mode.
 * The exact meaning of the return value depends on the hardware configuration
 * or simulated input mapping.
 *
 * @return int Rotary switch state or position value.
 */
int HAL_RotarySwitch(void);

/**
 * @brief Sets the output level of a digital pin (e.g., controlling an LED).
 *
 * @details
 * Writes a logical high or low value to a specified output pin.  
 * On host systems, this may print a debug message to the console instead of
 * interacting with actual hardware.
 *
 * @param[in] id_pin Identifier of the output pin.
 * @param[in] val Output value (0 = LOW/OFF, 1 = HIGH/ON).
 * @return int Status code (0 = success, <0 = error).
 */
int HAL_DigitalWrite(int id_pin, int val);


/*--------------------------ADC FUNCTIONS-----------------------------------*/

/**
 * @brief Reads the analog value from a specified ADC input channel.
 *
 * @details
 * Performs an analog-to-digital conversion and returns the measured value.  
 * The return type `float` indicates that the HAL may perform scaling to a
 * physical unit (e.g., volts) rather than returning raw ADC counts.
 *
 * @param[in] id_pin Identifier of the analog input channel.
 * @return float Converted analog value.
 */
float HAL_ADCRead(int id_pin);


/*--------------------------SPI / DISPLAY FUNCTIONS-----------------------------------*/

/**
 * @brief Sends a single byte of data over the SPI interface.
 *
 * @details
 * Used for low-level SPI transfers, typically when communicating with displays
 * or serial peripherals.  
 * On MCU targets, this performs an actual SPI transmission.  
 * On host systems, the sent byte may be printed to the console for debugging.
 *
 * @param[in] ms Data byte to transmit.
 */
void HAL_SPITx(char ms);


/*--------------------------CAN FUNCTIONS-----------------------------------*/

/**
 * @brief Receives data from the CAN bus.
 *
 * @details
 * Retrieves a message from the CAN interface, potentially filtered by message ID.
 * The behavior and meaning of the return value depend on the specific HAL implementation.
 *
 * @param[in] id Identifier for message filtering or receive buffer selection.
 * @return int CAN receive result (e.g., data, status code, or number of bytes).
 */
int HAL_CANRx(int id);

#endif /* HAL_H */
