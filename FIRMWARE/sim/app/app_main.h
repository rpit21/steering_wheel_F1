// This header file declares the main entry point for the application logic.
// Including this header allows other parts of the firmware (like main.c)
// to call the main application function.

// --- INCLUDE GUARD ---
// These preprocessor directives prevent the contents of this header file
// from being included multiple times if it's referenced by different files
// during compilation. This avoids potential redefinition errors.
#ifndef APP_MAIN_H // Checks if the symbol APP_MAIN_H has not been defined yet.
#define APP_MAIN_H // If it hasn't been defined, define it now.

// --- FUNCTION DECLARATION ---
// This declares the main application function `app_main`.
// - `void`: Indicates that the function does not return any value.
// - `app_main`: The name of the function. This function contains the primary
//               logic and main loop of the steering wheel application.
// - `(void)`: Indicates that the function does not accept any parameters.
void app_main(void);

#endif // End of the include guard block.
