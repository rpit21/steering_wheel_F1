/**
 * @file app_main.h
 * @brief Declaration of the main application entry point.
 *
 * This header allows other parts of the firmware (e.g., main.c)
 * to call the main application function that contains the primary
 * logic and main loop of the steering wheel.
 */

#ifndef APP_MAIN_H
#define APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Main application function for the steering wheel.
 *
 * This function initializes all hardware abstraction layers (HAL)
 * and enters the main control loop. It is not expected to return.
 */
void app_main(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H */
