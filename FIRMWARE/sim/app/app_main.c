
/**
 * @file app_main.c
 * @brief Main application entry point for the F1 Steering Wheel simulator.
 *
 * @details
 * This module initializes HAL drivers, executes the main real-time loop,
 * manages CAN communication with the ECU, debounces and interprets user inputs,
 * applies filtering (clutch & temperature), and updates the TFT display at ~60 FPS.
 *
 * The code supports:
 *  - PC simulation mode (SDL-based display)
 *  - Target MCU mode (ILI9341 TFT)
 *  - CAN TX/RX status visualization
 *  - Button callbacks + message UI
 *  - Rotary switch reading
 *  - Clutch smoothing using EMA
 *  - Temperature smoothing using rate-limit filtering
 *
 * @note All drawing operations refresh at 16 ms (~60 Hz), and CAN frames
 *       are transmitted event-based + keep-alive every 200 ms.
 */

/*==============================================================================
 *                              INCLUDES
 *==============================================================================*/

#include "hal_adc.h"
#include "hal_gpio.h"
#include "hal_delay.h"      // HAL for the delay 
#include "hal_spi.h"
#include "TFT_LCD.h"        // driver OLED of the display
#include "hal_lcd.h"        // HAL Displey [ONLY SIMULATION]

#include "clutch.h"
#include "rotary_switch.h"
#include "buttons.h"        // Includes functions for reading and debouncing button inputs
#include "can.h"            // Includes functions for receiving and sending CAN messages

#include <stdint.h>         // Includes standard integer types like uint8_t, uint32_t
#include <stddef.h>       
#include <math.h>           // include for fabsfs 
#include <string.h>
#include <stdio.h>          // include for prints [SIMULATION ONLY]


/*==============================================================================
 *                         GLOBAL STATE VARIABLES
 *==============================================================================*/

/** @brief Message displayed on screen (e.g., "GEAR UP"). */
static const char *msg = "-";

/** @brief Indicates that a button event has occurred. */
static bool Button_flag=false;

/** @brief Counter to clear message after short time. */
static int msg_clear_counter=0;

/*--- CAN Communication Visual Feedback ---*/
static bool can_tx_pulse = false;      /**< True when a CAN TX pulse is active. */
static bool can_rx_pulse = false;      /**< True when a CAN RX pulse is active. */
static uint32_t can_tx_time = 0;       /**< Timestamp of last CAN TX frame. */
static uint32_t can_rx_time = 0;       /**< Timestamp of last CAN RX frame. */
static bool can_active = false;        /**< True if ECU communication is active. */


/*==============================================================================
 *                           LOCAL UTILIY FUNCTIONS
 *==============================================================================*/

/**
 * @brief Prevents temperature values from jumping too fast on screen.
 *
 * @param previous Last displayed temperature.
 * @param input    New raw temperature from CAN.
 * @param max_step Maximum allowed change per update (°C per frame).
 * @return Smoothed output temperature.
 */
static int temp_rate_limit(int previous, int input, int max_step)
{
    int diff = input - previous;

    if (diff > max_step)
        return previous + max_step;

    if (diff < -max_step)
        return previous - max_step;

    return input;   // small change -> accept it
}

/*==============================================================================
 *                          DISPLAY RENDERING
 *==============================================================================*/

/**
 * @brief Render all dynamic information on the TFT display.
 *
 * @param clutch        Current clutch percentage (0–100%).
 * @param pos           Rotary switch position.
 * @param temp1         ECU-reported temperature 1.
 * @param temp2         ECU-reported temperature 2.
 * @param gear          Current gear.
 * @param pit_a         PIT limiter active flag.
 * @param drs_a         DRS active flag.
 * @param temp_alarm    Temperature alarm flag.
 * @param msg           Message to display (from button events).
 */
void lcd_update_status(float clutch, int pos, int temp1, int temp2, int gear, bool pit_a, bool drs_a, bool temp_alarm, const char * btn_msg){

    /*-----------------Clear screen--------------------*/
    LCD_fill_rectangle(0, 0, 320, 240, BLACK);


    /*------------------ECU & CAN Status --------------*/
    int icon_center_x = 160;
    int icon_center_y = 10;
    int icon_radius = 3;

    // ECU active/inactive text
    LCD_draw_string(icon_center_x - 24, icon_center_y-3, "ECU", can_active ? GREEN : RED, BLACK, 1);

    // TX (Blue) and RX (Green) indicators
    if(can_tx_pulse){
        LCD_fill_circle(icon_center_x, icon_center_y, icon_radius, BLUE); //Recent transmision
    }else{
        LCD_draw_circle(icon_center_x, icon_center_y, icon_radius, WHITE);
    }

    if(can_rx_pulse){
        LCD_fill_circle(icon_center_x+8, icon_center_y, icon_radius, GREEN); //Recent reception
    }else{
        LCD_draw_circle(icon_center_x+8, icon_center_y, icon_radius, WHITE);
    }
    

    /*-----------Temperatures (Y=20)-------------------*/
    LCD_draw_string(12, 20, "T1:", WHITE, BLACK, 2);
    LCD_draw_number(48, 20, temp1, WHITE, BLACK, 2);
    LCD_draw_string(85, 20, "C", WHITE, BLACK, 2);

    LCD_draw_string(220, 20, "T2:", WHITE, BLACK, 2);
    LCD_draw_number(256, 20, temp2, WHITE, BLACK, 2);
    LCD_draw_string(292, 20, "C", WHITE, BLACK, 2);

    
    /*-----------Clutch Bar(Y=50)---------------------*/
    int clutchY = 50;
    LCD_draw_string(12, clutchY, "Clutch", WHITE, BLACK, 2);
    int barX = 100, barY = clutchY, barW = 160, barH = 18;
    LCD_draw_rectangle(barX, barY, barW, barH, WHITE);
    int fillW = (int)((clutch / 100.0f) * barW);
    if (fillW < 0) fillW = 0;
    if (fillW > barW) fillW = barW;

    //Color of the bar
    uint16_t color_fill = GREEN;
    if (clutch > 70.0f) color_fill = RED;
    else if (clutch > 40.0f) color_fill = YELLOW;
    LCD_fill_rectangle(barX, barY, fillW, barH, color_fill);

    LCD_printf(barX + barW + 10, barY,WHITE,BLACK,2,"%d%%",(int)clutch);
    
    
    /*-----------Rotary Setup (Y=80)-------------------*/
    int setupY = 80;
    LCD_draw_string(12, setupY, "SETUP:", WHITE, BLACK, 2);
    LCD_draw_char(110, setupY, '[', WHITE, BLACK, 2);
    LCD_draw_number(124, setupY, pos, WHITE, BLACK, 2);
    LCD_draw_char(136 + (pos > 9 ? 6 : 0), setupY, ']', WHITE, BLACK, 2);

   
    /*-------------Buttons messages-------------------*/
    if (btn_msg && strcmp(btn_msg, "-") != 0) {

        static int blink_counter = 0;
        blink_counter++;
        if (blink_counter > 1000000) blink_counter = 0; // Reset every ~16 seconds at 60FPS

        bool visible = (blink_counter / 10) % 2 == 0;   // Toggle every ~10 frames

        if (visible){
            LCD_draw_string(180, setupY, btn_msg, YELLOW, BLACK, 2);
        }
    }

  
    /*--------------Gear box center-------------------*/
    
    // Position/Size
    int gearBoxW = 54; // Reduced width for visual lateral centering
    int gearBoxH = 60; // Adjusted height
    int gearBoxX = (320 - gearBoxW) / 2; // Centered X: (320 - 54) / 2 = 133
    int gearBoxY = 135; // Start Y position 
    
    int fontSize = 6;
    int fontWidth = 6 * fontSize; // 36 pixels
    int fontHeight = 6 * fontSize; // 36 pixels
    
    // "GEAR" label (size 2, Y=105)
    LCD_draw_string(135, 105, "GEAR", WHITE, BLACK, 2); 
    
    // Gear box rectangle
    LCD_draw_rectangle(gearBoxX, gearBoxY, gearBoxW, gearBoxH, WHITE);
    
    // Calculate character position within the 54x60px box
    // X Offset = (54 - 36) / 2 = 9. -> charX = 133 + 9 = 142 (Mathematical Center)
    int charX = gearBoxX + (gearBoxW - fontWidth) / 2; 
    // Y Offset = (60 - 36) / 2 = 12. -> charY = 135 + 12 = 147 (Mathematical Center)
    int charY = gearBoxY + (gearBoxH - fontHeight) / 2; 

    // This is the critical adjustment section:
    charX += 1; 
    charY -= 1; 
    
    if (gear == 0)
        LCD_draw_char(charX, charY, 'N', CYAN, BLACK, fontSize);
    else
        LCD_draw_number(charX, charY, gear, CYAN, BLACK, fontSize);

 

    /*--------------Bottom Status Boxes-------------------*/
    
    // Bottom rectangles definition
    int cubeY = 215;
    int cubeW = 106;
    int cubeH = 25;

    // DRS
    LCD_draw_rectangle(0, cubeY, cubeW, cubeH, WHITE);
    uint16_t drs_bg_color = BLACK;
    if (drs_a) {
        drs_bg_color = BLUE;
        LCD_fill_rectangle(0, cubeY, cubeW, cubeH, drs_bg_color);
    }
    // Draw text AFTER fill to ensure contrast
    LCD_draw_string(36, cubeY + 4, "DRS", WHITE, drs_bg_color, 2); 

    // PIT
    LCD_draw_rectangle(cubeW + 1, cubeY, cubeW, cubeH, WHITE);
    uint16_t pit_bg_color = BLACK;
    if (pit_a) {
        pit_bg_color = GREEN;
        LCD_fill_rectangle(cubeW + 1, cubeY, cubeW, cubeH, pit_bg_color);
    }
    // Draw text AFTER fill to ensure contrast
    LCD_draw_string(cubeW + 36, cubeY + 4, "PIT", WHITE, pit_bg_color, 2);

    // TEMP
    LCD_draw_rectangle(2 * cubeW + 2, cubeY, cubeW, cubeH, WHITE);
    uint16_t temp_bg_color = BLACK;
    if (temp_alarm) {
        temp_bg_color = RED;
        LCD_fill_rectangle(2 * cubeW + 2, cubeY, cubeW, cubeH, temp_bg_color);
    }
    // Draw text AFTER fill to ensure contrast
    LCD_draw_string(2 * cubeW + 30, cubeY + 4, "TEMP", WHITE, temp_bg_color, 2);
}

void ui_update(uint8_t btnmask, int pos, uint16_t raw_rot, float clutch, uint16_t raw_clutch, bool LED1, bool LED2, uint32_t now_ms){
    /*---------------------Print Banner-------------------------*/
    printf("\r\n==============================\r\n");
    printf("  F1 Steering Wheel - DEBUG UI\r\n");
    printf("  Target: NXP S32K118\r\n");
    printf("  UART:   115200 8N1\r\n");
    printf (" RUN Time: %u ms\r \n", now_ms);
    printf("==============================\r\n\r\n");

    /*--------------------Periodic Status-------------------------*/

    /*Buttons status*/
    printf ("---BTNS---\n");
    printf(" Buttons: 0x%02X\r\n", btnmask);

    /*ADC*/
    printf ("---ADC---\n");
    printf(" Rotary : %u -> %u\r \n", raw_rot, pos);

    printf(" Clutch : %u -> %.1f %%\r \n", raw_clutch, clutch);

    /*Led Status*/
    printf ("---LED STATUS---\n");
    printf (" Sent [%u] --> LED 1\r \n", LED1);
    printf (" Sent [%u] --> LED 2\r \n", LED2);

    /*CAN*/
    printf ("---CAN STATUS---\n");
    printf ( " CAN: %s \r \n" , can_active ? "ACTIVE" : "INACTIVE");
    printf ( " CAN TX: %ums ago \r \n", now_ms - can_tx_time);

    printf ( " CAN RX: %ums ago \r \n", now_ms - can_rx_time);


    printf ( "\r -----------------------------\r \n");



}

/*==============================================================================
 *                       BUTTONS CALLBACK DEFINITON
 *==============================================================================*/


/**
 * @brief Callback for Button 1 - Gear Up.
 * @param statebtn1 Current state of the button (true = pressed).
 */
void callback_Btn1(bool statebtn1) { 

    printf ( "[BTN] #1: UP -> Press[%u] \r \n" , statebtn1);
    msg="GEAR UP";
    Button_flag=true;
    msg_clear_counter=0;

}

/**
 * @brief Callback for Button 2 - Gear Down.
 */
void callback_Btn2(bool statebtn2) { 
   
    printf ( "[BTN] #2: DOWN-> Press[%u] \r \n" , statebtn2);
    msg="GEAR DOWN";
    Button_flag=true;
    msg_clear_counter=0;
}

/**
 * @brief Callback for Button 3 - DRS Activation.
 */
void callback_Btn3(bool statebtn3) { 
    if (statebtn3){
        printf("[BTN] #3: SPARE #1\n");
        msg="DRS";
        Button_flag=true;
        msg_clear_counter=0;
    }
    else{
      printf("[BTN] #3: Realeased \n");  
    }
}

/**
 * @brief Callback for Button 4 - PIT Limiter.
 */
void callback_Btn4(bool statebtn4) { 
    if (statebtn4) {
        printf("[BTN] #4: SPARE #2\n");
        msg="PIT";
        Button_flag=true;
        msg_clear_counter=0;
    }else{
        printf("[BTN] #4: Realeased \n"); 
    }
}


/*==============================================================================
 *                       MAIN APPLICATION FUNCTION
 *==============================================================================*/

/**
 * @brief Main application function for steering wheel simulation.
 *
 * @details
 * Initializes all modules, enters the main control loop, manages CAN
 * transmission/reception, and updates the display.
 */
void app_main(void) {

    /* --------------------------INITIALIZATION----------------------------------- */
    // Initialize all the necessary modules before starting the main loop.

    HAL_GPIO_Init();    //Initilize al GPIO 
    buttons_init();     // Initialize the button driver and its underlying HAL.

    hal_adc_init();     // Initialize the ADC peripheral (HAL layer)
    clutch_Init();      // Initialize the clutch input driver
    rotary_Init(10);    // Initialize the rotary switch with 10 discrete positions 

    HAL_SPI_Init();     // Initilize the SPI comunication 

    // Simulation / Target Display initialization
    HAL_Display_Init();         // [ONLY SIMULATION]
    //LCD_display9341_init();   // [TARGET ONLY]


    CAN_Init();         // Initialize CAN communication channel

    
    /*Register button callbacks*/
    buttons_registerCallback(0,callback_Btn1);
    buttons_registerCallback(1,callback_Btn2);
    buttons_registerCallback(2,callback_Btn3);
    buttons_registerCallback(3,callback_Btn4);


    /*-----------------------MAIN LOOP VARIABLES---------------------------------*/

    /* Declare structures for sending and receiving messages */
    SteeringWheelStatus_t status = {0};
    ECUStatus_t ecu = {0};

    uint32_t t_ms = 0, now_ms=0; 
    uint32_t last_can_time = 0;
    uint32_t last_display_time=0;
    uint32_t last_ui_time=0;

    const uint32_t DISPLAY_PERIOD_MS= 10000;    // 300000ms -> 5 minutes
    const uint32_t UI_PERIOD_MS= 500;         // Send Uart 500ms 
    const uint32_t CAN_PERIOD_MS = 200;         // Sed every 200 ms (5 Hz) como keep-alive       // A simple counter to approximate elapsed milliseconds.
    const float CLUTCH_THRESHOLD = 10.0f;       // % of minimun change to send the event
   
    
    uint8_t rotary_prev=0xFF;                   // rotary Initial invalid value to force the 1st send
    float clutch_prev = -1.0f;                  // clutch Initial invalid value to force the 1st send
    float clutch_filt   = 0.0f;                 // Smooth Persistent filtered value of the Clutch
    const float clutch_alpha = 0.15f;           // Smoothing factor (0.1–0.3 recommended) for the clutch

    uint8_t gear=0;
    int t1=0, t2=0;
    bool pit_l=0,drs=0, LED1_PL=0, LED2_T=0;


    int running = 1;                            // Loop control variable. Set to 0 to exit the loop.


    /*============================== MAIN LOOP =============================*/
    while (running) {

        // ---  WINDOW EVENT HANDLING ---
        // This is crucial for the SDL window on the host PC. It processes events
        // like closing the window. If the user clicks the 'X', this function will
        // set the 'running' variable to 0, causing the loop to terminate.22
        HAL_Poll_Events(&running);  //  [ONLY SIMULATION]

        /* ------------------------INPUT STATE UPDATE ---------------------------*/

        /*---Buttons---*/
        buttons_update();                               // Reads the raw hardware state and updates the internal debounce counters.
        uint8_t s_button_val = buttons_getStable();     // Gets the stable, debounced state of all buttons as a bitmask (bit 0 for button 1, etc.).

        /*---Rotary Switch---*/
        uint16_t pos_adc=rotary_GetRawValue();          // Obtain the raw value[New]
        uint8_t position=rotary_GetPosition();          // Determine the current position index

        // Rotary Change detection
        bool rotary_changed = (position != rotary_prev); 
        if (rotary_changed) rotary_prev = position;

        //float clutch_percentage=100; // Get calibrated clutch position (0–100%)
        //uint8_t position=2;  // Determine the current position index

        /*---Clutch---*/
        uint16_t clutch_adc= clutch_GetRawValue();  // Obtain the adc raw value[New]
        // Exponential Moving Average (EMA) Filter
        float clutch_raw = clutch_GetPercentage(); // Raw ADC-based clutch value (0–100%)                
        clutch_filt = clutch_alpha * clutch_raw + (1.0f - clutch_alpha) * clutch_filt;
        float clutch_percentage = clutch_filt;      // Get calibrated clutch position (0–100%)

        // Clutch Change detection
        bool clutch_changed = (fabsf(clutch_percentage - clutch_prev) > CLUTCH_THRESHOLD);
        if (clutch_changed) clutch_prev = clutch_percentage;

        /*------------------------------------ TIME LOGIC -----------------------------------*/

        // This is a simple, approximate way to keep track of time.
        t_ms += 16;                 // We assume each loop takes roughly 16ms due to hal_delay_ms(16).
        now_ms = t_ms;
        
        /*--------------------------------------CAN TRANSMIT----------------------------------*/

        // If there exist an event like a botton was pressed, change position of rotary, a hughe change of the clutch
        if (Button_flag || rotary_changed || clutch_changed) {
            status.button_state = s_button_val;
            status.rotary_position = position;
            status.clutch_value = (int)clutch_percentage;
    
            CAN_SendSteeringStatus(&status);    // send only if were a change on Board Inputs

            Button_flag=false;                  // Reset the Buttons Flag
            last_can_time = now_ms;             // Update/Reset the temporizator for the CAN send
            last_display_time = now_ms;         // Update display time to mantain the active screen

            can_tx_pulse = true;                // Set a Flag for TX can
            can_tx_time = now_ms;               // Update the TX time when Frame is send it 
        }

        // Send frame peridically (keep-alive)
        if ((now_ms - last_can_time) >= CAN_PERIOD_MS) {
            status.button_state = s_button_val;
            status.rotary_position = position;
            status.clutch_value = (int)clutch_percentage;

            CAN_SendSteeringStatus(&status);
            last_can_time = now_ms;             // Update/Reset the temporizator for the CAN send

            can_tx_pulse = true;                // Set a Flag for TX CAN
            can_tx_time = now_ms;               //Update the TX time when Frame is send it 
        }


        /*---------------------------------CAN RECEIVE---------------------------------*/
        if (CAN_ReceiveECUStatus(&ecu)==1){
            
            /* Smooth temperatures to avoid visual jumps */
            t1 = temp_rate_limit(t1, (int)ecu.temp1, 2);   // limit ±2°C per frame
            t2 = temp_rate_limit(t2, (int)ecu.temp2, 2);   // limit ±2°C per frame

            /* Direct values (no smoothing needed) */
            gear=ecu.gear_actual;
            pit_l=ecu.pit_limiter_active;
            drs=ecu.drs_status;
            LED1_PL=ecu.led_pit;
            LED2_T=ecu.led_temp;

            can_rx_pulse = true;              // Set a Flag for RX CAN
            can_rx_time = now_ms;             //Update the RX time when Frame is send it 
        } 
        

        // CAN activity timeout logic
        if ((now_ms - can_rx_time) < 1000) { // 1 second timeout
            can_active = true;
        }
        else{
            can_active = false;
        } 


        //printf("PitL= %u\n", pit_l);
        //printf("DRS= %u\n", drs);

        /*-------------------------------LED CONTROL----------------------------------*/

        HAL_GPIO_Write(GPIO_LED_S1, LED1_PL);   //ON/OFF LED PIT Limiter
        HAL_GPIO_Write(GPIO_LED_S2, LED2_T);    //ON/OFF LED Temperature


        /*-------------------------------MESSAGE TIMEOUT ----------------------------*/

        msg_clear_counter++;

        /*Evaluate the counter to erase the msg of the buttons (clear after ~50 ticks) */
        if(msg_clear_counter>50){   
            msg="-";
            msg_clear_counter=0;
        }

        /*-------------------------------PULSE TIMEOUT------------------------------*/

        // Short flash effect (50 ms visible)
        if ((now_ms - can_tx_time) > 50) can_tx_pulse = false;
        if ((now_ms - can_rx_time) > 50) can_rx_pulse = false;

        /*---------------------------------SERIAL DEBUG UI-------------------------------*/
        
        if ((now_ms - last_ui_time) >= UI_PERIOD_MS) {
            ui_update(s_button_val, position, pos_adc, clutch_raw, clutch_adc, LED1_PL, LED2_T, now_ms);
            last_ui_time = now_ms;
        }

       /*---------------------------------DISPLAY LOGIC-------------------------------*/

       
   
        if ((now_ms - last_display_time) >= DISPLAY_PERIOD_MS) {

            /*Shutdown Display*/
            //printf("DISPLAY OFF\n");
            LCD_fill_screen(BLACK);

        }else{

            /*Display Interface */
            lcd_update_status(clutch_percentage, position, t1, t2, gear, pit_l, drs, LED2_T,msg);
            
        }

        /*-----------------------------------PRESENT FRAME --------------------------------------*/
        // This is the final step of the loop.
        HAL_Display_Present(); // [ONLY SIMULATION]
        HAL_DelayMs(16); // Pauses for ~16 milliseconds to target a frame rate of ~60 FPS (1000ms / 60fps ≈ 16.6ms).
    }

    // --- SHUTDOWN ---
    // Clean up resources when the main loop exits.
}
