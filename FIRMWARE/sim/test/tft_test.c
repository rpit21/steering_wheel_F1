#include "TFT_LCD.h" // driver of the display
#include "hal_lcd.h" // HAL for simulation (only)
#include "hal_gpio.h" // For controling the gpio
#include "hal_delay.h" // For the delay
#include "buttons.h"    // Driver for controling the buttons
#include "tft_test.h"
#include "hal_spi.h"


#include "clutch.h"         // clutch_GetPercentage()
#include "rotary_switch.h"  // rotary_GetPosition()
#include "can.h"            // CAN_ReceiveECUStatus(), ECUStatus_t

#include <stdint.h>
#include <stdbool.h> 
#include <string.h> 
#include <stdio.h> 
#include <math.h>
#include <SDL2/SDL.h>

// --- HAL TICK (Local Definition) ---
// Returns the current time tick (milliseconds) using SDL.
uint32_t HAL_GetTick(void)
{
    return SDL_GetTicks();
}

// --- Forward Declarations ---
void test_callback_btn1(bool state);
void test_callback_btn2(bool state);
void test_callback_btn3(bool state);
void test_callback_btn4(bool state);

// --- Global Vars ---
static char test_msg[16] = "-";
static int current_gear = 0; // 0 = N (Neutral), 1–8 = gears
static bool pit_active = false;
static bool drs_active = false;


// --- Local SDL Keyboard Handler (Optimized for low latency) ---
// Handles SDL events (window close, keyboard input).
void local_sdl_keyboard_handler(int *running) {
    SDL_Event event;
    // Process all events in the queue for responsiveness
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *running = 0;
        }

        // Keyboard input handling (1, 2, 3, 4)
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_1: 
                    test_callback_btn1(true);
                    break;
                case SDLK_2: 
                    test_callback_btn2(true);
                    break;
                case SDLK_3: 
                    test_callback_btn3(true);
                    break;
                case SDLK_4: 
                    test_callback_btn4(true);
                    break;
            }
        }
    }
    // No explicit SDL_Delay here to maximize update rate
}


// --- Button Callbacks (Simulation Control) ---
void test_callback_btn1(bool state) {
    if (state) {
        if (current_gear < 8) current_gear++;
        strcpy(test_msg, "GEAR UP");
    }
}

void test_callback_btn2(bool state) {
    if (state) {
        if (current_gear > 0) current_gear--;
        strcpy(test_msg, "GEAR DOWN");
    }
}

void test_callback_btn3(bool state) {
    if (state) {
        drs_active = !drs_active;
        strcpy(test_msg, drs_active ? "DRS ON" : "DRS OFF");
    }
}

void test_callback_btn4(bool state) {
    if (state) {
        pit_active = !pit_active;
        strcpy(test_msg, pit_active ? "PIT ON" : "PIT OFF");
    }
}

// --- UI Rendering ---
// Renders the entire dashboard UI based on current simulation variables.
static void display_update_status_tft(float clutch_percentage,
                                      int rotary_pos,
                                      int t1, int t2,
                                      int gear,
                                      bool pit_active_param,
                                      bool drs_active_param,
                                      const char *msg) {
    // Clear screen
    LCD_fill_rectangle(0, 0, 320, 240, BLACK);

    // Temperatures (Y=20)
    LCD_draw_string(12, 20, "T1:", WHITE, BLACK, 2);
    LCD_draw_number(48, 20, t1, WHITE, BLACK, 2);
    LCD_draw_string(80, 20, "C", WHITE, BLACK, 2);

    LCD_draw_string(220, 20, "T2:", WHITE, BLACK, 2);
    LCD_draw_number(256, 20, t2, WHITE, BLACK, 2);
    LCD_draw_string(288, 20, "C", WHITE, BLACK, 2);

    bool temp_alarm = (t1 > 90) || (t2 > 90);

    // Clutch bar (Y=50)
    int clutchY = 50;
    LCD_draw_string(12, clutchY, "Clutch", WHITE, BLACK, 2);
    int barX = 100, barY = clutchY, barW = 160, barH = 18;
    LCD_draw_rectangle(barX, barY, barW, barH, WHITE);
    int fillW = (int)((clutch_percentage / 100.0f) * barW);
    if (fillW < 0) fillW = 0;
    if (fillW > barW) fillW = barW;

    uint16_t color_fill = GREEN;
    if (clutch_percentage > 70.0f) color_fill = RED;
    else if (clutch_percentage > 40.0f) color_fill = YELLOW;
    LCD_fill_rectangle(barX, barY, fillW, barH, color_fill);

    char clutch_buf[8];
    snprintf(clutch_buf, sizeof(clutch_buf), "%d%%", (int)clutch_percentage);
    LCD_draw_string(barX + barW + 10, barY, clutch_buf, WHITE, BLACK, 2);

    // Setup rotary (Y=80)
    int setupY = 80;
    LCD_draw_string(12, setupY, "SETUP:", WHITE, BLACK, 2);
    LCD_draw_char(110, setupY, '[', WHITE, BLACK, 2);
    LCD_draw_number(122, setupY, rotary_pos, WHITE, BLACK, 2);
    LCD_draw_char(136 + (rotary_pos > 9 ? 6 : 0), setupY, ']', WHITE, BLACK, 2);

    // Button message to the right of SETUP (Permanent)
    if (msg && strcmp(msg, "-") != 0) {
        char msg_copy[16];
        strncpy(msg_copy, msg, sizeof(msg_copy) - 1);
        msg_copy[sizeof(msg_copy) - 1] = '\0';
        LCD_draw_string(180, setupY, msg_copy, YELLOW, BLACK, 2);
    }

    // --- Gear box center (Centering Fix with Optimal Box Size) ---
    
    // Position/Size
    int gearBoxW = 54; // Reduced width for visual lateral centering
    int gearBoxH = 60; // Adjusted height
    int gearBoxX = (320 - gearBoxW) / 2; // Centered X: (320 - 54) / 2 = 133
    int gearBoxY = 135; // Start Y position 
    
    int fontSize = 6;
    int fontWidth = 6 * fontSize; // 36 pixels
    int fontHeight = 6 * fontSize; // 36 pixels
    
    // "GEAR" label (size 2, Y=105)
    LCD_draw_string(135, 105, "GEAR", WHITE, BLACK, 2); 
    
    // Gear box rectangle
    LCD_draw_rectangle(gearBoxX, gearBoxY, gearBoxW, gearBoxH, WHITE);
    
    // Calculate character position within the 54x60px box
    // X Offset = (54 - 36) / 2 = 9. -> charX = 133 + 9 = 142 (Mathematical Center)
    int charX = gearBoxX + (gearBoxW - fontWidth) / 2; 
    // Y Offset = (60 - 36) / 2 = 12. -> charY = 135 + 12 = 147 (Mathematical Center)
    int charY = gearBoxY + (gearBoxH - fontHeight) / 2; 

    // This is the critical adjustment section:
    // 1. Taglio in basso (verticale): Dobbiamo sollevare il carattere.
    // 2. Decentramento laterale: L'offset matematico (142) è corretto, ma applichiamo un piccolo offset ottico.
    charX += 1; // Sposta 1px a destra (centratura ottica)
    charY -= 1; // Sposta 1px in ALTO (solleva per evitare il taglio) 
    
    if (gear == 0)
        LCD_draw_char(charX, charY, 'N', CYAN, BLACK, fontSize);
    else
        LCD_draw_number(charX, charY, gear, CYAN, BLACK, fontSize);

    // --- Bottom Status Boxes ---
    
    // Bottom rectangles definition
    int cubeY = 215;
    int cubeW = 106;
    int cubeH = 25;

    // DRS
    LCD_draw_rectangle(0, cubeY, cubeW, cubeH, WHITE);
    uint16_t drs_bg_color = BLACK;
    if (drs_active_param) {
        drs_bg_color = BLUE;
        LCD_fill_rectangle(0, cubeY, cubeW, cubeH, drs_bg_color);
    }
    // Draw text AFTER fill to ensure contrast
    LCD_draw_string(36, cubeY + 4, "DRS", WHITE, drs_bg_color, 2); 

    // PIT
    LCD_draw_rectangle(cubeW + 1, cubeY, cubeW, cubeH, WHITE);
    uint16_t pit_bg_color = BLACK;
    if (pit_active_param) {
        pit_bg_color = GREEN;
        LCD_fill_rectangle(cubeW + 1, cubeY, cubeW, cubeH, pit_bg_color);
    }
    // Draw text AFTER fill to ensure contrast
    LCD_draw_string(cubeW + 36, cubeY + 4, "PIT", WHITE, pit_bg_color, 2);

    // TEMP
    LCD_draw_rectangle(2 * cubeW + 2, cubeY, cubeW, cubeH, WHITE);
    uint16_t temp_bg_color = BLACK;
    if (temp_alarm) {
        temp_bg_color = RED;
        LCD_fill_rectangle(2 * cubeY + 2, cubeY, cubeW, cubeH, temp_bg_color);
    }
    // Draw text AFTER fill to ensure contrast
    LCD_draw_string(2 * cubeW + 24, cubeY + 4, "TEMP", WHITE, temp_bg_color, 2);

    HAL_Display_Present();
}

// --- Main loop ---
void tft_test(void) {
    printf("Initializing display...\n");
    
    HAL_GPIO_Init();
    HAL_Display_Init();
    HAL_SPI_Init();

    buttons_init();
    clutch_Init();
    rotary_Init(10);
    CAN_Init();

    buttons_registerCallback(0, test_callback_btn1);
    buttons_registerCallback(1, test_callback_btn2);
    buttons_registerCallback(2, test_callback_btn3);
    buttons_registerCallback(3, test_callback_btn4);

    // Variables for live data acquisition
    float clutch_percentage = 0.0f;
    int rotary_pos = 0;
    int t1 = 50, t2 = 52;
    ECUStatus_t ecu = {0};
    int running = 1;

    printf("Simulation running. Use keys 1, 2, 3, 4 for control.\n");

    while (running) {
        // We rely on the core SDL_PollEvent in local_sdl_keyboard_handler 
        // to keep the thread alive and responsive to input, without explicit delay.
        local_sdl_keyboard_handler(&running);
        
        buttons_update();
        
        // --- LIVE DATA ACQUISITION (This part may be slow due to VM/driver issues) ---
        clutch_percentage = clutch_GetPercentage();
        rotary_pos = (int)rotary_GetPosition();

        // Check if new ECU data is available and update temperatures
        if (CAN_ReceiveECUStatus(&ecu) == 1) {
            t1 = ecu.temp1;
            t2 = ecu.temp2;
        }

        // UI Update (Runs every loop cycle)
        display_update_status_tft(clutch_percentage, rotary_pos, 
                                  t1, t2,
                                  current_gear, pit_active, drs_active, test_msg);
        
        // No explicit delays in this test file to maximize update rate.
        HAL_DelayMs(16);
    }
    
    printf("Simulation ended.\n");
}