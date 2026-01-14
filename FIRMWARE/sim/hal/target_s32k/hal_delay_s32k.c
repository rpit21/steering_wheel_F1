// hal_delay_target.c
/*

#include "hal_delay.h"
#include "device_registers.h"

void HAL_DelayMs(uint32_t ms) {
    volatile uint32_t count;
    for (uint32_t i = 0; i < ms; i++) {
        count = 8000; // aprox. 1 ms si F_CPU=8 MHz (ajustar)
        while (count--) __NOP();
    }
}

void HAL_DelayUs(uint32_t us) {
    volatile uint32_t count = us * 8; // aprox. 1 µs
    while (count--) __NOP();
}

*/