/**
 * @file buttons.c
 * @brief Implementation of the button driver with debounce and callback mechanism.
 *
 * @details
 * This file contains the logic for detecting button presses using debounce filtering
 * and optional callbacks. It uses the HAL layer (GPIO-based) for raw input acquisition
 *
 * The debouncing algorithm works as follows:
 * - If a raw input differs from its last stable state, a counter increments.
 * - Once the counter exceeds the threshold (`DEBOUNCE_COUNT`), the change is accepted.
 * - The driver updates the stable state and triggers any registered callback.
 *
 * @note
 * The function `buttons_update()` must be called periodically in the main loop.
 */


#include "buttons.h"
#include "hal_gpio.h"           
#include <stddef.h>            /* For NULL */

/*--------------------------PRIVATE VARIABLES-----------------------------------*/
// These variables are only accessible within this file (buttons.c).

/** @brief Debounced state of all buttons (bitmask). */
static uint8_t stableState = 0;

/** @brief Latest raw button reading (bitmask). */
static uint8_t rawState = 0;

/** @brief Debounce counter for each button. */
static uint8_t counter[NUM_BUTTONS] = {0}; 

/** @brief Registered callback functions for each button. */
static ButtonCallback_t buttonCallbacks[NUM_BUTTONS];



/*--------------------------PRIVATE HELPER FUNCTIONS-----------------------------------*/

/**
 * @brief Processes debounce logic for a single button.
 *
 * @param[in] id  Button index (0–NUM_BUTTONS–1).
 * @param[in] raw Current raw bitfield read from HAL.
 */
static void button_process(uint8_t id, uint8_t raw) {

    /* Bitmask for the current button 'id' */
    // E.g., id=0 -> mask=0x01, id=1 -> mask=0x02, id=2 -> mask=0x04, etc.
    uint8_t mask = (1 << id);

    /* Current raw state (1=pressed) */
    bool bit_raw = (raw & mask); // Use bool for clarity (true if pressed, false if released)

    /* Extract the current stable state of the *specific* button 'id'*/
    bool bit_stable = (stableState & mask);

    // --- Debounce Logic ---
    /* Detect change and start debounce process */
    if (bit_raw != bit_stable) {

        // If there's a difference increment the counter for this button.
        counter[id]++;

        // Check if the counter has reached the debounce threshold.
        if (counter[id] >= DEBOUNCE_COUNT) {

            /* The change is considered stable. Update the stableState*/
            if (bit_raw) {

                // If the raw input is now high (pressed), set the corresponding bit in stableState.
                stableState |= mask; // |= (Bitwise OR assignment) sets the bit.

            } else {

                // If the raw input is now low (released), clear the corresponding bit in stableState.
                stableState &= ~mask; // &= (Bitwise AND assignment) with the inverted mask clears the bit.
            }

            counter[id] = 0; // Reset the counter for this button after updating the state.

            /*--- Callback Management ---*/

            /* Trigger callback if registered */
            if (buttonCallbacks[id] != NULL) {
                // If a callback exists, call it, passing the *new stable state* of the button
                // (which is the same as bit_raw that caused the update).
                buttonCallbacks[id](bit_raw); // Pass true if pressed, false if released.
            }
        }
    } else {

        // If the raw input state matches the stable state, reset the counter.
        // This ensures that brief noise doesn't trigger a state change.
        counter[id] = 0;
    }
}


/*--------------------------PUBLIC API FUNCTIONS-----------------------------------*/


void buttons_init(void) {

    // Call the HAL initialization function, which sets up the underlying hardware or simulation input.
    //hal_buttons_init();

    // Initialize all driver-level states to zero/NULL.
    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttonCallbacks[i] = NULL; // Ensure no callbacks are registered initially.
        counter[i] = 0;         // Reset all debounce counters.
    }

    stableState = 0; // Ensure initial stable state is all released.
    rawState=0; // Ensure the inital raw state is released
}


void buttons_update() {

    // Read the current raw state of all buttons from the HAL.
    uint8_t raw = buttons_getRaw();

    // Optional: Invert raw bits if buttons are wired in pull-up configuration
    // (where pressed = low voltage = 0, released = high voltage = 1).
    // raw = ~raw; // Uncomment this line if using pull-up resistors.

    // Iterate through each button (0 to NUM_BUTTONS-1).
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        // Call the helper function to process debouncing for button 'i'.
        button_process(i, raw);
    }
}


uint8_t buttons_getStable(void) {

    // Return the stableState bitmask.
    return stableState;
}

uint8_t buttons_getRaw(void) {

     /* Read each button GPIO and build bitmask */
     
    if (HAL_GPIO_Read(GPIO_BTN_1)){
        rawState |= (1 << BTN_1);
    }else{
        rawState &= ~(1 << BTN_1);
    }

    if (HAL_GPIO_Read(GPIO_BTN_2)){
        rawState |= (1 << BTN_2);
    }else{
        rawState &= ~(1 << BTN_2);
    }

    if (HAL_GPIO_Read(GPIO_BTN_3)){
        rawState |= (1 << BTN_3);
    }else{
        rawState &= ~(1 << BTN_3);
    }

    if (HAL_GPIO_Read(GPIO_BTN_4)){
        rawState |= (1 << BTN_4);
    }else{
        rawState &= ~(1 << BTN_4);
    }

    return rawState;
}


void buttons_registerCallback(uint8_t buttonId, ButtonCallback_t cb) {
    
    // Check for valid buttonId to prevent out-of-bounds array access.
    if (buttonId < NUM_BUTTONS) {
        // Store the function pointer in the corresponding slot.
        buttonCallbacks[buttonId] = cb;
    }
}
