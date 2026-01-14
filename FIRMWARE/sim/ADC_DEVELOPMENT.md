@page ADC_DEVELOPMENT ADC Subsystem — Step-by-Step Development Guide
@tableofcontents
@brief Detailed documentation of the ADC subsystem, covering its layered design, driver implementation, and usage examples.

---

# Subsystem ADC - Step-by-Step Guide


## Layered Architecture

The ADC system is divided in the next layer:

1. **HAL (`hal\hal_adc*`)`:**
   Manage the hardware layer (s32k) or create the simulated environment (host PC)

2. **Drivers:**
   Provides signal processing and scaling functions for specific peripherals:  
   - `clutch.c` / `clutch.h`  
   - `rotary_switch.c` / `rotary_switch.h`

3. **Test (`test\adc_test.c`):**
   Contains all high level code for testing the drivers and hal

4. **adc_data.csv (`test\adc_data.csv`):**
    Stores simulated ADC input values.  
   Each column represents one ADC channel, separated by commas.

---

## HAL APIs (`hal\adc.h`)

| Function | Description |
|-----------|--------------|
| `void hal_adc_init(void)` | Initializes and configures the ADC peripheral (on S32K) or sets up the simulation environment (on host PC). |
| `uint16_t hal_adc_read(uint8_t channel)` | Returns a 12-bit integer corresponding to the current reading from a specific ADC channel. |


> 💡 **Host Mode:**  
> When running on *host PC*, `hal_adc_read()` retrieves values from `adc_data.csv` in the `/test` folder.  
> If no CSV file is found, the function generates random values.

---

## Driver APIs

### ⚙️ Clutch Driver (`clutch.h`)

| Function | Description |
|-----------|--------------|
| `void clutch_Init(void)` | Initializes all internal variables and sets a default calibration (min = 0, max = 4095). |
| `void clutch_SetCalibration(uint16_t min, uint16_t max)` | Sets a custom calibration range for the clutch analog input. |
| `uint16_t clutch_GetRawValue(void)` | Reads and returns the raw ADC value. |
| `float clutch_GetPercentage(void)` | Converts the raw ADC value into a percentage (0–100%). |

----

### 🔄 Rotary Switch Driver (`rotary_switch.h`)

| Function | Description |
|-----------|--------------|
| `void rotary_Init(uint8_t num_positions)` | Initializes the rotary switch and sets the total number of positions. Default calibration: min = 0, max = 4095. |
| `void rotary_SetCalibration(uint16_t min, uint16_t max)` | Sets calibration range for the rotary switch input. |
| `uint16_t rotary_GetRawValue(void)` | Reads and returns the raw ADC value from the rotary switch channel. |
| `uint8_t rotary_GetPosition(void)` | Converts the raw ADC value into a discrete position (0–`num_positions`–1). |


> ⚠️ In both drivers (`clutch.h` and `rotary_switch.h`), constants such as  
> `CLUTCH_ADC_CHANNEL` and `ROTARY_ADC_CHANNEL` define which ADC channel is used.

---

## Example of Use

### Clutch

```c
#include "../hal/hal_adc.h"
#include "../drivers/clutch.h"

#include <stdio.h>

void main(void) {

    hal_adc_init(); //Initialization of the ADC
    
    clutch_Init(); //Initialization of the clutch
    clutch_SetCalibration(400,4000); // Putting min and max for calibration (optional)

    while(1) {

        uint16_t raw_c=clutch_GetRawValue(); //read and update the raw value (optional)
        float clutch=clutch_GetPercentage(); // obtaing the porcentage value

        printf("Clutch: %.1f%% | Raw value: %u\n", clutch, raw_c);
    }
}
```
>⚙️ **Usage Notes:**
> It is important to initializates the hal and the clutch init
> Calibration is an optional function
> You can use `clutch_GetRawValue();` and `clutch_GetPercentage();` independant. But If you're going to use both function at the same time, always put first the *GetRawValue*.

---

### Rotary Switch

```c
#include "../hal/hal_adc.h"
#include "../drivers/rotary_switch.h"

#include <stdio.h>

void adc_test(void) {

    hal_adc_init(); //Initialization of the ADC
    
    rotary_Init(16); //Initialization of the rotary switch putting the #of position 
    //rotary_SetCalibration(400,400) // You can put min an max for calibration (optional)

    while(1) {

        uint16_t raw_r=rotary_GetRawValue(); // read raw value of rotary switch (optional)
        uint8_t position=rotary_GetPosition(); //obtain the position

        printf("Position: %u | Raw value: %u\n", position, raw_r);

    }
}
```
>⚙️ **Usage Notes:**
> It is important to initializates the hal and the rotary init with the number of positions
> Calibration is an optional function
> You can use `Rotary_GetRawValue();` and `Rotary_GetPosition();` independant. But If you're going to use both function at the same time, always put first the *GetRawValue*.
