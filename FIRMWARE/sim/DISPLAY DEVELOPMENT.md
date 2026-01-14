
# Subsystem Display (128×64, 1‑bpp) — Step-by-Step Guide

## 🎯 Objective
This guide provides a practical explanation of how the simulated display for the steering wheel works:
- what the (portable) driver does vs. the HAL (target-dependent),
- which functions are called and in what order,
- how to integrate it into the application,
- how to build and run it in simulator mode (host_pc).

The display is modeled as a 1‑bpp (1 bit/pixel) framebuffer with a typical resolution of 128×64 and a “paged” layout compatible with SSD1309/SSD1306.

---

## 🧭 Layered Architecture (why)
To keep the app code portable (PC and MCU), we separate it into 3 layers:

1) App (e.g., app/app_main.c): uses high-level “display_*” APIs; knows nothing about SDL or SPI.
2) Display driver (drivers/display.*): manages the framebuffer, graphics primitives, and `display_flush()`.
3) Display HAL (hal/…/hal_display_*): converts the framebuffer into real output:
    - host_pc: expands 1‑bpp → ARGB and draws in an SDL2 window,
    - target_s32k: (future) sends pages via SPI/I²C to the SSD1309 controller.

Advantages: application logic does not change when switching platforms.

---

## 🧱 1‑bpp Framebuffer Layout (how)
Resolution: 128×64 → 8192 bits → 1024 bytes total.
- Height is divided into 8 pages (64/8 = 8).
- Each page contains 128 bytes, one per column x.
- Each byte represents 8 vertical pixels (bits 0..7) for local rows y%8.

Useful indices
- page = y >> 3
- bit  = y & 7
- idx  = page * WIDTH + x
- fb[idx] bit ‘bit’ → pixel (x,y)

This scheme is identical to that used by SSD1309/SSD1306 controllers and simplifies future porting to MCUs.

---

## 🧩 Public Driver APIs (display.h)
Main functions exposed to the application:

- int display_init(int width, int height, int scale)  
  Initializes the 1‑bpp framebuffer (width×height/8 bytes) and the HAL. `scale` is a zoom factor for the SDL window (e.g., 4 → 512×256). Returns 0 on success.

- void display_shutdown(void)  
  Frees driver resources and closes the HAL (SDL window on host).

- void display_clear(uint8_t color)  
  Clears the framebuffer. color=0 black, color=1 white (all bits set).

- void display_draw_pixel(int x, int y, uint8_t c)  
  Sets/clears the pixel (internal clipping). `c` 0/1.

- void display_flush(void)  
  Delivers the ready 1‑bpp image to the HAL for display/transmission (on host_pc updates the SDL texture).

Utility primitives
- display_draw_hline, display_draw_vline, display_draw_rect, display_fill_rect
- display_demo_frame(t) for a test frame (border + animated bar).

Why they are defined this way
- `display_*` hides platform details; the app works in coordinates/pixels, not registers or graphics libraries.
- `display_flush()` separates “drawing” from “presenting”: useful for both SDL (one update per frame) and SSD1309 (page-wise transfer).

---

## 🧩 HAL APIs (hal_display.h)
- int  hal_display_init(int width, int height, int scale)  
  On host, creates SDL window, renderer, and “logical” w×h texture (scaled to window with `scale`).

- void hal_display_shutdown(void)  
  Destroys texture/renderer/window (SDL_Quit).

- void hal_display_present_1bpp(const uint8_t* fb_bits, int width, int height)  
  Converts the 1‑bpp framebuffer (paged layout) to ARGB8888 and displays it via SDL.

- void hal_poll_events(int* running)  
  Handles events (window close, etc.). If SDL_QUIT is received, sets `*running = 0`.

- void hal_delay_ms(int ms)  
  Convenience pause (SDL_Delay) to limit frame rate (e.g., ~60 FPS with 16 ms).

Why this way
- The minimal APIs cover initialization/teardown, image presentation, and event/timing management for the loop; they are easily replaceable on MCUs.

---

## ▶️ Typical Main Loop Flow
1) display_init(128, 64, 4)  
2) while (running) {  
    • hal_poll_events(&running)  
    • display_clear(0)  
    • [draw with primitives]  
    • display_flush()  
    • hal_delay_ms(16)  
}  
3) display_shutdown()

This pattern is portable: on host_pc it keeps the window responsive; on MCU it will send the framebuffer to the physical display in the future.

---

## 🔧 Integration in Files
Driver (portable)
- drivers/display.h/.c  
  • maintains 1‑bpp framebuffer (size = width×height/8)  
  • implements primitives and `display_flush()` → calls HAL 1‑bpp

HAL host (SDL2)
- hal/hal_display.h  
- hal/host_pc/hal_display_host.c  
  • `hal_display_init` → SDL_Init + Window + Renderer + ARGB Texture w×h  
  • `hal_display_present_1bpp` → 1‑bpp→ARGB expansion and render/present  
  • `hal_poll_events`, `hal_delay_ms`

Note: for the MCU, `hal_display_host.c` will be replaced with `hal_display_s32k.c` that sends pages via SPI/I²C.

---

## 🧱 Build on PC (host_pc)
MSYS2/UCRT64 prerequisites (Windows)
- packages: gcc, make, pkgconf, SDL2

Check:
```
which gcc           # /ucrt64/bin/gcc
which make          # /ucrt64/bin/make
pkg-config --cflags sdl2  # should show -I.../include/SDL2
```

Makefile (excerpt)
- include path: `-I./drivers -I./hal -I./hal/host_pc`
- link SDL2 in “sim” and/or “sim_demo”:
  - `CFLAGS += $(shell pkg-config --cflags sdl2)`
  - `LDLIBS += $(shell pkg-config --libs sdl2)`

Commands
```
make sim      # build version with official main
make run      # runs app_sim
# or
make sim_demo # build standalone demo
make run_demo
```

---

## 🧩 Implementation Choices (rationale)
- 1‑bpp “paged”: matches hardware → zero conversions on MCU side; good performance on PC (8192 bits).
- explicit flush: separates drawing and presentation; fits the “page transfer” model of OLEDs.
- SDL ARGB w×h texture: simple, cross‑platform update; clean scaling with `scale`.
- minimal primitives: allow composing UI (rectangles, bars, text).

---

## 🆘 Troubleshooting
- VSCode highlights `stdio.h`, `SDL.h` in red → configure `.vscode/c_cpp_properties.json` with `compilerPath: C:/msys64/ucrt64/bin/gcc.exe` and `includePath` for `C:/msys64/ucrt64/include/SDL2`.
- `pkg-config --cflags sdl2` prints nothing → install `mingw-w64-ucrt-x86_64-pkgconf` and `mingw-w64-ucrt-x86_64-SDL2`.
- black window: check that `display_flush()` is called and that the 1‑bpp→ARGB conversion is done on the correct frame.
- height not a multiple of 8 → `display_init` fails by design (required by page layout).

