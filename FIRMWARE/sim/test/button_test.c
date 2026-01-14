/**
 * @file button_test.c
 * @brief Functional test for the button input driver.
 *
 * @details
 * This test validates the functionality of the button input system,
 * including hardware reading through the HAL layer and debouncing logic
 * implemented in the driver.
 *
 * It performs the following:
 * - Initializes the button module and the HAL.
 * - Registers callback functions for four independent buttons.
 * - Continuously updates the button state and monitors callbacks.
 *
 * Each callback prints a message to the console whenever a button changes state.
 * This test is primarily used for verifying:
 * - Hardware input reading.
 * - Software debounce behavior.
 * - Callback triggering reliability.
 *
 * @note
 * For PC simulation or host testing, ensure that `hal_buttons_read()` returns
 * emulated input states.  
 * On target hardware, connect buttons with proper pull-up or pull-down configuration.
 */

#define _DEFAULT_SOURCE /**< Required to expose `usleep()` on some glibc versions. */
#include "buttons.h"
#include "button_test.h"
#include "hal_gpio.h"
#include <stdio.h>

#include "TFT_LCD.h"
#include "hal_spi.h"
#include "hal_delay.h"
#include "hal_lcd.h"


/* ------------------------- Callback Implementations for Buttons ------------------------ */

/**
 * @brief Callback for Button #1.
 * @param[in] statebtn1 True if pressed, False if released.
 */
void cbBtn1(bool statebtn1) { 
    if (statebtn1){
        printf("Button #1: UP \n");
    }
    else {
        printf("Button #1: Realeased \n"); 
    }  
}

/**
 * @brief Callback for Button #2.
 * @param[in] statebtn2 True if pressed, False if released.
 */
void cbBtn2(bool statebtn2) { 
    if (statebtn2){
        printf("Button #2: DONW\n"); 
    }
    else{
        printf("Button #2: Realeased \n"); 
    }
}

/**
 * @brief Callback for Button #3.
 * @param[in] statebtn3 True if pressed, False if released.
 */
void cbBtn3(bool statebtn3) { 
    if (statebtn3){
        printf("Button #3: SPARE #1\n"); 
    }
    else{
      printf("Button #3: Realeased \n");  
    }
}

/**
 * @brief Callback for Button #4.
 * @param[in] statebtn4 True if pressed, False if released.
 */
void cbBtn4(bool statebtn4) { 
    if (statebtn4) {
        printf("Button #4: SPARE #2\n"); 
    }else{
        printf("Button #4: Realeased \n"); 
    }
}

/**
 * @brief Executes the button input functional test.
 *
 * @details
 * Initializes the button driver, registers the callbacks for each button,
 * and continuously monitors the input state in a loop.  
 * The HAL is queried for raw button states, while the driver provides
 * stable (debounced) readings.
 *
 * The test displays button transitions in real time through callback messages.
 *
 * @return void
 */
void button_test(void){

    /* ----------------------- Initialization Section ----------------------- */

    uint8_t raw_state_buttons;  /**< Raw value read directly from HAL (bitfield). */
    uint8_t state_buttons;      /**< Debounced and stable state from driver. */

    int running = 1;

    /* Initialization */
    printf("Starting button test....");
    HAL_GPIO_Init(); // always initializate the pins configutation
    buttons_init();  // inizializate all the internal variables of the buttons drivers

    /* Register button callbacks */
    buttons_registerCallback(0,cbBtn1);
    buttons_registerCallback(1,cbBtn2);
    buttons_registerCallback(2,cbBtn3);
    buttons_registerCallback(3,cbBtn4);
    /*
     if (display_init(128, 64, 4) != 0) {
        // display_init returns non-zero on error.
        return; // Exit the application if the display cannot be created.
    }
    */
    HAL_SPI_Init();
    HAL_Display_Init();


    /* ------------------------- Test Execution Loop ------------------------ */
    while (running) {
        
        HAL_Poll_Events(&running); // Read SDL + teclado

        //LCD_draw_string(12, 80, "SETUP:", WHITE, BLACK, 2);

        //Testing buttons

        /* Update button states and invoke callbacks if necessary */
        buttons_update();

        /* Optional: read and display raw and stable button states */
        raw_state_buttons=buttons_getRaw();
        printf("Raw Binary of buttons: %d\n",raw_state_buttons);

        state_buttons=buttons_getStable();
        printf("Stable val of buttons: %d\n",state_buttons);

        /* Delay between iterations (100 ms) */
        HAL_Display_Present();
        HAL_DelayMs(16);
    }

}