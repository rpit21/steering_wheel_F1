/**
 * @file hal_gpio.h
 * @brief Hardware Abstraction Layer (HAL) interface for General-Purpose Input/Output (GPIO) control.
 *
 * @details
 * This module defines a platform-independent interface for manipulating digital input
 * and output pins. It provides functions to initialize GPIOs, read and write pin states,
 * and emulate GPIO events in simulation environments.
 *
 * Implementation behavior depends on the build target:
 * - **Hardware (e.g., MCU)**: Direct access to GPIO peripheral registers.
 * - **Host simulation**: Emulates GPIOs using SDL events (keyboard input) and
 *   console output for pin state changes.
 *
 * The abstraction enables drivers and application code to remain portable between
 * embedded and simulated environments.
 *
 * @note
 * All GPIO operations must be initialized via ::HAL_GPIO_Init() before use.
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

// --- INCLUDES ---
#include <stdint.h> /**< Provides fixed-width integer types (e.g., uint8_t). */

/*--------------------------ENUMERATIONS-----------------------------------*/

/**
 * @brief Logical identifiers for GPIO pins managed by the HAL.
 *
 * @details
 * These symbolic identifiers represent logical GPIO roles used across
 * the firmware. The actual hardware mapping is defined in the platform-specific
 * implementation, allowing consistent behavior on both MCU and host systems.
 */
typedef enum {
    GPIO_TFT_CS,   /**< Chip Select for TFT display. */
    GPIO_TFT_DC,   /**< Data/Command select for TFT display. */
    GPIO_TFT_RST,  /**< Reset pin for TFT display. */
    GPIO_LED_S2,   /**< Secondary status LED. */
    GPIO_LED_S1,   /**< Primary status LED. */
    GPIO_BTN_1,    /**< Button 1 input. */
    GPIO_BTN_2,    /**< Button 2 input. */
    GPIO_BTN_3,    /**< Button 3 input. */
    GPIO_BTN_4,    /**< Button 4 input. */
    GPIO_COUNT     /**< Total number of GPIO pins. */
} GPIO_Pin_t;

/*--------------------------PUBLIC API FUNCTIONS-----------------------------------*/

/**
 * @brief Initializes all GPIOs and simulation subsystems.
 *
 * @details
 * Prepares all GPIOs for use by configuring direction, setting default levels,
 * and initializing any platform-specific input or output mechanisms.
 * On host systems, this may initialize SDL to capture keyboard input for GPIO emulation.
 */
void HAL_GPIO_Init(void);

/**
 * @brief Sets the logical output value of a specific GPIO pin.
 *
 * @details
 * Writes a digital HIGH (1) or LOW (0) level to the specified pin.
 * In simulation mode, this may output a message to the console indicating
 * the pin and its new logical state.
 *
 * @param[in] pin   GPIO identifier (see ::GPIO_Pin_t).
 * @param[in] value Logical state to apply: 0 = LOW, 1 = HIGH.
 */
void HAL_GPIO_Write(GPIO_Pin_t pin, uint8_t value);

/**
 * @brief Toggles the current logical output value of the specified GPIO pin.
 *
 * @details
 * Reverses the current logical state (HIGH ↔ LOW) of the given pin.
 * On host systems, this operation may also update console feedback.
 *
 * @param[in] pin GPIO identifier (see ::GPIO_Pin_t).
 */
void HAL_GPIO_Toggle(GPIO_Pin_t pin);

/**
 * @brief Reads the current logical input value of a GPIO pin.
 *
 * @details
 * Returns the digital state of the specified pin.  
 * On hardware, this reads directly from the MCU’s input register.  
 * On host systems, the value is derived from emulated keyboard or input events.
 *
 * @param[in] pin GPIO identifier (see ::GPIO_Pin_t).
 * @return uint8_t Pin state: 0 = LOW, 1 = HIGH.
 */
uint8_t HAL_GPIO_Read(GPIO_Pin_t pin);

/**
 * @brief Handles simulated key events for GPIO emulation.
 *
 * @details
 * This function integrates with the SDL event loop to translate key press
 * and release events into virtual GPIO state changes.  
 * It updates the internal state map used by ::HAL_GPIO_Read() to reflect
 * the simulated input status.
 *
 * @param[in] keysym  SDL key symbol code (e.g., `SDLK_1` for Button 1).
 * @param[in] is_down Set to 1 if the key is pressed, 0 if released.
 */
void HAL_GPIO_on_key(int keysym, int is_down);

#endif /* HAL_GPIO_H */
