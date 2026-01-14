/**
 * @file hal_spi.h
 * @brief Hardware Abstraction Layer (HAL) for Serial Peripheral Interface (SPI) communication.
 *
 * @details
 * This module defines a platform-independent interface for SPI-based data transfer.
 * It abstracts away the hardware-specific details of configuring and communicating
 * through the SPI peripheral, providing a consistent API for both embedded targets
 * and host-based simulation environments.
 *
 * Implementation behavior varies by platform:
 * - **Target (MCU)**: Configures and drives the hardware SPI peripheral for high-speed
 *   synchronous serial communication with peripherals (e.g., displays, sensors).
 * - **Host (PC Simulation)**: Emulates SPI communication by logging transmitted data
 *   to the console for debugging and visualization purposes.
 *
 * @note
 * Before performing any SPI transaction, the interface must be initialized
 * through ::HAL_SPI_Init().
 */

#ifndef HAL_SPI_H
#define HAL_SPI_H

// --- INCLUDES ---
#include <stdint.h>  /**< Provides fixed-width integer types (uint8_t, uint32_t). */
#include <stddef.h>  /**< Provides size_t type for data length parameters. */

/*--------------------------PUBLIC API FUNCTIONS-----------------------------------*/

/**
 * @brief Initializes the SPI communication subsystem.
 *
 * @details
 * Configures the SPI peripheral (clock polarity, phase, bit order, and baud rate)
 * or sets up a simulated communication channel in the host environment.  
 * On the host system, this function typically outputs an initialization message
 * to the console for debugging purposes.
 */
void HAL_SPI_Init(void);

/**
 * @brief Sends a single command byte over SPI.
 *
 * @details
 * Transmits one byte with the display or peripheral’s Data/Command (DC) line
 * held LOW to indicate command mode.  
 * Typically used for register configuration or control commands.
 *
 * @param[in] cmd Command byte to transmit.
 */
void HAL_SPI_WriteCommand(uint8_t cmd);

/**
 * @brief Sends a continuous block of data bytes over SPI.
 *
 * @details
 * Used to transfer large data buffers such as display pixel data or configuration tables.  
 * On the host simulation, only the first few bytes are printed to the console
 * to aid debugging while avoiding excessive output.
 *
 * @param[in] data Pointer to the data buffer to be transmitted.
 * @param[in] length Number of bytes to send from the buffer.
 */
void HAL_SPI_WriteData(const uint8_t* data, size_t length);

/**
 * @brief Transmits a single byte via SPI.
 *
 * @details
 * Simplified variant of ::HAL_SPI_WriteData used for quick tests or sending
 * isolated bytes (e.g., dummy transfers for synchronization or SPI clocking).
 *
 * @param[in] byte Single byte to transmit.
 */
void HAL_SPI_TransmitByte(uint8_t byte);

#endif /* HAL_SPI_H */
