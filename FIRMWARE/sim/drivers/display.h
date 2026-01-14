/**
 * @file display.h
 * @brief High-level driver interface for monochrome display rendering.
 *
 * @details
 * This module provides an abstraction layer for drawing graphics and text
 * on a monochrome display (e.g., 128x64 OLED/LCD).  
 * It manages a 1-bit-per-pixel framebuffer and exposes a set of drawing
 * primitives such as pixels, lines, rectangles, text, and numbers.
 *
 * The driver communicates with the hardware through the HAL (`hal_display.h`),
 * which is responsible for the actual data transfer or simulation window.
 *
 * @note
 * The framebuffer must be initialized using `display_init()` before calling
 * any drawing or rendering functions.  
 * After drawing operations, call `display_flush()` to update the physical display.
 */
#ifndef DISPLAY_H 
#define DISPLAY_H 

// --- INCLUDES ---
#include <stdint.h> 


/*--- DEFINITIONS --- */

/** @brief Display width in pixels. */
#define DISPLAY_WIDTH   128 

/** @brief Display height in pixels. */
#define DISPLAY_HEIGHT  64  


/* --- PUBLIC FUNCTION --- */

/*--- Core Display Control Functions ---*/

/**
 * @brief Initializes the display driver and underlying HAL.
 *
 * @details
 * Allocates memory for the framebuffer, initializes display dimensions,
 * and sets up the hardware abstraction layer.
 *
 * @param[in] width Display width in pixels.
 * @param[in] height Display height in pixels (must be multiple of 8).
 * @param[in] scale Scaling factor for simulation environments (e.g., SDL window).
 * @return int 0 on success, negative value on failure.
 */
int  display_init(int width, int height, int scale);

/**
 * @brief Shuts down the display driver and releases allocated resources.
 *
 * @details
 * Frees framebuffer memory and calls HAL shutdown routines.
 * Should be invoked once when the application terminates.
 */
void display_shutdown(void);

/**
 * @brief Clears the entire framebuffer with a uniform color.
 *
 * @param[in] color 0 for black (all pixels off), 1 for white (all pixels on).
 */
void display_clear(uint8_t color);

/**
 * @brief Draws a single pixel on the framebuffer.
 *
 * @param[in] x X coordinate of the pixel.
 * @param[in] y Y coordinate of the pixel.
 * @param[in] c Pixel color (0 = black, 1 = white).
 */
void display_draw_pixel(int x, int y, uint8_t c);

/**
 * @brief Transfers the framebuffer contents to the physical display.
 *
 * @details
 * The HAL handles the actual transfer (e.g., via SPI, I²C, or host simulation).
 * Must be called after drawing operations to make updates visible.
 */
void display_flush(void);


/* --- Graphics Primitives --- */

/**
 * @brief Draws a horizontal line.
 *
 * @param[in] x Starting X coordinate (left end).
 * @param[in] y Starting Y coordinate.
 * @param[in] w Line width in pixels.
 * @param[in] c Line color (0 = black, 1 = white).
 */
void display_draw_hline(int x, int y, int w, uint8_t c);

/**
 * @brief Draws a vertical line.
 *
 * @param[in] x Starting X coordinate.
 * @param[in] y Starting Y coordinate (top end).
 * @param[in] h Line height in pixels.
 * @param[in] c Line color (0 = black, 1 = white).
 */
void display_draw_vline(int x, int y, int h, uint8_t c);

/**
 * @brief Draws the outline of a rectangle.
 *
 * @param[in] x Top-left X coordinate.
 * @param[in] y Top-left Y coordinate.
 * @param[in] w Rectangle width.
 * @param[in] h Rectangle height.
 * @param[in] c Border color (0 = black, 1 = white).
 */
void display_draw_rect(int x, int y, int w, int h, uint8_t c);

/**
 * @brief Draws a filled (solid) rectangle.
 *
 * @param[in] x Top-left X coordinate.
 * @param[in] y Top-left Y coordinate.
 * @param[in] w Rectangle width.
 * @param[in] h Rectangle height.
 * @param[in] c Fill color (0 = black, 1 = white).
 */
void display_fill_rect(int x, int y, int w, int h, uint8_t c);


/*--- Text Primitives ---*/

/**
 * @brief Draws a single ASCII character.
 *
 * @param[in] x X coordinate (top-left of character).
 * @param[in] y Y coordinate.
 * @param[in] c ASCII character to render.
 * @param[in] color Foreground color (0 = black, 1 = white).
 */
void display_draw_char(int x, int y, char c, int color);

/**
 * @brief Draws a null-terminated text string.
 *
 * @param[in] x X coordinate for the start of the string.
 * @param[in] y Y coordinate for the string baseline.
 * @param[in] s Pointer to the string to render.
 * @param[in] color Text color (0 = black, 1 = white).
 */
void display_draw_text(int x, int y, const char* s, int color);

/**
 * @brief Draws an integer value as text.
 *
 * @param[in] x X coordinate for the number.
 * @param[in] y Y coordinate.
 * @param[in] value Integer value to render.
 * @param[in] color Text color (0 = black, 1 = white).
 */
void display_draw_number(int x, int y, int value, int color);

#endif /* DISPLAY_H */