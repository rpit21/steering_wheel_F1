/**
 * @file hal_display.h
 * @brief Hardware Abstraction Layer (HAL) for TFT display control (e.g., ILI9341).
 *
 * @details
 * This module defines a unified, platform-independent interface for managing
 * graphical display devices. It abstracts hardware-specific details such as
 * SPI communication and GPIO control, allowing the same high-level display
 * driver to run on both embedded targets and host-based simulations.
 *
 * Implementation behavior varies per platform:
 * - **Target (MCU)**: Communicates with the display using SPI and control GPIOs.
 * - **Host (PC)**: Simulates display behavior using SDL for visualization.
 *
 * @note
 * All functions must be initialized through ::HAL_Display_Init() before use.
 */

#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

// --- INCLUDES ---
#include <stdint.h> /**< Provides standard integer types (e.g., uint8_t, uint32_t). */

/*--------------------------PUBLIC API FUNCTIONS-----------------------------------*/

/**
 * @brief Initializes the display subsystem and communication interface.
 *
 * @details
 * Sets up SPI, GPIO control pins, and any platform-specific subsystems required
 * for display communication. On the host system, initializes the SDL window and
 * prepares the framebuffer for rendering.
 */
void HAL_Display_Init(void);

/**
 * @brief Executes a hardware reset sequence for the display.
 *
 * @details
 * Drives the display's RESET pin low and high in the proper sequence to ensure
 * the controller starts from a known state.  
 * On host systems, this may be a no-op or a visual reset of the simulated window.
 */
void HAL_Display_Reset(void);

/**
 * @brief Sends an 8-bit command to the display controller.
 *
 * @details
 * Sets the Data/Command (DC) line LOW to indicate command mode and transmits the
 * specified byte over SPI or equivalent interface.
 *
 * @param[in] cmd Command byte to send.
 */
void HAL_Display_WriteCommand(uint8_t cmd);

/**
 * @brief Sends an 8-bit data value to the display.
 *
 * @details
 * Sets the Data/Command (DC) line HIGH to indicate data mode and transmits the
 * specified byte over SPI or equivalent interface.
 *
 * @param[in] data Data byte to send.
 */
void HAL_Display_WriteData(uint8_t data);

/**
 * @brief Sends a buffer of consecutive data bytes to the display.
 *
 * @details
 * This function efficiently transfers a block of pixel data or configuration bytes
 * over the active interface. It is typically used when updating a region of the display.
 *
 * @param[in] data Pointer to the data buffer to transmit.
 * @param[in] len  Number of bytes to send.
 */
void HAL_Display_WriteDataBuffer(const uint8_t* data, uint32_t len);

/**
 * @brief Turns ON the display or simulated backlight.
 *
 * @details
 * Activates the display’s backlight control (if available) or enables
 * visible output in the simulated environment.
 */
void HAL_Display_On(void);

/**
 * @brief Turns OFF the display or simulated backlight.
 *
 * @details
 * Deactivates the display’s backlight or hides the display window in simulation mode.
 */
void HAL_Display_Off(void);

/*--------------------------HOST-SPECIFIC FUNCTIONS [SIMULATION ONLY]-----------------------------------*/

/**
 * @brief Refreshes the simulated display window.
 *
 * @details
 * Transfers the current framebuffer contents to the SDL window,
 * making visual updates visible in the simulation environment.
 */
void HAL_Display_Present(void);

/**
 * @brief Polls and processes window or keyboard events in simulation mode.
 *
 * @details
 * Handles SDL input events (such as key presses or window closure) to ensure
 * the host simulation remains interactive and responsive.
 *
 * @param[in,out] running Pointer to the main loop control variable.
 *                        Set to 0 when a quit event is detected.
 */
void HAL_Poll_Events(int* running);

#endif /* HAL_DISPLAY_H */
