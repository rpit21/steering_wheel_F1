/**
 * @file hal_lcd_host.c
 * @brief Host (PC) simulation of TFT display (ILI9341) using SDL2.
 *
 * @details
 * This module simulates a 320x240 TFT display with RGB565 framebuffer.
 * - SDL2 creates a window that mimics the physical display.
 * - Commands and data sent to the display are interpreted and drawn to the SDL texture.
 * - Supports basic display operations: ON/OFF, reset, memory write, and column/page addressing.
 * - Keyboard events are forwarded to HAL GPIO button simulation.
 */

#include "hal_lcd.h"
#include "hal_delay.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>

/*-------------------------- DEFINES --------------------------*/

#define TFT_WIDTH    320
#define TFT_HEIGHT   240
#define DISPLAY_SCALE 3   // Visual scaling factor (x2, x3, etc.)

/*-------------------------- STATIC GLOBAL VARIABLES --------------------------*/

static SDL_Window*    window   = NULL;           // SDL window pointer
static SDL_Renderer*  renderer = NULL;           // SDL renderer pointer
static SDL_Texture*   texture  = NULL;           // SDL texture pointer

static uint16_t framebuffer[TFT_WIDTH * TFT_HEIGHT]; // RGB565 framebuffer

static uint8_t  lastCmd    = 0;  // Last command sent
static uint8_t  displayOn  = 1;  // Display ON/OFF flag

// Window coordinates for drawing (simulates "address window")
static uint16_t windowX0 = 0, windowY0 = 0, windowX1 = TFT_WIDTH - 1, windowY1 = TFT_HEIGHT - 1;
static uint16_t curX = 0, curY = 0;  // Current pixel coordinates

/*-------------------------- INTERNAL FUNCTIONS --------------------------*/

/**
 * @brief Converts RGB565 color to ARGB8888 for SDL texture.
 */
static uint32_t rgb565_to_argb8888(uint16_t color) {
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5)  & 0x3F;
    uint8_t b = color & 0x1F;
    return 0xFF000000 | (r << 19) | (g << 10) | (b << 3);
}

/**
 * @brief Updates the SDL texture with the current framebuffer content.
 */
static void update_texture(void) {
    if (!displayOn) return; // Skip update if display is OFF

    void* pixels;
    int pitch;
    if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) return;

    uint32_t* dst = (uint32_t*)pixels;
    for (int y = 0; y < TFT_HEIGHT; ++y) {
        for (int x = 0; x < TFT_WIDTH; ++x) {
            dst[y * (pitch / 4) + x] = rgb565_to_argb8888(framebuffer[y * TFT_WIDTH + x]);
        }
    }

    SDL_UnlockTexture(texture);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

/*-------------------------- PUBLIC API FUNCTIONS --------------------------*/

/**
 * @brief Initializes the TFT display simulation.
 */
void HAL_Display_Init(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return;
    }

    window = SDL_CreateWindow(
        "TFT Display (ILI9341 Simulation)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        TFT_WIDTH*DISPLAY_SCALE, TFT_HEIGHT*DISPLAY_SCALE, 0
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 TFT_WIDTH, TFT_HEIGHT);

    SDL_RenderSetScale(renderer, DISPLAY_SCALE, DISPLAY_SCALE);
    memset(framebuffer, 0, sizeof(framebuffer));
    displayOn = 1;

    printf("[HAL_DISPLAY_HOST] Initialized TFT %dx%d (scale x%d)\n",
           TFT_WIDTH, TFT_HEIGHT, DISPLAY_SCALE);
}

/**
 * @brief Resets the display (clears framebuffer and updates SDL renderer).
 */
void HAL_Display_Reset(void) {
    memset(framebuffer, 0, sizeof(framebuffer));
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    HAL_DelayMs(100);
    printf("[HAL_DISPLAY_HOST] Display reset\n");
}

/**
 * @brief Turns the display OFF (clears screen, stops updates).
 */
void HAL_Display_Off(void) {
    displayOn = 0;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    printf("[HAL_DISPLAY_HOST] Display OFF\n");
}

/**
 * @brief Turns the display ON (reenables updates).
 */
void HAL_Display_On(void) {
    displayOn = 1;
    printf("[HAL_DISPLAY_HOST] Display ON\n");
}

/**
 * @brief Writes a command byte to the display simulation.
 */
void HAL_Display_WriteCommand(uint8_t cmd) {
    lastCmd = cmd;
    //printf("[HAL_DISPLAY_HOST] CMD: 0x%02X\n", cmd);

    if (cmd == 0x28) HAL_Display_Off();  // Display OFF
    if (cmd == 0x29) HAL_Display_On();   // Display ON

    if (cmd == 0x2A) { /* Column address set */ }
    else if (cmd == 0x2B) { /* Page address set */ }
    else if (cmd == 0x2C) { /* Memory write */ curX = windowX0; curY = windowY0; }
}

/**
 * @brief Writes a data byte to the display simulation.
 */
void HAL_Display_WriteData(uint8_t data) {
    static uint8_t buf[4];
    static int idx = 0;

    buf[idx++] = data;

    if (lastCmd == 0x2A && idx == 4) { // Column set
        windowX0 = (buf[0] << 8) | buf[1];
        windowX1 = (buf[2] << 8) | buf[3];
        idx = 0;
    } else if (lastCmd == 0x2B && idx == 4) { // Page set
        windowY0 = (buf[0] << 8) | buf[1];
        windowY1 = (buf[2] << 8) | buf[3];
        idx = 0;
    } else if (lastCmd == 0x2C && idx == 2) { // Pixel write RGB565
        uint16_t color = (buf[0] << 8) | buf[1];
        if (curX >= windowX0 && curX <= windowX1 &&
            curY >= windowY0 && curY <= windowY1 &&
            curX < TFT_WIDTH && curY < TFT_HEIGHT)
            framebuffer[curY * TFT_WIDTH + curX] = color;

        curX++;
        if (curX > windowX1) { curX = windowX0; curY++; }
        if (curY > windowY1) curY = windowY0;

        idx = 0;
    }
}

/**
 * @brief Writes a buffer of data bytes to the display.
 */
void HAL_Display_WriteDataBuffer(const uint8_t* data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) HAL_Display_WriteData(data[i]);
}

/**
 * @brief Updates the SDL texture with the current framebuffer.
 */
void HAL_Display_Present(void) {
    update_texture();
}

/**
 * @brief Polls SDL events and forwards key events to GPIO simulation.
 */
void HAL_Poll_Events(int* running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            if (running) *running = 0;
            break;
        } else if (e.type == SDL_KEYDOWN) {
            HAL_GPIO_on_key(e.key.keysym.sym, 1);
            if (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q) {
                if (running) *running = 0;
                break;
            }
        } else if (e.type == SDL_KEYUP) {
            HAL_GPIO_on_key(e.key.keysym.sym, 0);
        }
    }
}
