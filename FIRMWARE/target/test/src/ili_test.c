/*
 * ili_test.c
 *
 *  Created on: Dec 14, 2025
 *      Author: Ruben
 */

#include "hal_gpio.h"
#include "hal_delay.h"
#include "hal_spi.h"
#include "hal_uart.h"

#include "ili9341.h"

#include <stdint.h>
#include <stdbool.h>
#include <math.h>       /* for fabsf */
#include <string.h>
#include <stdio.h>      /* optional: debug prints */

#include "device_registers.h"

static void TFT_SetWindow(uint16_t x0, uint16_t y0,
                          uint16_t x1, uint16_t y1)
{
    // Column address
    HAL_GPIO_Write(GPIO_TFT_DC, 0);
    HAL_SPI_TransmitByte(0x2A);
    HAL_GPIO_Write(GPIO_TFT_DC, 1);
    HAL_SPI_TransmitByte(x0 >> 8);
    HAL_SPI_TransmitByte(x0 & 0xFF);
    HAL_SPI_TransmitByte(x1 >> 8);
    HAL_SPI_TransmitByte(x1 & 0xFF);

    // Row address
    HAL_GPIO_Write(GPIO_TFT_DC, 0);
    HAL_SPI_TransmitByte(0x2B);
    HAL_GPIO_Write(GPIO_TFT_DC, 1);
    HAL_SPI_TransmitByte(y0 >> 8);
    HAL_SPI_TransmitByte(y0 & 0xFF);
    HAL_SPI_TransmitByte(y1 >> 8);
    HAL_SPI_TransmitByte(y1 & 0xFF);

    // RAM write
    HAL_GPIO_Write(GPIO_TFT_DC, 0);
    HAL_SPI_TransmitByte(0x2C);
}

static void TFT_TestFillRed(void)
{
    uint32_t i;

    // Set window 0..239 x 0..319
    HAL_GPIO_Write(GPIO_TFT_DC, 0);
    HAL_SPI_TransmitByte(0x2A);
    HAL_GPIO_Write(GPIO_TFT_DC, 1);
    HAL_SPI_TransmitByte(0); HAL_SPI_TransmitByte(0);
    HAL_SPI_TransmitByte(0); HAL_SPI_TransmitByte(239);

    HAL_GPIO_Write(GPIO_TFT_DC, 0);
    HAL_SPI_TransmitByte(0x2B);
    HAL_GPIO_Write(GPIO_TFT_DC, 1);
    HAL_SPI_TransmitByte(0); HAL_SPI_TransmitByte(0);
    HAL_SPI_TransmitByte(1); HAL_SPI_TransmitByte(63); // 319

    HAL_GPIO_Write(GPIO_TFT_DC, 0);
    HAL_SPI_TransmitByte(0x2C);
    HAL_GPIO_Write(GPIO_TFT_DC, 1);

    // Enviar MUCHOS píxeles sin delays
    for (i = 0; i < (240 * 320); i++) {
        IP_LPSPI0->TDR = 0xF8;
        while(!(IP_LPSPI0->SR & LPSPI_SR_TDF_MASK));
        IP_LPSPI0->TDR = 0x00;
        while(!(IP_LPSPI0->SR & LPSPI_SR_TDF_MASK));
    }
}

static void TFT_WriteColorStream(uint16_t color, uint32_t count)
{
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    uint8_t i;

    HAL_GPIO_Write(GPIO_TFT_CS, 0);
    HAL_GPIO_Write(GPIO_TFT_DC, 0);
    HAL_SPI_TransmitByte(0x2C);
    HAL_GPIO_Write(GPIO_TFT_DC, 1);

    for (i = 0; i < 240*320; i++) {
        while (!(IP_LPSPI0->SR & LPSPI_SR_TDF_MASK));
        IP_LPSPI0->TDR = 0x00;
        while (!(IP_LPSPI0->SR & LPSPI_SR_TDF_MASK));
        IP_LPSPI0->TDR = 0x1F; // BLUE
    }

    while (IP_LPSPI0->SR & LPSPI_SR_MBF_MASK);
    HAL_GPIO_Write(GPIO_TFT_CS, 1);

}





static void TFT_DrawRedLine(void)
{
    uint16_t x;

    TFT_SetWindow(0, 0, 239, 0);   // línea horizontal arriba

    HAL_GPIO_Write(GPIO_TFT_DC, 1);

    for (x = 0; x < 240; x++) {
        HAL_SPI_TransmitByte(0xF8); // RED high
        HAL_SPI_TransmitByte(0x00); // RED low
    }
}


static void TFT_DrawOnePixel(void)
{
	// Column 0..0
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x2A);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0x00);

	// Row 0..0
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x2B);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0x00);

	// Write RAM
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x2C);
	HAL_DelayUs(2);

	// Pixel RED
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0xF8);
	HAL_SPI_TransmitByte(0x00);

}

int ili_main(void) {
	HAL_GPIO_Init();

	HAL_SPI_Init();
	//LPSPI0_init_master ();

	/* ================= ILI9341 INIT (WORKING SEQUENCE) ================= */

	HAL_GPIO_Write(GPIO_TFT_CS, 0);

	//Reset
	HAL_GPIO_Write(GPIO_TFT_RST, 0);
	HAL_DelayMs(10);
	HAL_GPIO_Write(GPIO_TFT_RST, 1);
	HAL_DelayMs(120);



	/* SOFTWARE RESET */
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x01);
	HAL_DelayMs(120);


	//POWER CONTROL A
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xCB);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x39);
	HAL_SPI_TransmitByte(0x2C);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0x34);
	HAL_SPI_TransmitByte(0x02);

	//POWER CONTROL B
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xCF);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0xC1);
	HAL_SPI_TransmitByte(0x30);

	//DRIVER TIMING CONTROL A
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xE8);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x85);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0x78);

	//DRIVER TIMING CONTROL B
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xEA);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0x00);




	//POWER ON SEQUENCE CONTROL
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xED);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x64);
	HAL_SPI_TransmitByte(0x03);
	HAL_SPI_TransmitByte(0x12);
	HAL_SPI_TransmitByte(0x81);

	// PUMP RATIO CONTROL
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xF7);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x20);

	// POWER CONTROL VRH
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xC0);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x23);

	// POWER CONTROL SAP/BT
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xC1);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x10);


/*
	// VCM CONTROL
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xC5);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x3E);
	HAL_SPI_TransmitByte(0x28);



	// VCM CONTROL 2
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xC7);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x86);

	*/


	/* MEMORY ACCESS CONTROL (MADCTL) */
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x36);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x00);//0x48
	HAL_DelayMs(10);

	/* PIXEL FORMAT */
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x3A);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x55);
	HAL_DelayMs(10);

	/* FRAME RATE CONTROL */
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xB1);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x00);
	HAL_SPI_TransmitByte(0x18);
	HAL_DelayMs(10);

	/* DISPLAY FUNCTION CONTROL */
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xB6);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x08);
	HAL_SPI_TransmitByte(0x82);
	HAL_SPI_TransmitByte(0x27);


	//3GAMMA DISABLE
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xF2);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x00);

	//GAMMA CURVE SELECT
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x26);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(0x01);


	// POSITIVE GAMMA
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xE0);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	uint8_t pos_gamma[] = {
	    0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,
	    0x37,0x07,0x10,0x03,0x0E,0x09,0x00
	};
	for(int i=0;i<15;i++) HAL_SPI_TransmitByte(pos_gamma[i]);

	// NEGATIVE GAMMA
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0xE1);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	uint8_t neg_gamma[] = {
	    0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,
	    0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F
	};
	for(int i=0;i<15;i++) HAL_SPI_TransmitByte(neg_gamma[i]);

	/* EXIT SLEEP */
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x11);
	HAL_DelayMs(120);

	/* DISPLAY ON */
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x29);
	HAL_DelayMs(20);

	/* ================= END INIT ================= */
	// Invert ON
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x21);
	HAL_DelayMs(500);

	// Invert OFF
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(0x20);
	HAL_DelayMs(500);

	//TFT_DrawRedBlock();
	//TFT_TestFillRed();
	// FILL RED
	TFT_WriteColorStream(0xF800, 240 * 320);




	//TFT_DrawOnePixel();

	//HAL_GPIO_Write(GPIO_TFT_DC, 0);
	//HAL_SPI_TransmitByte(0x2C);

	//HAL_GPIO_Write(GPIO_TFT_DC, 1);

	/*
	// escribe 100 píxeles rojos
	for (int i = 0; i < 100; i++) {
	    HAL_SPI_TransmitByte(0xF8);
	    HAL_SPI_TransmitByte(0x00);
	}
	*/

	// ==== TEST DRAW ====
	//TFT_DrawRedLine();
	//TFT_DrawRedBlock();



	//TFT_DrawOnePixel();



/*
	uint8_t test = 0xAA;

	HAL_GPIO_Write(GPIO_TFT_CS, 0);
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(test);
	HAL_SPI_TransmitByte(0x55);

	HAL_DelayUs(1);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(test);
	HAL_SPI_TransmitByte(0x55);

	HAL_DelayUs(1);
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(test);
	HAL_SPI_TransmitByte(0x55);

	HAL_DelayUs(1);
	HAL_GPIO_Write(GPIO_TFT_DC, 1);
	HAL_SPI_TransmitByte(test);
	HAL_SPI_TransmitByte(0x55);

	HAL_DelayUs(1);
	HAL_GPIO_Write(GPIO_TFT_DC, 0);
	HAL_SPI_TransmitByte(test);
	HAL_SPI_TransmitByte(0x55);
	HAL_DelayUs(1);

	HAL_GPIO_Write(GPIO_TFT_CS, 1);
	*/


	//ILI9341_Init();


/*

	   // Check fonts
	ILI9341_FillScreen(ILI9341_BLACK);
	ILI9341_WriteString(0, 0, "Font_7x10, HELLO", Font_7x10, ILI9341_RED, ILI9341_BLACK);
	ILI9341_WriteString(0, 3*10, "Font_11x18, HELLO", Font_11x18, ILI9341_GREEN, ILI9341_BLACK);
	ILI9341_WriteString(0, 3*10+3*18, "Font_16x26,HELLO", Font_16x26, ILI9341_BLUE, ILI9341_BLACK);
	HAL_DelayMs(500);


	// Check colors
	ILI9341_FillScreen(ILI9341_WHITE);
	ILI9341_WriteString(0, 0, "WHITE", Font_11x18, ILI9341_BLACK, ILI9341_WHITE);
	HAL_DelayMs(500);

	ILI9341_FillScreen(ILI9341_BLUE);
	ILI9341_WriteString(0, 0, "BLUE", Font_11x18, ILI9341_BLACK, ILI9341_BLUE);
	HAL_DelayMs(500);

	ILI9341_FillScreen(ILI9341_RED);
	ILI9341_WriteString(0, 0, "RED", Font_11x18, ILI9341_BLACK, ILI9341_RED);
	HAL_DelayMs(500); */

	/*

	HAL_GPIO_Write(GPIO_TFT_CS, 0);
	for (;;){


		HAL_SPI_TransmitByte(0xAA);
		HAL_SPI_TransmitByte(0x55);

		HAL_UART_Printf("TR\r\n");
		HAL_DelayMs(1);
	}

	*/




    return 0;

}




