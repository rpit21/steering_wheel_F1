#include "hal_gpio.h"
#include "hal_spi.h"
#include "hal_delay.h"
#include "hal_lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef HAL_DISPLAY_HOST
#include <SDL2/SDL.h>
#endif

// Colores RGB565
#define COLOR_RED    0xF800
#define COLOR_GREEN  0x07E0
#define COLOR_BLUE   0x001F
#define COLOR_YELLOW 0xFFE0
#define COLOR_WHITE  0xFFFF
#define COLOR_BLACK  0x0000
#define COLOR_CYAN   0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_GRAY   0x8410

#define WIDTH  320
#define HEIGHT 240

// Pequeña utilidad para escribir un pixel directo
static void draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    HAL_Display_WriteCommand(0x2A);
    HAL_Display_WriteData(x >> 8);
    HAL_Display_WriteData(x & 0xFF);
    HAL_Display_WriteData(x >> 8);
    HAL_Display_WriteData(x & 0xFF);

    HAL_Display_WriteCommand(0x2B);
    HAL_Display_WriteData(y >> 8);
    HAL_Display_WriteData(y & 0xFF);
    HAL_Display_WriteData(y >> 8);
    HAL_Display_WriteData(y & 0xFF);

    HAL_Display_WriteCommand(0x2C);
    HAL_Display_WriteData(color >> 8);
    HAL_Display_WriteData(color & 0xFF);
}

// Dibuja rectángulo sólido
static void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    HAL_Display_WriteCommand(0x2A);
    HAL_Display_WriteData(x >> 8); HAL_Display_WriteData(x & 0xFF);
    HAL_Display_WriteData((x + w - 1) >> 8); HAL_Display_WriteData((x + w - 1) & 0xFF);

    HAL_Display_WriteCommand(0x2B);
    HAL_Display_WriteData(y >> 8); HAL_Display_WriteData(y & 0xFF);
    HAL_Display_WriteData((y + h - 1) >> 8); HAL_Display_WriteData((y + h - 1) & 0xFF);

    HAL_Display_WriteCommand(0x2C);

    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        HAL_Display_WriteData(color >> 8);
        HAL_Display_WriteData(color & 0xFF);
    }

//#ifdef HAL_DISPLAY_HOST
    HAL_Display_Present();
//#endif
}

// Dibuja una línea horizontal
static void draw_hline(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
    for (uint16_t i = 0; i < w; i++)
        draw_pixel(x + i, y, color);
}

// Dibuja una línea vertical
static void draw_vline(uint16_t x, uint16_t y, uint16_t h, uint16_t color)
{
    for (uint16_t i = 0; i < h; i++)
        draw_pixel(x, y + i, color);
}

// Cuadro
static void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    draw_hline(x, y, w, color);
    draw_hline(x, y + h - 1, w, color);
    draw_vline(x, y, h, color);
    draw_vline(x + w - 1, y, h, color);
}

// Patrón de colores
static void color_test_pattern(void)
{
    uint16_t colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA, COLOR_WHITE};
    const char* names[] = {"RED", "GREEN", "BLUE", "YELLOW", "CYAN", "MAGENTA", "WHITE"};
    uint8_t n = sizeof(colors)/sizeof(colors[0]);

    uint16_t bandHeight = HEIGHT / n;

    for (uint8_t i = 0; i < n; i++) {
        fill_rect(0, i * bandHeight, WIDTH, bandHeight, colors[i]);
        printf("[TEST] Color %s\n", names[i]);
        HAL_DelayMs(500);
    }
}

// Patrón de cuadros y líneas
static void draw_shapes(void)
{
    fill_rect(0, 0, WIDTH, HEIGHT, COLOR_BLACK);
    printf("[TEST] Drawing shapes...\n");

    draw_rect(20, 20, 100, 60, COLOR_WHITE);
    draw_rect(140, 40, 60, 120, COLOR_GREEN);
    draw_rect(240, 100, 60, 100, COLOR_RED);

    // Líneas cruzadas
    for (int x = 0; x < WIDTH; x += 20)
        draw_vline(x, 0, HEIGHT, COLOR_GRAY);

    for (int y = 0; y < HEIGHT; y += 20)
        draw_hline(0, y, WIDTH, COLOR_GRAY);

    //Only for simulation
    HAL_Display_Present();

}

// Simulación texto (solo rectángulos blancos)
static void fake_text(uint16_t x, uint16_t y, const char* str, uint16_t color)
{
    uint16_t w = strlen(str) * 8;
    uint16_t h = 12;
    fill_rect(x, y, w, h, color);
}

// Test de encendido/apagado
static void test_display_power(void)
{
    printf("[TEST] Turning display OFF for 1s...\n");
    HAL_Display_Off();
    HAL_DelayMs(1000);

    printf("[TEST] Turning display ON...\n");
    HAL_Display_On();
}

int lcd_test(void)
{
    printf("=== [DISPLAY VISUAL TEST START] ===\n");

    HAL_GPIO_Init();
    HAL_SPI_Init();
    HAL_Display_Init();
    HAL_Display_Reset();

    fill_rect(0, 0, WIDTH, HEIGHT, COLOR_WHITE);
    HAL_DelayMs(500);

    // Fondo de colores
    color_test_pattern();

    // Formas geométricas
    draw_shapes();

    // Texto simulado
    fake_text(30, 210, "TFT SIMULATION OK", COLOR_WHITE);

    //Only for display
    HAL_Display_Present();

    HAL_DelayMs(1500);

    // Encendido / apagado
    test_display_power();

    // Texto simulado
    fill_rect(0, 0, WIDTH, HEIGHT, COLOR_GREEN);
    fake_text(30, 210, "TFT SIMULATION ON", COLOR_RED);

    //Only on simulator 
    HAL_Display_Present();

    /*ONLy for simulation */
    int running = 1;
    while (running)
    {
        HAL_Poll_Events(&running);
        HAL_DelayMs(16);
    }

/*
    while (1)
    {
        color_test_pattern();
        draw_shapes();
        HAL_DelayMs(1000);
    }
*/
    printf("=== [DISPLAY VISUAL TEST END] ===\n");
    return 0;
}
