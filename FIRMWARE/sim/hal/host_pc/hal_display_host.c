// hal_display.c
// Host PC (simulation) version of the Hardware Abstraction Layer (HAL) for a monochrome display.
// Uses SDL2 to simulate the physical display in a window, with 1-bit-per-pixel framebuffer.

// --- INCLUDES ---
#include "../hal_display.h" // HAL function prototypes for display
#include "hal_gpio.h"       // For forwarding key events to the button HAL
#include <SDL.h>            // SDL2 graphics and event handling
#include <stdlib.h>         // Standard library (general use)
#include <string.h>         // String/memory utilities
#include <stdint.h>         // Standard integer types

// --- STATIC GLOBAL VARIABLES ---
// SDL window, renderer, and texture handles
static SDL_Window   *g_win   = NULL; 
static SDL_Renderer *g_ren   = NULL;
static SDL_Texture  *g_tex   = NULL;
// Native display width, height, and scale factor
static int g_w = 0, g_h = 0, g_scale = 1;

// --- PUBLIC FUNCTIONS ---

/**
 * @brief Initializes the SDL window, renderer, and texture for the display simulation.
 * @param width  Native display width in pixels.
 * @param height Native display height in pixels.
 * @param scale  Integer scaling factor for the window.
 * @return 0 on success, negative value on failure.
 *
 * @details
 * Sets up SDL video subsystem, creates a window, renderer, and streaming texture.
 * The texture will hold the 1bpp framebuffer converted to 32bpp ARGB8888 format.
 */
int hal_display_init(int width, int height, int scale)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    g_w = width; g_h = height; g_scale = (scale > 0 ? scale : 1);

    g_win = SDL_CreateWindow(
        "F1 Steering Display",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        g_w * g_scale,
        g_h * g_scale,
        0
    );
    if (!g_win) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -2;
    }

    g_ren = SDL_CreateRenderer(
        g_win,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!g_ren) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_win);
        SDL_Quit();
        return -3;
    }

    g_tex = SDL_CreateTexture(
        g_ren,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        g_w,
        g_h
    );
    if (!g_tex) {
        fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(g_ren);
        SDL_DestroyWindow(g_win);
        SDL_Quit();
        return -4;
    }

    return 0;
}

/**
 * @brief Shuts down the display simulation, destroying SDL resources.
 */
void hal_display_shutdown(void)
{
    if (g_tex) { SDL_DestroyTexture(g_tex); g_tex = NULL; }
    if (g_ren) { SDL_DestroyRenderer(g_ren); g_ren = NULL; }
    if (g_win) { SDL_DestroyWindow(g_win); g_win = NULL; }
    SDL_Quit();
}

/**
 * @brief Updates the SDL texture with a 1bpp framebuffer and renders it to the window.
 * @param fb_bits Pointer to 1bpp framebuffer (page layout).
 * @param w       Width of framebuffer in pixels.
 * @param h       Height of framebuffer in pixels.
 *
 * @details
 * Converts 1bpp data to 32bpp ARGB8888 pixels, locks the texture, copies data, unlocks,
 * and renders it stretched to the window.
 */
void hal_display_present_1bpp(const uint8_t* fb_bits, int w, int h)
{
    if (!g_tex || w != g_w || h != g_h || !fb_bits) return;

    void* pixels = NULL;
    int pitch = 0;
    if (SDL_LockTexture(g_tex, NULL, &pixels, &pitch) != 0) {
        fprintf(stderr, "SDL_LockTexture Error: %s\n", SDL_GetError());
        return;
    }

    for (int y = 0; y < h; ++y) {
        uint32_t* dst = (uint32_t*)((uint8_t*)pixels + y * pitch);
        int page = y >> 3;
        int bit  = y & 7;
        int base = page * w;
        for (int x = 0; x < w; ++x) {
            uint8_t byte = fb_bits[base + x];
            uint8_t on   = (byte >> bit) & 1u;
            dst[x] = on ? 0xFFFFFFFFu : 0xFF000000u;
        }
    }

    SDL_UnlockTexture(g_tex);

    SDL_RenderClear(g_ren);
    SDL_Rect dst_rect = {0, 0, g_w * g_scale, g_h * g_scale};
    SDL_RenderCopy(g_ren, g_tex, NULL, &dst_rect);
    SDL_RenderPresent(g_ren);
}

/**
 * @brief Processes SDL events such as key presses or window close events.
 * @param running Pointer to main loop control variable (0 = exit).
 *
 * @details
 * Maps SDL key events to HAL_GPIO_on_key to simulate button presses.
 * Sets `*running` to 0 if the user requests quit (window close or ESC/q key).
 */
void hal_poll_events(int* running)
{
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

/**
 * @brief Pauses execution for a specified number of milliseconds using SDL_Delay.
 * @param ms Number of milliseconds to delay.
 */
void hal_delay_ms(int ms)
{
    if (ms < 0) ms = 0;
    SDL_Delay((Uint32)ms);
}

/**
 * @brief Returns the number of milliseconds since SDL_Init() was called.
 * @return Milliseconds elapsed.
 */
uint32_t hal_get_ticks(void) {
    return SDL_GetTicks();
}
