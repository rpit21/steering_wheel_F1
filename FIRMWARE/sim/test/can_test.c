
/**
 * @file can_test.c
 * @brief Functional test for the CAN driver module.
 *
 * @details
 * This test verifies the correct operation of the CAN communication layer
 * between the Steering Wheel ECU and the main ECU. It performs the following:
 * - Initializes the CAN interface using the HAL.
 * - Periodically sends a simulated Steering Wheel Status frame.
 * - Continuously listens for ECU Status messages.
 * - Prints the decoded temperature values on the console.
 *
 * The test runs in an infinite loop to simulate real-time communication
 * (e.g., during bench testing or PC simulation with SocketCAN / vcan0).
 *
 * @note
 * This test requires a virtual CAN interface (`vcan0`) or compatible CAN bus setup.
 * To stop the test, manually terminate the program (Ctrl + C).
 */
#include "../drivers/can.h"
#include "hal_delay.h"
#include <stdio.h>

/**
 * @brief Executes the CAN driver functional test.
 *
 * @details
 * Initializes the CAN layer and sends continuous status frames representing
 * a fixed steering wheel input (buttons pressed, rotary position, clutch).
 *
 * At each cycle, it attempts to receive ECU status frames and print
 * the decoded temperatures.
 *
 * @return int Always returns 0 (function loops indefinitely).
 */
int can_test(void) {
    
    /* Initialize CAN communication channel */
    CAN_Init();

    /* Declare structures for sending and receiving messages */
    SteeringWheelStatus_t status = {0};
    ECUStatus_t ecu = {0};

    while (1) {
        /* Simulate steering wheel inputs */
        status.button_state = 1;   // All 4 buttons pressed (0b0001)
        status.rotary_position = 2; // Rotary switch position 2
        status.clutch_value = 97;   // Clutch 97%

        /* Transmit status frame over CAN */
        CAN_SendSteeringStatus(&status);

        /* Attempt to receive ECU status frame and decode temperatures */
        if (CAN_ReceiveECUStatus(&ecu) == 1) {
            printf("Gear=%u  Pit=%d  DRS=%u  T1=%.1f°C  T2=%.1f°C\n",
                ecu.gear_actual,
                ecu.pit_limiter_active,
                ecu.drs_status,
                ecu.temp1,
                ecu.temp2);
            printf ("LED_P=%u LED_T=%u Clutch=%u  Rotary=%u\n",
                ecu.led_pit,
                ecu.led_temp,
                ecu.clutch_feedback,
                ecu.rotary_feedback
            );
        }
        
        /* Wait 100 ms between transmissions */
        HAL_DelayMs(100);
    }

    return 0;
}