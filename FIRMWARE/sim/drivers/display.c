/**
 * @file display.c
 * @brief Implementation of the 1-bit-per-pixel monochrome display driver.
 *
 * @details
 * This file implements a framebuffer-based graphics driver for displays using a
 * vertical page memory layout (e.g., 128x64 OLED). It supports basic drawing primitives
 * and text rendering using a 5x7 pixel font.
 *
 * The driver maintains an internal framebuffer, where each bit represents one pixel.
 * The display is updated by calling `display_flush()`, which delegates the data transfer
 * to the HAL layer (`hal_display_present_1bpp()`).
 *
 * @note
 * Designed for compatibility with both embedded hardware and PC simulation environments.
 */

// --- INCLUDES ---
#include "display.h"      // Includes the public function declarations for this driver.
#include "hal_display.h"  // Includes the HAL function declarations (e.g., hal_display_init, hal_display_present_1bpp).
#include <string.h>       // Includes standard C string/memory functions like memset, memcpy.
#include <stdlib.h>       // Includes standard C library functions like malloc, free.
#include <stdio.h>        // Includes standard I/O functions like snprintf (needed for display_draw_number).

// --- STATIC VARIABLES ---
// These variables store the display dimensions and a pointer to the framebuffer memory.
// `static` means they are only visible within this file.

static int g_w = 0, g_h = 0;  /**< Display dimensions (width, height). */
static uint8_t *g_fb = NULL;  /* Pointer to the framebuffer memory.
                               * 1-bpp: Each pixel is represented by 1 bit (0=off/black, 1=on/white).
                               * page layout: Memory is organized vertically in pages of 8 rows.
                               * size = (width * height) / 8 bytes. For 128x64, size = 1024 bytes. */

// --- UTILITY MACROS ---
// These macros help calculate framebuffer parameters based on width and height.

// BYTES_PER_PAGE: Calculates the number of bytes needed to store one horizontal row of pages.
// Since each byte stores 8 vertical pixels in a column, for a width 'w', you need 'w' bytes per page row.
#define BYTES_PER_PAGE(w)   (w)             /* e.g., 128 bytes per page row for w=128 */

// PAGES: Calculates the total number of 8-pixel high pages for a given height 'h'.
#define PAGES(h)            ((h) >> 3)      /* h / 8 using bit shift */

// FB_SIZE_BYTES: Calculates the total size of the framebuffer in bytes.
#define FB_SIZE_BYTES(w,h)  ((w) * (h) / 8) // width * height / 8 bits per byte

/* --- STATIC HELPER FUNCTION --- */
/**
 * @brief Sets or clears a pixel in the framebuffer.
 *
 * @param[in] x X coordinate.
 * @param[in] y Y coordinate.
 * @param[in] on 1 to turn pixel on, 0 to turn it off.
 */
static inline void fb_set_pixel(int x, int y, uint8_t on)
{
    // Bounds checking: Ignore requests outside the display area.
    // Using unsigned comparison handles negative coordinates as well (they wrap around to large positive numbers).
    if ((unsigned)x >= (unsigned)g_w || (unsigned)y >= (unsigned)g_h) return;

    // Calculate the page number (0 to 7 for h=64). Each page is 8 pixels high.
    int page = y >> 3;       /* y / 8 */
    // Calculate the bit index within the byte (0 to 7). This corresponds to the row within the page.
    int bit  = y & 7;        /* y % 8 */

    // Calculate the linear index of the byte in the framebuffer array.
    // `page * BYTES_PER_PAGE(g_w)` finds the start of the correct page row.
    // `+ x` moves to the correct column within that page row.
    size_t idx = (size_t)page * BYTES_PER_PAGE(g_w) + (size_t)x;

    // Create a bitmask for the specific pixel within the byte.
    // `1u << bit` creates a byte with only the 'bit'-th bit set (e.g., bit=0 -> 0x01, bit=1 -> 0x02, bit=7 -> 0x80).
    uint8_t mask = (uint8_t)(1u << bit);

    // Modify the byte in the framebuffer.
    if (on) {
        g_fb[idx] |= mask; // Turn pixel ON: Use bitwise OR to set the specific bit to 1, leaving others unchanged.
    } else {
        g_fb[idx] &= (uint8_t)~mask; // Turn pixel OFF: Use bitwise AND with the inverted mask to clear the specific bit to 0.
    }
}

// --- FONT RENDERING LOGIC ---

/**
 * @brief Retrieves 5x7 pixel glyph data for an ASCII character.
 *
 * @param[in] c Character ASCII to render.
 * @param[out] rows Array of 7 bytes, each representing one row of pixel data.
 * Each byte in `rows` represents a row, with the 5 least significant bits used.
 * Bit 4 = leftmost pixel, Bit 0 = rightmost pixel.
 * @return int Always returns 1 (success).
 */
static int glyph5x7_rows(char c, uint8_t rows[7]) {
    // Default to space character if lookup fails.
    static const uint8_t SPACE[7] = {0,0,0,0,0,0,0}; // All pixels off.
    memcpy(rows, SPACE, 7); // Copy the space pattern into the output array initially.

    // Use a switch statement for efficient character lookup.
    // The constants (e.g., 0x0E, 0x11) represent the pixel pattern for each row.
    // Example '0': 0x0E = 00001110 (Top row), 0x11 = 00010001 (Sides), etc.
    switch (c) {
        case ' ': return 1; // Handled by the initial memcpy.

        // ===== DIGITS 0..9 =====
        case '0': { uint8_t r[7]={0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}; memcpy(rows,r,7); return 1; }
        case '1': { uint8_t r[7]={0x04,0x0C,0x04,0x04,0x04,0x04,0x1F}; memcpy(rows,r,7); return 1; }
        case '2': { uint8_t r[7]={0x0E,0x11,0x01,0x02,0x04,0x08,0x1F}; memcpy(rows,r,7); return 1; }
        case '3': { uint8_t r[7]={0x1F,0x02,0x04,0x02,0x01,0x11,0x0E}; memcpy(rows,r,7); return 1; }
        case '4': { uint8_t r[7]={0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}; memcpy(rows,r,7); return 1; }
        case '5': { uint8_t r[7]={0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}; memcpy(rows,r,7); return 1; }
        case '6': { uint8_t r[7]={0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}; memcpy(rows,r,7); return 1; }
        case '7': { uint8_t r[7]={0x1F,0x01,0x02,0x04,0x08,0x08,0x08}; memcpy(rows,r,7); return 1; }
        case '8': { uint8_t r[7]={0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}; memcpy(rows,r,7); return 1; }
        case '9': { uint8_t r[7]={0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}; memcpy(rows,r,7); return 1; }

        // ===== SYMBOLS =====
        case ':': { uint8_t r[7]={0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x00}; memcpy(rows,r,7); return 1; }
        case '.': { uint8_t r[7]={0x00,0x00,0x00,0x00,0x0C,0x0C,0x00}; memcpy(rows,r,7); return 1; }
        case '-': { uint8_t r[7]={0x00,0x00,0x00,0x1F,0x00,0x00,0x00}; memcpy(rows,r,7); return 1; }
        case '>': { uint8_t r[7]={0x10,0x08,0x04,0x02,0x04,0x08,0x10}; memcpy(rows,r,7); return 1; }
        case '<': { uint8_t r[7]={0x01,0x02,0x04,0x08,0x04,0x02,0x01}; memcpy(rows,r,7); return 1; }
        case '[': { uint8_t r[7]={0x0E,0x08,0x08,0x08,0x08,0x08,0x0E}; memcpy(rows,r,7); return 1; }
        case ']': { uint8_t r[7]={0x0E,0x02,0x02,0x02,0x02,0x02,0x0E}; memcpy(rows,r,7); return 1; }

        case '%': { uint8_t r[7]={0x11,0x01,0x02,0x04,0x08,0x10,0x11}; memcpy(rows,r,7); return 1; }
        case '°': { uint8_t r[7]={0x06,0x09,0x06,0x00,0x00,0x00,0x00}; memcpy(rows,r,7); return 1; }

        // ===== UPPERCASE LETTERS (only those needed for "HELLO" are defined) =====
        case 'A': { uint8_t r[7]={0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}; memcpy(rows,r,7); return 1; }
        case 'E': { uint8_t r[7]={0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}; memcpy(rows,r,7); return 1; }
        case 'H': { uint8_t r[7]={0x11,0x11,0x11,0x1F,0x11,0x11,0x11}; memcpy(rows,r,7); return 1; }
        case 'L': { uint8_t r[7]={0x10,0x10,0x10,0x10,0x10,0x10,0x1F}; memcpy(rows,r,7); return 1; }
        case 'O': { uint8_t r[7]={0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}; memcpy(rows,r,7); return 1; }
        case 'B': { uint8_t r[7]={0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}; memcpy(rows,r,7); return 1; }
        case 'C': { uint8_t r[7]={0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}; memcpy(rows,r,7); return 1; }
        case 'D': { uint8_t r[7]={0x1E,0x11,0x11,0x11,0x11,0x11,0x1E}; memcpy(rows,r,7); return 1; }
        case 'I': { uint8_t r[7]={0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}; memcpy(rows,r,7); return 1; }
        case 'N': { uint8_t r[7]={0x11,0x19,0x15,0x13,0x11,0x11,0x11}; memcpy(rows,r,7); return 1; }
        case 'P': { uint8_t r[7]={0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}; memcpy(rows,r,7); return 1; }
        case 'R': { uint8_t r[7]={0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}; memcpy(rows,r,7); return 1; }
        case 'S': { uint8_t r[7]={0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E}; memcpy(rows,r,7); return 1; }
        case 'T': { uint8_t r[7]={0x1F,0x04,0x04,0x04,0x04,0x04,0x04}; memcpy(rows,r,7); return 1; }
        case 'U': { uint8_t r[7]={0x11,0x11,0x11,0x11,0x11,0x11,0x0E}; memcpy(rows,r,7); return 1; }
        case 'W': { uint8_t r[7]={0x11,0x11,0x11,0x15,0x15,0x15,0x0A}; memcpy(rows,r,7); return 1; }
        case 'Y': { uint8_t r[7]={0x11,0x11,0x0A,0x04,0x04,0x04,0x04}; memcpy(rows,r,7); return 1; }
        case 'G': { uint8_t r[7]={0x0E,0x11,0x10,0x13,0x11,0x11,0x0E}; memcpy(rows,r,7); return 1; }

        // Add more letters (B, C, D, F, G, I, J, K, M, N, P-Z) here if needed.

        // Handle lowercase by converting to uppercase recursively.
        default:
            if (c >= 'a' && c <= 'z') {
                // Recursive call with the uppercase equivalent character.
                return glyph5x7_rows((char)(c - 'a' + 'A'), rows);
            }
            // If character is not found and not lowercase, it remains a space (from initial memcpy).
            return 1;
    }
}

// display_draw_char: Draws a single character at the specified position.
//   x, y: Top-left corner coordinates for the character.
//   c: The ASCII character to draw.
//   color: 1 for white, 0 for black.
void display_draw_char(int x, int y, char c, int color) {
    uint8_t rows[7]; // Array to hold the pixel data for the character's 7 rows.
    glyph5x7_rows(c, rows); // Get the pixel data using the helper function.

    // Iterate through each row of the character (0 to 6).
    for (int ry = 0; ry < 7; ++ry) {
        uint8_t line = rows[ry]; // Get the pixel data for the current row.
        // Iterate through each column of the character (0 to 4).
        for (int cx = 0; cx < 5; ++cx) {
            // Check if the pixel for this column should be drawn.
            // (1u << (4 - cx)) creates a mask for the current column bit (bit 4 is leftmost).
            if (line & (1u << (4 - cx))) {
                // If the bit is set in the font data, draw a pixel at the calculated screen coordinate.
                // NOTE: Using display_fill_rect(..., 1, 1, color) is inefficient for single pixels.
                // It's better to use display_draw_pixel directly.
                // display_fill_rect(x + cx, y + ry, 1, 1, color); // Less efficient way
                 display_draw_pixel(x + cx, y + ry, (uint8_t)color); // More efficient
            }
            // else { /* Do nothing for background pixels (transparent background) */ }
        }
    }
}

// display_draw_text: Draws a null-terminated string at the specified position.
//   x, y: Top-left corner coordinates for the start of the string.
//   s: Pointer to the character string.
//   color: 1 for white, 0 for black.
void display_draw_text(int x, int y, const char* s, int color) {
    int cursor = 0; // Tracks the horizontal position for the next character.
    // Iterate through the string until the null terminator ('\0') is reached.
    for (const char* p = s; *p; ++p) {
        // Handle newline character '\n'.
        if (*p == '\n') {
            y += 8;      // Move down to the next line (assumes 7-pixel font + 1 pixel space).
            cursor = 0;  // Reset horizontal cursor to the starting x position.
            continue;    // Skip drawing the newline character itself.
        }
        // Draw the current character at the cursor position.
        display_draw_char(x + cursor, y, *p, color);
        // Advance the cursor for the next character: 5 pixels width + 1 pixel spacing.
        cursor += 6;
        // Basic check to prevent drawing off the right edge of the screen.
        if (x + cursor >= g_w) break;
    }
}

// display_draw_number: Draws an integer value at the specified position.
//   x, y: Top-left corner coordinates.
//   value: The integer number to draw.
//   color: 1 for white, 0 for black.
void display_draw_number(int x, int y, int value, int color) {
    char buf[16]; // A buffer to hold the string representation of the number.
                  // 16 chars is usually enough for typical integer ranges including sign.
    // Convert the integer 'value' into a string stored in 'buf'.
    // snprintf is safer than sprintf as it prevents buffer overflows.
    snprintf(buf, sizeof(buf), "%d", value);
    // Draw the resulting string using the text drawing function.
    display_draw_text(x, y, buf, color);
}

// --- PUBLIC FUNCTIONS ---

// display_init: Initializes the display driver and the underlying HAL.
//   width, height: Desired display dimensions.
//   scale: Scaling factor for the simulation window (used by HAL).
// Returns 0 on success, negative value on error.
int display_init(int width, int height, int scale)
{
    // Basic validation of dimensions. Height must be a multiple of 8 due to page layout.
    if (width <= 0 || height <= 0 || (height & 7)) return -1; /* h must be multiple of 8 */
    g_w = width; g_h = height;

    // Calculate required framebuffer size.
    size_t n = (size_t)FB_SIZE_BYTES(g_w, g_h);
    // Allocate memory for the framebuffer. Free old one if exists.
    if (g_fb) free(g_fb);
    g_fb = (uint8_t*)malloc(n);
    if (!g_fb) return -2; // Allocation failed.

    // Initialize the framebuffer memory to all zeros (black).
    memset(g_fb, 0x00, n);

    // Initialize the hardware abstraction layer (e.g., SDL for the host simulation).
    if (hal_display_init(g_w, g_h, scale) != 0) {
        // If HAL init fails, free the framebuffer memory we just allocated.
        free(g_fb); g_fb = NULL;
        return -3; // Return HAL error code.
    }
    return 0; // Success.
}

// display_shutdown: Releases resources used by the display driver and HAL.
void display_shutdown(void)
{
    // Free the framebuffer memory if it was allocated.
    if (g_fb) { free(g_fb); g_fb = NULL; }
    // Call the HAL shutdown function (e.g., closes SDL window).
    hal_display_shutdown();
}

// display_clear: Fills the entire framebuffer with a specified color.
//   color: 0 for black (all bits 0), 1 for white (all bits 1).
void display_clear(uint8_t color)
{
    if (!g_fb) return; // Do nothing if framebuffer isn't allocated.
    // Use memset to efficiently fill the memory block.
    // If color is 1 (white), fill with 0xFF. If color is 0 (black), fill with 0x00.
    memset(g_fb, (color ? 0xFF : 0x00), FB_SIZE_BYTES(g_w, g_h));
}

// display_draw_pixel: Sets a single pixel in the framebuffer to the specified color.
//   x, y: Coordinates of the pixel.
//   c: Color (0 for black, 1 for white).
void display_draw_pixel(int x, int y, uint8_t c)
{
    // Uses the internal helper function for actual pixel setting.
    fb_set_pixel(x, y, (c ? 1 : 0)); // Ensure 'on' parameter is strictly 0 or 1.
}

// display_draw_hline: Draws a horizontal line.
//   x, y: Starting coordinates (left end).
//   w: Width (length) of the line. Can be negative.
//   c: Color (0 for black, 1 for white).
void display_draw_hline(int x, int y, int w, uint8_t c)
{
    if (w < 0) { x += w + 1; w = -w; } // Adjust starting point and width if w is negative.
    // Draws pixel by pixel. Could be optimized for page-based displays
    // by writing full bytes when possible.
    for (int i = 0; i < w; ++i) display_draw_pixel(x + i, y, c);
}

// display_draw_vline: Draws a vertical line.
//   x, y: Starting coordinates (top end).
//   h: Height (length) of the line. Can be negative.
//   c: Color (0 for black, 1 for white).
void display_draw_vline(int x, int y, int h, uint8_t c)
{
    if (h < 0) { y += h + 1; h = -h; } // Adjust starting point and height if h is negative.
    // Draws pixel by pixel. This is efficient for page-based displays
    // as it modifies bits within the same byte column (mostly).
    for (int j = 0; j < h; ++j) display_draw_pixel(x, y + j, c);
}

// display_draw_rect: Draws the outline of a rectangle.
//   x, y: Top-left corner coordinates.
//   w, h: Width and height of the rectangle.
//   c: Color (0 for black, 1 for white).
void display_draw_rect(int x, int y, int w, int h, uint8_t c)
{
    if (w <= 0 || h <= 0) return; // Nothing to draw if width or height is zero/negative.
    // Draw the four sides using hline and vline functions.
    display_draw_hline(x, y, w, c);           // Top edge
    display_draw_hline(x, y + h - 1, w, c); // Bottom edge
    // Draw vertical lines excluding corners to avoid drawing them twice.
    display_draw_vline(x, y + 1, h - 2, c);       // Left edge (excluding corners)
    display_draw_vline(x + w - 1, y + 1, h - 2, c); // Right edge (excluding corners)
}

// display_fill_rect: Draws a filled rectangle.
//   x, y: Top-left corner coordinates.
//   w, h: Width and height of the rectangle.
//   c: Color (0 for black, 1 for white).
void display_fill_rect(int x, int y, int w, int h, uint8_t c)
{
    if (w <= 0 || h <= 0 || !g_fb) return; // Nothing to draw or no framebuffer.

    // Basic Clipping: Adjust coordinates and dimensions to stay within screen bounds.
    int x1 = x;
    int y1 = y;
    int x2 = x + w;
    int y2 = y + h;
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > g_w) x2 = g_w;
    if (y2 > g_h) y2 = g_h;

    // Calculate effective width and height after clipping.
    int clipped_w = x2 - x1;
    int clipped_h = y2 - y1;
    if (clipped_w <= 0 || clipped_h <= 0) return; // Nothing visible after clipping.

    // Draw using horizontal lines (less efficient than byte/page operations but simpler).
    for (int j = 0; j < clipped_h; ++j) {
        display_draw_hline(x1, y1 + j, clipped_w, c);
    }
}

// display_flush: Sends the contents of the in-memory framebuffer (g_fb) to the actual display hardware or simulation window.
void display_flush(void)
{
    if (!g_fb) return; // Do nothing if framebuffer isn't allocated.
    // Call the HAL function responsible for transferring the 1bpp framebuffer data.
    hal_display_present_1bpp(g_fb, g_w, g_h);
}
