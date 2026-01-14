/**
 * @file hal_display.h
 * @brief Hardware Abstraction Layer (HAL) interface for display control and rendering.
 *
 * @details
 * This module provides a unified interface for initializing, controlling, and updating
 * display output across both hardware targets and host-based simulations.
 *
 * The abstraction enables upper software layers to interact with a display device
 * (e.g., OLED, LCD, or SDL2 window) without depending on the underlying transport
 * mechanism (SPI, I²C, or graphical simulation API).
 *
 * Typical implementation targets:
 * - **Embedded MCU**: Drives a physical display using SPI or I²C.
 * - **Host (simulation)**: Uses SDL2 or equivalent to emulate display behavior.
 *
 * @note
 * The display HAL must be initialized before calling any drawing or update functions.
 * All framebuffer data is expected to be in a 1-bit-per-pixel (1bpp) format.
 */

#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

// --- INCLUDES ---
#include <stdint.h> /**< Provides fixed-width integer types such as uint8_t and uint32_t. */

/*--------------------------PUBLIC API FUNCTIONS-----------------------------------*/

/**
 * @brief Initializes the display hardware or simulation environment.
 *
 * @details
 * Sets up all resources required for display operation.  
 * On hardware, this configures communication buses (e.g., SPI/I²C) and resets the display.  
 * On the host PC, it creates a window (typically via SDL2) representing the simulated screen.
 *
 * @param[in] width  Horizontal resolution of the display, in pixels.
 * @param[in] height Vertical resolution of the display, in pixels.
 * @param[in] scale  Scaling factor for simulation window size (used only on host systems).
 *
 * @return int
 * @retval 0   Initialization successful.
 * @retval <0  Initialization failed (e.g., resource allocation or device error).
 */
int hal_display_init(int width, int height, int scale);

/**
 * @brief Shuts down the display subsystem and releases allocated resources.
 *
 * @details
 * Performs cleanup of hardware or software resources associated with the display.
 * On hardware targets, this may power down the display or disable communication buses.
 * On host systems, this closes the SDL window and frees graphical resources.
 */
void hal_display_shutdown(void);

/**
 * @brief Transfers a 1-bit-per-pixel framebuffer to the physical or simulated display.
 *
 * @details
 * Updates the visual output based on the provided framebuffer data.
 * Each bit in `fb_bits` corresponds to one pixel (1 = on, 0 = off).
 * This function serves as the link between the driver’s in-memory representation
 * and the actual visual display.
 *
 * @param[in] fb_bits Pointer to the framebuffer memory containing the image data.
 * @param[in] width   Width of the framebuffer in pixels.
 * @param[in] height  Height of the framebuffer in pixels.
 */
void hal_display_present_1bpp(const uint8_t* fb_bits, int width, int height);

/*--------------------------UTILITY FUNCTIONS (HOST SIMULATION)-----------------------------------*/

/**
 * @brief Processes system events for the simulation environment.
 *
 * @details
 * Keeps the simulated display window responsive by handling OS-level events
 * such as window close, keyboard input, or focus changes.
 * If a quit event is detected, the `running` flag is cleared to terminate
 * the main application loop gracefully.
 *
 * @param[in,out] running Pointer to the application’s main loop control variable.
 *                        Set to 0 if a quit event is detected.
 */
void hal_poll_events(int* running);

/**
 * @brief Delays execution for a specified duration (in milliseconds).
 *
 * @details
 * Used primarily in host simulations to regulate frame rate or pacing.
 * On embedded targets, use ::HAL_DelayMs from ::hal_delay.h instead.
 *
 * @param[in] ms Duration of delay in milliseconds.
 */
void hal_delay_ms(int ms);

/**
 * @brief Returns the number of milliseconds since display initialization.
 *
 * @details
 * Provides a simple time base for animations, timers, or periodic updates.
 * On host systems using SDL, this function typically wraps `SDL_GetTicks()`.
 *
 * @return uint32_t Elapsed time in milliseconds since initialization.
 */
uint32_t hal_get_ticks(void);

#endif /* HAL_DISPLAY_H */
