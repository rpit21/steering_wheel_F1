/*
 * tftnxp.c
 *
 *  Created on: Dec 14, 2025
 *      Author: Ruben
 */
#include "hal_gpio.h"
#include "hal_delay.h"
#include "hal_spi.h"
#include "hal_uart.h"

#include "TFT_LCD.h"

#include <stdint.h>
#include <stdbool.h>
#include <math.h>       /* for fabsf */
#include <string.h>
#include <stdio.h>      /* optional: debug prints */

#include "device_registers.h"

int tftnxp_main(void)
{
	HAL_GPIO_Init();

	HAL_SPI_Init();

	/*Test for delay and reset*/
	HAL_GPIO_Write(GPIO_TFT_RST, 0);
	HAL_DelayMs(15);
	HAL_GPIO_Write(GPIO_TFT_RST, 1);
	HAL_DelayMs(120);

	LCD_display9341_init();					/* Initialize LCD Display */

    LCD_draw_string(80, 10, "Hello World!", WHITE, BLACK, 3);
    LCD_fill_triangle(10, 10, 10, 100, 50, 50, MAGENTA);
    LCD_draw_circle(270, 100, 25, CYAN);
    LCD_draw_square(20, 120, 100, RED);
    LCD_fill_rectangle(150, 100, 15, 80, YELLOW);
    LCD_draw_line(0, 0, TFT_HEIGHT, TFT_WIDTH, GREEN);

    return 0;

}
