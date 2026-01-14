/**
 * @file hal_uart.c
 * @brief HAL implementation for UART (LPUART0) on S32K118.
 */

#include "hal_uart.h"
#include "device_registers.h"
#include <stdio.h>
#include <stdarg.h>

/* UART = LPUART0 mapped to PTA2 (RX) and PTA3 (TX)
 *
 * PTA3 - ALT6 -> LPUART0_TX
 * PTA2 - ALT6 -> LPUART0_RX
 */

#define UART_PORT       IP_PORTA
#define UART_TX_PIN     3   /* PTA3 */
#define UART_RX_PIN     2   /* PTA2 */


/**
 * @brief Configure pin muxing for UART TX/RX.
 */
static void UART_ConfigPins(void)
{
    /* Enable PORTA clock */
    IP_PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK;

    /* TX pin: PTA3 -> ALT6 */
    UART_PORT->PCR[UART_TX_PIN] &= ~PORT_PCR_MUX_MASK;
    UART_PORT->PCR[UART_TX_PIN] |= PORT_PCR_MUX(6);

    /* RX pin: PTA2 -> ALT6 */
    UART_PORT->PCR[UART_RX_PIN] &= ~PORT_PCR_MUX_MASK;
    UART_PORT->PCR[UART_RX_PIN] |= PORT_PCR_MUX(6);
}


/**
 * @brief Initializes LPUART0 peripheral.
 * 8 bits - 0 parity - 1 stop bit
 */
void HAL_UART_Init(uint32_t baudrate)
{
    uint32_t baud_div;

    UART_ConfigPins();

    /* Enable clock for LPUART0 */
    IP_PCC->PCCn[PCC_LPUART0_INDEX] &= ~PCC_PCCn_CGC_MASK;    	/* Ensure clk disabled for config */
    IP_PCC->PCCn[PCC_LPUART0_INDEX] |= PCC_PCCn_PCS(3) 			/* Clock Src= 3 (FIRCDIV2_CLK) 48MHz */
                                	|  PCC_PCCn_CGC_MASK;     	/* Enable clock for LPUART1 regs */

    /* Disable LPUART during configuration */
    IP_LPUART0->CTRL &= ~LPUART_CTRL_TE_MASK;
    IP_LPUART0->CTRL &= ~LPUART_CTRL_RE_MASK;

    /* LPUART clock = 48 MHz (FIRCDIV clock) */
    uint32_t uart_clk = 48000000u;

    /* Baud rate formula: SBR = uart_clk / (16 * baudrate) */
    baud_div = uart_clk / (16 * baudrate);

    IP_LPUART0->BAUD &= ~LPUART_BAUD_SBR_MASK;
    IP_LPUART0->BAUD |= LPUART_BAUD_SBR(baud_div);

    /* Enable TX and RX */
    IP_LPUART0->CTRL |= LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK;

    HAL_UART_Printf("[UART] Init @%lu baud: PCC=0x%08lX BAUD=0x%08lX CTRL=0x%08lX\r\n",
                        baudrate,
                        (uint32_t)IP_PCC->PCCn[PCC_LPUART0_INDEX],
                        (uint32_t)IP_LPUART0->BAUD,
                        (uint32_t)IP_LPUART0->CTRL);
}

/**
 * @brief Sends a single character over UART.
 */
void HAL_UART_SendChar(char c)
{
    /* Wait for TX buffer empty */
    while(!(IP_LPUART0->STAT & LPUART_STAT_TDRE_MASK));

    IP_LPUART0->DATA = (uint8_t)c;
}


/**
 * @brief Sends a string over UART.
 */
void HAL_UART_SendString(const char *s)
{
    while (*s)
    {
        HAL_UART_SendChar(*s++);
    }
}

/**
 * @brief printf-style printing to UART using vsnprintf.
 */
int HAL_UART_Printf(const char *fmt, ...)
{
    char buffer[256];

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    HAL_UART_SendString(buffer);
    return len;
}
