/**
 * @file buttons.h
 * @brief Driver interface for digital button input handling with debounce and callbacks.
 *
 * @details
 * This module provides a software abstraction for reading multiple buttons,
 * applying debounce filtering, and triggering user-defined callbacks upon
 * stable state changes. It relies on the HAL (Hardware Abstraction Layer)
 * to read raw button inputs.
 *
 * The driver supports:
 * - Configurable debounce threshold.
 * - Retrieval of stable button states as a bitmask.
 * - Registration of per-button callback functions.
 * - Direct access to raw (non-debounced) input values
 *
 * @note
 * The function `buttons_update()` must be called periodically (e.g., in the main loop)
 * to ensure proper debouncing and event detection.
 */
#ifndef BUTTONS_H 
#define BUTTONS_H 

// --- INCLUDES ---
#include <stdint.h>  // Provides standard integer types like uint8_t.
#include <stdbool.h> // Provides the boolean type `bool` (true/false).


/*--------------------------ENUMERATIONS-----------------------------------*/

/**
 * @brief Logical identifiers for each button.
 *
 * @details
 * Each enumerator represents a button connected to the system.
 * Used for indexing callback arrays and bit positions in state bitmasks.
 */
typedef enum {
    BTN_1,          /**< Button 1. */
    BTN_2,          /**< Button 2. */
    BTN_3,          /**< Button 3. */
    BTN_4,          /**< Button 4. */
    NUM_BUTTONS     /**< Total number of buttons. */
} ButtonId_t;

/*--------------------------DEFINITIONS-----------------------------------*/

/** @brief Number of consecutive identical readings required for a stable state change. */
#define DEBOUNCE_COUNT 5  

/*--------------------------DATA TYPES-----------------------------------*/

/**
 * @brief Type definition for button state change callbacks.
 *
 * @details
 * A callback function is executed when the corresponding button’s
 * stable (debounced) state changes.
 *
 * @param[in] pressed `true` if the button is pressed, `false` if released.
 */
typedef void (*ButtonCallback_t)(bool pressed);


/*--------------------------PUBLIC API FUNCTIONS-----------------------------------*/

/**
 * @brief Initializes the button driver and underlying HAL.
 *
 * @details
 * Resets internal states and prepares the driver for operation.
 * Must be called once before using other button functions.
 */
void buttons_init (void);

/**
 * @brief Periodically updates button states and debounces inputs.
 *
 * @details
 * Reads raw button states via the HAL and processes each button to
 * determine stable transitions. Must be called frequently in the main loop.
 */
void buttons_update (void);

/**
 * @brief Returns the current stable (debounced) state of all buttons.
 *
 * @details
 * The returned bitmask contains one bit per button:
 * - Bit = 1 → Button pressed
 * - Bit = 0 → Button released
 *
 * @return uint8_t Bitmask of stable button states.
 */
uint8_t buttons_getStable(void);

/**
 * @brief Reads and returns the instantaneous (raw) state of all buttons.
 *
 * @details
 * Reads each button directly from the HAL (e.g., via ::HAL_GPIO_Read).
 * No debounce filtering is applied.
 *
 * @return uint8_t Bitmask of raw button states.
 */
uint8_t buttons_getRaw(void);

/**
 * @brief Registers a callback for a specific button.
 *
 * @details
 * Associates a user-defined function to be called whenever the given
 * button’s stable state changes. Pass `NULL` to unregister a callback.
 *
 * @param[in] buttonId Index of the button (0 to NUM_BUTTONS–1).
 * @param[in] callback Pointer to callback function (type ::ButtonCallback_t).
 */
void buttons_registerCallback(uint8_t buttonId, ButtonCallback_t callback);

#endif /* BUTTONS_H */
