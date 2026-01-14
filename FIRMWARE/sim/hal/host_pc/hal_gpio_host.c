/**
 * @file hal_gpio_host.c
 * @brief Host (PC) simulation of the GPIO Hardware Abstraction Layer (HAL).
 *
 * @details
 * This module simulates GPIO functionality for PC-based testing and visualization:
 * - Button inputs are simulated via SDL keyboard events.
 * - LED and pin outputs are logged to the console.
 * - GPIO pin states are tracked internally in an array.
 *
 * This allows the same application code to run on both simulation and embedded hardware.
 */

#include "hal_gpio.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

/*-------------------------- PRIVATE VARIABLES --------------------------*/

/** Internal state for each GPIO pin (0 = LOW, 1 = HIGH). */
static uint8_t gpio_state[GPIO_COUNT] = {0};

/** Flag indicating whether GPIO has been initialized. */
static uint8_t gpio_initialized = 0;

static uint8_t gpio_toggle_latch[2] = {0, 0};
// index 0 → BTN1
// index 1 → BTN2

/*-------------------------- HELPER FUNCTIONS --------------------------*/

/**
 * @brief Returns the name of a GPIO pin as a string.
 */
static const char* pinName(GPIO_Pin_t pin) {
    switch (pin) {
        case GPIO_TFT_CS:  return "TFT_CS";
        case GPIO_TFT_DC:  return "TFT_DC";
        case GPIO_TFT_RST: return "TFT_RST";
        case GPIO_LED_S1:  return "LED_STATUS1";
        case GPIO_LED_S2:  return "LED_STATUS2";
        case GPIO_BTN_1:   return "BTN_1";
        case GPIO_BTN_2:   return "BTN_2";
        case GPIO_BTN_3:   return "BTN_3";
        case GPIO_BTN_4:   return "BTN_4";
        default:           return "UNKNOWN";
    }
}

/**
 * @brief Initializes SDL event subsystem for keyboard input simulation.
 */
void HAL_Keyboard_Init(void) {
    if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        exit(1);
    }
    printf("[HAL_GPIO_HOST] SDL ready for input\n");
}

/**
 * @brief Simulates a button press or release in the GPIO state array.
 *
 * @param pin GPIO pin identifier (must be one of the button pins)
 * @param pressed 1 = pressed, 0 = released
 */
void HAL_GPIO_SimulateButton(GPIO_Pin_t pin, uint8_t pressed) {
    if (pin >= GPIO_BTN_1 && pin <= GPIO_BTN_4) {
        gpio_state[pin] = pressed;
        // Optional debug:
        // printf("[HAL_GPIO_HOST] SimButton %d -> %s\n",
        //        pin - GPIO_BTN_1 + 1, pressed ? "PRESSED" : "RELEASED");
    }
}

/*-------------------------- PUBLIC API --------------------------*/

/**
 * @brief Initializes the GPIO HAL (simulation mode).
 *
 * Sets all pins to LOW and initializes the SDL keyboard subsystem.
 */
void HAL_GPIO_Init(void) {
    if (!gpio_initialized) {
        for (int i = 0; i < GPIO_COUNT; i++) gpio_state[i] = 0;
        gpio_initialized = 1;
        printf("[HAL_GPIO_HOST] Initialized (simulation mode)\n");
    }

    HAL_Keyboard_Init();
}

/**
 * @brief Writes a value to a GPIO pin.
 *
 * @param pin GPIO pin identifier
 * @param value 0 = LOW, 1 = HIGH
 */
void HAL_GPIO_Write(GPIO_Pin_t pin, uint8_t value) {
    if (pin >= GPIO_COUNT) return;
    gpio_state[pin] = value;
    //printf("[HAL_GPIO_HOST] Pin %d = %s\n", pin, value ? "HIGH" : "LOW");
}

/**
 * @brief Toggles a GPIO pin state.
 *
 * @param pin GPIO pin identifier
 */
void HAL_GPIO_Toggle(GPIO_Pin_t pin) {
    if (pin >= GPIO_COUNT) return;
    gpio_state[pin] ^= 1;
    //printf("[HAL_GPIO_HOST] Pin %d toggled -> %d\n", pin, gpio_state[pin]);
    // printf("[HAL_GPIO_HOST] %s toggled\n", pinName(pin));
}

/**
 * @brief Reads the current state of a GPIO pin.
 *
 * @param pin GPIO pin identifier
 * @return 0 = LOW, 1 = HIGH
 */
uint8_t HAL_GPIO_Read(GPIO_Pin_t pin) {
    if (pin >= GPIO_COUNT) return 0;
    return gpio_state[pin];
}

/**
 * @brief Maps SDL keyboard events to simulated GPIO button presses.
 *
 * @param keysym SDL key symbol
 * @param is_down 1 = key pressed, 0 = key released
 */
void HAL_GPIO_on_key(int keysym, int is_down) {
    switch (keysym) {

        // --------- TOGGLE Buttons (new hardware) ---------
        case SDLK_1: 
            if (is_down) {
                    gpio_toggle_latch[0] ^= 1;       // toogle 0↔1
                    gpio_state[GPIO_BTN_1] = gpio_toggle_latch[0];
                }
            break;

        case SDLK_2:
            if (is_down) {
                    gpio_toggle_latch[1] ^= 1;
                    gpio_state[GPIO_BTN_2] = gpio_toggle_latch[1];
                }
            break;

        // --------- Normal buttons ---------
        case SDLK_3: HAL_GPIO_SimulateButton(GPIO_BTN_3, is_down); break;
        case SDLK_4: HAL_GPIO_SimulateButton(GPIO_BTN_4, is_down); break;
        // Optional: add other keys for control
    }
}
