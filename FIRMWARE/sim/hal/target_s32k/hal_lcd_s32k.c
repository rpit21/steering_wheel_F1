/*
#include "hal_display.h"
#include "hal_gpio.h"
#include "hal_spi.h"
#include "hal_delay.h"

// =======================================================
// --- PINES DE CONTROL ---
// =======================================================
// Ajusta estos nombres para que coincidan con tu enumeración en hal_gpio.h
#define PIN_DC   GPIO_TFT_DC
#define PIN_RST  GPIO_TFT_RST
#define PIN_CS   GPIO_TFT_CS
#define PIN_LED  GPIO_LED_STATUS   // opcional, para backlight control

// =======================================================
// --- IMPLEMENTACIÓN DEL HAL ---
// =======================================================
void HAL_Display_Init(void) {
    HAL_GPIO_Init();  // Inicializa GPIOs
    HAL_SPI_Init();   // Inicializa SPI
    HAL_GPIO_Write(PIN_CS, 1); // CS high (inactivo)
    HAL_GPIO_Write(PIN_DC, 1); // DC default high
    HAL_GPIO_Write(PIN_LED, 1); // encender backlight
}

void HAL_Display_Reset(void) {
    HAL_GPIO_Write(PIN_RST, 0);
    HAL_DelayMs(5);
    HAL_GPIO_Write(PIN_RST, 1);
    HAL_DelayMs(120);
}

void HAL_Display_WriteCommand(uint8_t cmd) {
    HAL_GPIO_Write(PIN_DC, 0);   // DC = 0 (comando)
    HAL_GPIO_Write(PIN_CS, 0);   // seleccionar chip
    HAL_SPI_Write(&cmd, 1);
    HAL_GPIO_Write(PIN_CS, 1);   // CS = 1 (fin de transmisión)
}

void HAL_Display_WriteData(uint8_t data) {
    HAL_GPIO_Write(PIN_DC, 1);   // DC = 1 (datos)
    HAL_GPIO_Write(PIN_CS, 0);
    HAL_SPI_Write(&data, 1);
    HAL_GPIO_Write(PIN_CS, 1);
}

void HAL_Display_WriteDataBuffer(const uint8_t* data, uint32_t len) {
    HAL_GPIO_Write(PIN_DC, 1);
    HAL_GPIO_Write(PIN_CS, 0);
    HAL_SPI_Write(data, len);
    HAL_GPIO_Write(PIN_CS, 1);
}

void HAL_Display_On(void) {
    HAL_GPIO_Write(PIN_LED, 1); // encender retroiluminación
}

void HAL_Display_Off(void) {
    HAL_GPIO_Write(PIN_LED, 0); // apagar retroiluminación
}

*/