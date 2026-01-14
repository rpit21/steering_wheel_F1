/**
 * @file hal_uart.h
 * @brief HAL interface for UART (LPUART0) on NXP S32K118.
 */

#ifndef HAL_UART_H_
#define HAL_UART_H_

#include <stdint.h>
#include <stdarg.h>


/**
 * @brief Initializes LPUART0 peripheral.
 *
 * @param baudrate Desired UART baudrate (e.g., 115200)
 */
void HAL_UART_Init(uint32_t baudrate);

/**
 * @brief Sends a single character over UART.
 *
 * @param c Character to send.
 */
void HAL_UART_SendChar(char c);

/**
 * @brief Sends a null-terminated string over UART.
 *
 * @param s Pointer to string.
 */
void HAL_UART_SendString(const char *s);

/**
 * @brief printf-style printing through UART.
 *
 * @param fmt Format string
 * @return Number of characters printed
 */
int HAL_UART_Printf(const char *fmt, ...);

#endif /* HAL_UART_H_ */
