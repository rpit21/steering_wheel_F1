
/**
 * @file can.h
 * @brief High-level CAN communication interface for the Steering Wheel ECU.
 *
 * @details
 * This module provides a simplified API for sending and receiving CAN messages
 * between the Steering Wheel ECU and other control units (e.g., Main ECU).
 *
 * It abstracts the lower-level hardware access handled by the HAL (Hardware Abstraction Layer),
 * exposing only application-level data such as button states, rotary switch position,
 * clutch percentage, and ECU status feedback (temperatures, gear, LEDs, etc.).
 *
 * @note
 * Before calling any send or receive function, `CAN_Init()` must be executed
 * to properly configure the CAN interface.
 */

#ifndef CAN_H
#define CAN_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Steering Wheel status message structure.
 *
 * @details
 * This structure represents the data periodically transmitted from the Steering Wheel ECU
 * over the CAN network. It contains:
 * - Button states (4 bits)
 * - Rotary switch position (0–15)
 * - Clutch pedal position (0–100%)
 */
typedef struct {
    uint8_t button_state;     /**< Bitfield representing up to 4 buttons (bits 0–3). */
    uint8_t rotary_position;  /**< Rotary switch position: range 0–15. */
    uint8_t clutch_value;     /**< Clutch position: percentage (0–100%). */
} SteeringWheelStatus_t;

/**
 * @brief ECU status message structure.
 *
 * @details
 * This structure holds decoded feedback received from the main ECU,
 * containing sensor and system information such as temperatures, LEDs, and gear state.
 */
typedef struct {
    float temp1;                /**< Temperature sensor 1 (°C). */
    float temp2;                /**< Temperature sensor 2 (°C). */
    bool pit_limiter_active;    /**< Pit limiter active flag. */
    uint8_t drs_status;         /**< DRS status (0 = off, 1 = active). */
    bool led_temp;              /**< LED indicator for temperature warning. */
    bool led_pit;               /**< LED indicator for pit limiter. */
    uint8_t gear_actual;        /**< Current gear value. */
    uint8_t clutch_feedback;    /**< Clutch feedback percentage (0–100%). */
    uint8_t rotary_feedback;    /**< Rotary switch feedback position (0–15). */
} ECUStatus_t;

/**
 * @brief Initializes the CAN communication interface.
 *
 * @details
 * Configures the CAN channel by invoking the HAL initialization routine.
 * On PC simulation, this may connect to a virtual CAN interface (e.g., `vcan0`).
 */
void CAN_Init(void);

/**
 * @brief Sends the Steering Wheel status frame over the CAN bus.
 *
 * @details
 * Encodes the steering wheel state (buttons, rotary, clutch) into a CAN message
 * and transmits it using a predefined message ID.
 *
 * @param[in] status Pointer to a ::SteeringWheelStatus_t structure containing the data to send.
 */
void CAN_SendSteeringStatus(const SteeringWheelStatus_t *status);

/**
 * @brief Receives and decodes ECU status messages from the CAN bus.
 *
 * @details
 * Checks for new incoming frames, validates their ID and payload length,
 * and decodes ECU-related data according to the DBC format.
 *
 * @param[out] ecu_status Pointer to a ::ECUStatus_t structure where decoded data will be stored.
 * @return int Returns 1 if a valid ECU message was received and decoded, 0 otherwise.
 */
int  CAN_ReceiveECUStatus(ECUStatus_t *ecu_status);

#endif /* CAN_H */