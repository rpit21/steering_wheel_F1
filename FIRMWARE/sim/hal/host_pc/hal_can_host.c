// hal_can.c
// Host PC (simulation) version of the Hardware Abstraction Layer (HAL) for CAN.
// Uses SocketCAN on Linux to send/receive CAN frames in a platform-independent way.

// --- DEFINES ---
#define _GNU_SOURCE // Needed to expose certain Linux/POSIX features (struct ifreq, etc.)

// --- INCLUDES ---
#include "hal_can.h"     // HAL function prototypes
#include <stdio.h>       // perror, printf, fprintf
#include <stdlib.h>      // atexit, general utilities
#include <string.h>      // memcpy, strncpy
#include <unistd.h>      // read, write, close
#include <errno.h>       // errno codes
#include <fcntl.h>       // fcntl flags

// Linux CAN/SocketCAN headers
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

// --- STATIC VARIABLES ---
// The CAN socket descriptor. Static to limit visibility to this file.
static int can_socket = -1;

// --- PUBLIC FUNCTIONS ---

/**
 * @brief Initializes the CAN interface using SocketCAN.
 * @param interface_name Name of the CAN interface (e.g., "vcan0").
 * @return 0 on success, negative error code on failure.
 *
 * @details
 * Creates a raw CAN socket, binds it to the requested interface,
 * and sets the socket to non-blocking mode.
 */
int hal_can_init(const char* interface_name) {
    struct sockaddr_can addr;
    struct ifreq ifr;

    // Create raw CAN socket
    can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_socket < 0) {
        perror("Error creating CAN socket");
        return -1;
    }

    // Copy interface name into ifreq structure
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    // Retrieve interface index
    if (ioctl(can_socket, SIOCGIFINDEX, &ifr) < 0) {
        perror("Error ioctl SIOCGIFINDEX");
        close(can_socket);
        can_socket = -1;
        return -2;
    }

    // Prepare CAN socket address structure
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Bind socket to interface
    if (bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Error bind CAN socket");
        close(can_socket);
        can_socket = -1;
        return -3;
    }

    // Set socket to non-blocking mode
    int flags = fcntl(can_socket, F_GETFL, 0);
    if (flags == -1) {
        perror("Error fcntl F_GETFL");
        close(can_socket);
        can_socket = -1;
        return -4;
    }
    if (fcntl(can_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Error fcntl F_SETFL O_NONBLOCK");
        close(can_socket);
        can_socket = -1;
        return -5;
    }

    printf("CAN Interface '%s' initialized.\n", interface_name);
    return 0;
}

/**
 * @brief Sends a single CAN frame.
 * @param id CAN identifier (standard or extended).
 * @param data Pointer to payload bytes.
 * @param len Number of data bytes (0-8).
 * @return 0 on success, negative on failure.
 *
 * @details
 * Uses write() on the raw socket to transmit the frame. Handles partial writes
 * and ensures data length does not exceed CAN_MAX_DLEN.
 */
int hal_can_send(uint32_t id, const uint8_t* data, uint8_t len) {
    if (can_socket < 0) return -1; // Socket not initialized

    if (len > CAN_MAX_DLEN) len = CAN_MAX_DLEN;

    struct can_frame frame;
    frame.can_id = id;
    frame.can_dlc = len;
    memcpy(frame.data, data, len);

    int bytes_sent = write(can_socket, &frame, sizeof(struct can_frame));

    if (bytes_sent < 0) return -2; // Failed to send
    if (bytes_sent < (int)sizeof(struct can_frame)) {
        fprintf(stderr, "Error write: partial CAN frame sent\n");
        return -3;
    }

    return 0;
}

/**
 * @brief Receives a single CAN frame in non-blocking mode.
 * @param id Pointer to store received CAN ID.
 * @param data Buffer to store received payload.
 * @param len Pointer to store received payload length.
 * @return 1 if frame received, 0 if none available, negative on error.
 *
 * @details
 * Uses read() on the non-blocking socket. Handles EAGAIN/EWOULDBLOCK as normal.
 */
int hal_can_receive(uint32_t* id, uint8_t* data, uint8_t* len) {
    if (can_socket < 0) return -1; // Socket not initialized

    struct can_frame frame;
    int bytes_read = read(can_socket, &frame, sizeof(struct can_frame));

    if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0; // No data available
        perror("Error read CAN socket");
        return -2;
    }
    if (bytes_read < (int)sizeof(struct can_frame)) {
        fprintf(stderr, "Error read: incomplete CAN frame received\n");
        return -3;
    }

    *id = frame.can_id;
    *len = frame.can_dlc;
    if (*len > CAN_MAX_DLEN) *len = CAN_MAX_DLEN;
    memcpy(data, frame.data, *len);

    return 1;
}

/**
 * @brief Closes the CAN interface.
 *
 * @details
 * Safely closes the socket if it was previously opened and resets the global descriptor.
 */
void hal_can_shutdown(void) {
    if (can_socket >= 0) {
        close(can_socket);
        can_socket = -1;
        printf("CAN Interface closed.\n");
    }
}
