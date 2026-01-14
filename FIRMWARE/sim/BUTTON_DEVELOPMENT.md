# Subsystem Buttons  — Step-by-Step Guide

## Layered Architecture
The system of the buttons is divided in 3 layers
1. **Test** (test/buttons_test.c) - Contains all the useful High level logic to test functionality

2. **Driver** (driver/buttons.c) - Have all the functions for using the 

3. **HAL** (hal/.../hal_buttons_*) - Manage the hadware layer or create simulated registar for digital Inputs

## HAL APIs (hal_buttons.h)
- `void hal_buttons_init(void);` 
    Configuration of the ports or set up of the simulation

- `uint8_t hal_buttons_read(void);`
    Creates and return a binary state register of all the buttons

>Note: The actual hal for simulation is only functional for the Windows OS

## Driver APIs (buttons.h)
- `void buttons_init (void);`
    Initialize the HAL and the internal variables of the driver.

- `void buttons_update (void);`
    Update the value of the reading register and process all button's singals

- `uint8_t buttons_getStable(void);`
    Return the stable status of all the buttons after the debouncing phase

- `void buttons_registerCallback(uint8_t buttonId, ButtonCallback_t callback);`
    Set any defined function as a callback for a button

> One important thing, is that the driver apply a filter for the mechanical Debouncing. You can change the value of `DEBOUNCE_COUNT` to make it more or less agressive.
>
>Take care of this value: Too **High** reading will be slow

## Typical example of Use
The actual driver permits two ways of using the buttons by *callback* or *if structures*

```c
//Include buttons.h driver
#include "../drivers/buttons.h"

/*Creation of the callbacks function for each button*/
void callback_Btn(bool state_btn) { 
    if (state_btn){
        //Execute Action when the button is pressed 
    }
    else {
        //Execute Action when the button is realesed
    }  
}

void main (void) {

    //Initialization of the buttons 
    buttons_init();

    //Set the callbacks
    buttons_registerCallback(0,callback_Btn); // Argument: ButtonID, callback function

    while (1) {

        //Update the value of the buttons
        buttons_update();

        delay(100); // Include any delay 
    }
}
```
The previus example make use of the callbacks. But if you don't want to use the callbacks, you can use the function `buttons_getStable();` and make and structure of `if`

```c
//Include buttons.h driver
#include "../drivers/buttons.h"

void main (void) {

    //Initialization of the buttons 
    buttons_init();

    while (1) {

        //Update the value of the buttons
        buttons_update();

        uint8_t state_buttons=buttons_getStable();

        //see the state of the button by masking and if structures
        if (state_buttons & 0x01) {
            //executes Action when it is press 1st button
        }else{
            //executes Action when it is realesed 1st button
        }

        delay(100); // Include any delay 
    }
}
```
## Build and Compilation (Host Computer)
1. Insert your code in the `app/app_main.c` or you can create a new file on the following folder `app/` or `test/`

2. On `main.c` make the call to the `app_mainc` or the `.c` file that you create
>Remember well the path

3. On a terminal execute the next comand: `make sim` to create an executable file

4. Run the executable file with `start build/app_sim.exe`

> There is a file called `test/button_test.c` that you can use to test the buttons. In order to used call this file into the `main.c` and follow al the step descrived previusly.
