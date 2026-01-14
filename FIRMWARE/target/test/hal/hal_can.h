/**
 * @file hal_can.h
 * @brief Hardware Abstraction Layer (HAL) interface for Controller Area Network (CAN) communication.
 *
 * @details
 * This module provides a generic API for CAN communication across both
 * real hardware targets (e.g., S32K microcontrollers) and simulation
 * environments (e.g., Linux/WSL using SocketCAN).
 *
 * The abstraction allows higher-level modules such as ::canbus.c to transmit
 * and receive CAN frames without being aware of the underlying platform.
 *
 * Supported operations:
 * - CAN interface initialization.
 * - Frame transmission and reception.
 * - Graceful interface shutdown and cleanup.
 *
 * @note
 * The specific implementation of this interface depends on the build target:
 * - Hardware target: Uses MCU CAN peripherals (e.g., FlexCAN).
 * - Host PC simulation: Uses SocketCAN or equivalent interface.
 */

#ifndef HAL_CAN_H
#define HAL_CAN_H

// --- INCLUDES ---
#include <stdint.h> /**< Provides fixed-width integer types like uint8_t and uint32_t. */

/*--------------------------PUBLIC API FUNCTIONS-----------------------------------*/

/**
 * @brief Initializes the CAN hardware or simulation interface.
 *
 * @details
 * Configures and prepares the CAN controller or simulated interface for communication.
 * On embedded hardware, this includes setting bit timing, enabling clocks, and configuring pins.
 * On host systems (e.g., Linux with SocketCAN), it opens the specified virtual or physical interface.
 *
 * @param[in] interface_name String specifying the CAN interface name.
 *            - Example (simulation): `"vcan0"`
 *            - Example (hardware): `"CAN0"`, `"FlexCAN_0"`
 *
 * @return int
 * @retval 0   Initialization successful.
 * @retval <0  Initialization failed (e.g., interface not found or already in use).
 */
int hal_can_init(const char* interface_name);

/**
 * @brief Sends a CAN frame over the active CAN interface.
 *
 * @details
 * Transmits a CAN frame identified by its message ID and data payload.
 * The function may block briefly or queue the frame for asynchronous transmission,
 * depending on the underlying HAL implementation.
 *
 * @param[in] id    CAN message identifier (standard 11-bit or extended 29-bit).
 * @param[in] data  Pointer to an array containing the message payload.
 * @param[in] len   Length of the payload in bytes (Data Length Code, typically 0–8).
 *
 * @return int
 * @retval 0   Frame successfully transmitted or queued.
 * @retval <0  Transmission failed (e.g., interface error or full buffer).
 */
int hal_can_send(uint32_t id, const uint8_t* data, uint8_t len);

/**
 * @brief Receives a CAN frame from the bus (non-blocking).
 *
 * @details
 * Attempts to retrieve one CAN frame from the interface’s receive buffer.
 * If no frame is currently available, the function returns immediately.
 *
 * @param[out] id   Pointer to a variable where the received CAN ID will be stored.
 * @param[out] data Pointer to a buffer where received payload bytes will be copied.
 * @param[out] len  Pointer to a variable where the received payload length (DLC) will be written.
 *
 * @return int
 * @retval 1   A frame was successfully received.
 * @retval 0   No frame available (non-blocking mode).
 * @retval <0  Receive error occurred.
 */
int hal_can_receive(uint32_t* id, uint8_t* data, uint8_t* len);

/**
 * @brief Shuts down the CAN interface and releases all associated resources.
 *
 * @details
 * Disables or closes the CAN interface. On hardware platforms, this may disable
 * peripheral clocks or interrupts; on host-based simulations, it closes sockets
 * and frees allocated resources.
 *
 * Should be called once when CAN communication is no longer required.
 */
void hal_can_shutdown(void);

#endif /* HAL_CAN_H */
