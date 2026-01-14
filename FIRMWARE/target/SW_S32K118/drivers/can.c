
/**
 * @file can.c
 * @brief Implementation of the high-level CAN communication driver.
 *
 * This file implements the CAN interface logic for the Steering Wheel ECU.
 * It handles message formatting, transmission, and decoding of received frames.
 *
 * The driver interacts with the hardware through the HAL (Hardware Abstraction Layer),
 * ensuring portability between PC simulation and the target MCU (e.g., NXP S32K118).
 */

#include "can.h"
#include "../hal/hal_uart.h"
#include "../hal/hal_can.h"
#include <string.h>
#include <stdio.h>

/* --- CAN Message Identifiers (11-bit standard IDs) --- */
#define CAN_ID_STEERING_STATUS  0x101   /**< Message ID for Steering Wheel status frames. */
#define CAN_ID_ECU_STATUS       0x00000201   /**< Message ID for ECU status frames. */


void CAN_Init(void) {
    /* Initialize CAN channel through the HAL layer (VCAN0). */
    hal_can_init("can0");
    HAL_UART_Printf("[CAN] INIT DONE\r\n");
}


void CAN_SendSteeringStatus(const SteeringWheelStatus_t *status) {
    uint8_t payload[8] = {0};

    /* Encode the steering wheel data into payload bytes */
    payload[0] = status->button_state &0x0F;       // Byte 0 -> Bits 0–3: button states
    payload[1] = status->rotary_position &0x0F;    // Vyte 1 -> Bits 0–3: rotary position
    payload[2] = status->clutch_value;             // Byte 2: clutch percentage (0–100%)

    // Remaining bytes are reserved and remain 0
    hal_can_send(CAN_ID_STEERING_STATUS, payload, 8);
}


int CAN_ReceiveECUStatus(ECUStatus_t *ecu_status) {
    uint8_t data[8];
    uint8_t len;
    uint32_t id;

    int ret = hal_can_receive(&id, data, &len);

    if (ret <= 0) return ret; /* 0 = no data available */

    //if (ret > 0) {
    //    printf("RX Frame: ID=0x%03X, DLC=%d\n", id, len);
    //}
    
    /* Mask off EFF/RTR/ERR bits and check for the expected ID */
    if ((id & 0x1FFFFFFF) == CAN_ID_ECU_STATUS) { 
        
        /* Decode temperature values (int16, big-endian format) (For little endian chage position)*/
        int16_t raw1 = (data[1] << 8) | data[0];
        int16_t raw2 = (data[3] << 8) | data[2];

        // Apply scaling factor and offset according to DBC
        ecu_status->temp1 = (raw1 * 0.1f) - 40.0f;
        ecu_status->temp2 = (raw2 * 0.1f) - 40.0f;

        /* Decode digital status flags and feedback signals */
        ecu_status->pit_limiter_active = (data[4] >> 0) & 0x01;
        ecu_status->drs_status         = (data[4] >> 1) & 0x01;
        ecu_status->led_pit            = (data[4] >> 6) & 0x01;
        ecu_status->led_temp           = (data[4] >> 7) & 0x01;
        ecu_status->gear_actual        = data[5];
        ecu_status->clutch_feedback    = data[6];
        ecu_status->rotary_feedback    = (data[7] & 0x0F);

        return 1; // Successfully decoded
    }
    return 0; // No valid frame received
}
