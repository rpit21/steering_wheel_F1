/**
 * @file hal_spi_host.c
 * @brief Host (PC) simulation of SPI interface for TFT display.
 *
 * @details
 * This module simulates SPI communication for testing on a PC.
 * - No real SPI transactions are performed.
 * - All operations are logged to the console.
 * - Commands and data are forwarded to the display simulation.
 *
 * Typical usage:
 * - HAL_SPI_WriteCommand() → prints command byte
 * - HAL_SPI_WriteData()    → prints data buffer (truncated for display)
 * - HAL_SPI_TransmitByte() → prints single byte, forwards to display
 */

#include "hal_spi.h"
#include <stdio.h>
#include "hal_lcd.h"
#include "hal_gpio.h"

/*-------------------------- PUBLIC API FUNCTIONS --------------------------*/

/**
 * @brief Initializes the SPI interface simulation.
 */
void HAL_SPI_Init(void) {
    printf("[HAL_SPI_HOST] SPI initialized (simulation mode)\n");
}

/**
 * @brief Sends a command byte to the display (simulated).
 *
 * @param cmd Command byte to send.
 */
void HAL_SPI_WriteCommand(uint8_t cmd) {
    printf("[HAL_SPI_HOST] CMD -> 0x%02X\n", cmd);
    HAL_Display_WriteCommand(cmd); // Forward command to display simulation
}

/**
 * @brief Sends a buffer of data to the display (simulated).
 *
 * @param data Pointer to data buffer.
 * @param length Number of bytes to send.
 */
void HAL_SPI_WriteData(const uint8_t *data, size_t length) {
    if (!data || length == 0) return;

    uint8_t dc = HAL_GPIO_Read(GPIO_TFT_DC); // DC pin: 1=data, 0=command

    if (dc) {
        // Data mode: write to framebuffer and log
        HAL_Display_WriteDataBuffer(data, length);
        //printf("[HAL_SPI_HOST] DATA (%zu bytes): ", length);
        //for (size_t i = 0; i < length && i < 8; i++) printf("%02X ", data[i]);
        //if (length > 8) printf("...");
        //printf("\n");
    } else {
        // Command mode: only first byte is used
        HAL_Display_WriteCommand(data[0]);
        //printf("[HAL_SPI_HOST] CMD (from data) -> 0x%02X\n", data[0]);
    }
}

/**
 * @brief Transmits a single byte over SPI (simulated).
 *
 * @param byte Byte to transmit.
 */
void HAL_SPI_TransmitByte(uint8_t byte) {
    uint8_t dc = HAL_GPIO_Read(GPIO_TFT_DC);

    if (dc) {
        HAL_Display_WriteData(byte);
        printf("[HAL_SPI_HOST] BYTE(DATA) -> 0x%02X\n", byte);
    } else {
        HAL_Display_WriteCommand(byte);
        printf("[HAL_SPI_HOST] BYTE(CMD) -> 0x%02X\n", byte);
    }
}
