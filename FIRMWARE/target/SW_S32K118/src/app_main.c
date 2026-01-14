/**
 * @file app_main.c
 * @brief Main application entry point for the F1 Steering Wheel firmware (S32K118 target).
 *
 * @details
 * This module initializes HAL drivers, executes the main real-time loop,
 * manages CAN communication with the ECU, debounces and interprets user inputs,
 * applies filtering (clutch & temperature), and updates the TFT display.
 *
 * The code is intended for the S32K118 target MCU with an ILI9341 TFT display:
 *  - ADC-based clutch reading with EMA smoothing
 *  - Rotary switch reading (ADC or digital, depending on HAL implementation)
 *  - Button handling with debouncing and callbacks
 *  - CAN TX/RX for communication with the ECU
 *  - TFT update with ECU status, clutch bar, rotary position and alarms
 *
 * Display refresh is paced using HAL_DelayMs(16), approximating ~60 FPS.
 */

/*==============================================================================
 *                              INCLUDES
 *==============================================================================*/

#include "hal_adc.h"
#include "hal_gpio.h"
#include "hal_delay.h"
#include "hal_spi.h"
#include "hal_uart.h"

#include "TFT_LCD.h"
#include "clutch.h"
#include "rotary_switch.h"
#include "buttons.h"
#include "can.h"

#include <stdint.h>
#include <stdbool.h>
#include <math.h>       /* for fabsf */
#include <string.h>
#include <stdio.h>      /* optional: debug prints */

#include "device_registers.h"

/*==============================================================================
 *                         GLOBAL STATE VARIABLES
 *==============================================================================*/

/** @brief Message displayed on screen (e.g., "GEAR UP"). */
static const char *msg = "-";

/** @brief Indicates that a button event has occurred. */
static bool Button_flag = false;

/** @brief Counter to clear message after short time. */
static int msg_clear_counter = 0;

/*--- CAN Communication Visual Feedback ---*/
static bool     can_tx_pulse = false;  /**< True when a CAN TX pulse is active. */
static bool     can_rx_pulse = false;  /**< True when a CAN RX pulse is active. */
static uint32_t can_tx_time  = 0;      /**< Timestamp of last CAN TX frame. */
static uint32_t can_rx_time  = 0;      /**< Timestamp of last CAN RX frame. */
static bool     can_active   = false;  /**< True if ECU communication is active. */

/*==============================================================================
 *                           LOCAL UTILITY FUNCTIONS
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

    return input;   /* small change -> accept it */
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
 * @param btn_msg       Message to display (from button events).
 */
static void lcd_update_status(float clutch,
                              int   pos,
                              int   temp1,
                              int   temp2,
                              int   gear,
                              bool  pit_a,
                              bool  drs_a,
                              bool  temp_alarm,
                              const char *btn_msg)
{
    /*----------------- Clear screen --------------------*/
    LCD_fill_rectangle(0, 0, 320, 240, BLACK);

    /*------------------ ECU & CAN Status --------------*/
    int icon_center_x = 160;
    int icon_center_y = 10;
    int icon_radius   = 3;

    /* ECU active/inactive text */
    LCD_draw_string(icon_center_x - 24, icon_center_y - 3,
                    "ECU", can_active ? GREEN : RED, BLACK, 1);

    /* TX (Blue) and RX (Green) indicators */
    if (can_tx_pulse) {
        LCD_fill_circle(icon_center_x, icon_center_y, icon_radius, BLUE);
    } else {
        LCD_draw_circle(icon_center_x, icon_center_y, icon_radius, WHITE);
    }

    if (can_rx_pulse) {
        LCD_fill_circle(icon_center_x + 8, icon_center_y, icon_radius, GREEN);
    } else {
        LCD_draw_circle(icon_center_x + 8, icon_center_y, icon_radius, WHITE);
    }

    /*----------- Temperatures (Y = 20) -------------------*/
    LCD_draw_string(12,  20, "T1:", WHITE, BLACK, 2);
    LCD_draw_number(48,  20, temp1, WHITE, BLACK, 2);
    LCD_draw_string(85,  20, "C",   WHITE, BLACK, 2);

    LCD_draw_string(220, 20, "T2:", WHITE, BLACK, 2);
    LCD_draw_number(256, 20, temp2, WHITE, BLACK, 2);
    LCD_draw_string(292, 20, "C",   WHITE, BLACK, 2);

    /*----------- Clutch Bar (Y = 50) ---------------------*/
    int clutchY = 50;
    LCD_draw_string(12, clutchY, "Clutch", WHITE, BLACK, 2);

    int barX = 100;
    int barY = clutchY;
    int barW = 160;
    int barH = 18;

    LCD_draw_rectangle(barX, barY, barW, barH, WHITE);

    int fillW = (int)((clutch / 100.0f) * (float)barW);
    if (fillW < 0)     fillW = 0;
    if (fillW > barW)  fillW = barW;

    /* Color of the bar */
    uint16_t color_fill = GREEN;
    if (clutch > 70.0f) {
        color_fill = RED;
    } else if (clutch > 40.0f) {
        color_fill = YELLOW;
    }

    LCD_fill_rectangle(barX, barY, fillW, barH, color_fill);
    LCD_printf(barX + barW + 10, barY, WHITE, BLACK, 2, "%d%%", (int)clutch);

    /*----------- Rotary Setup (Y = 80) -------------------*/
    int setupY = 80;
    LCD_draw_string(12, setupY, "SETUP:", WHITE, BLACK, 2);
    LCD_draw_char(110, setupY, '[', WHITE, BLACK, 2);
    LCD_draw_number(124, setupY, pos, WHITE, BLACK, 2);
    LCD_draw_char(136 + (pos > 9 ? 6 : 0), setupY, ']', WHITE, BLACK, 2);

    /*------------- Buttons messages -------------------*/
    if (btn_msg && (strcmp(btn_msg, "-") != 0)) {

        static int blink_counter = 0;
        blink_counter++;
        if (blink_counter > 1000000) {
            blink_counter = 0; /* Reset periodically to avoid overflow */
        }

        bool visible = ((blink_counter / 10) % 2) == 0;   /* Toggle every ~10 frames */

        if (visible) {
            LCD_draw_string(180, setupY, btn_msg, YELLOW, BLACK, 2);
        }
    }

    /*-------------- Gear box center -------------------*/
    int gearBoxW = 54;                  /* Reduced width for lateral centering */
    int gearBoxH = 60;                  /* Box height */
    int gearBoxX = (320 - gearBoxW) / 2;
    int gearBoxY = 135;

    int fontSize   = 6;
    int fontWidth  = 6 * fontSize;      /* 36 pixels */
    int fontHeight = 6 * fontSize;      /* 36 pixels */

    /* "GEAR" label (size 2, Y = 105) */
    LCD_draw_string(135, 105, "GEAR", WHITE, BLACK, 2);

    /* Gear box rectangle */
    LCD_draw_rectangle(gearBoxX, gearBoxY, gearBoxW, gearBoxH, WHITE);

    /* Center the character inside the 54x60 box */
    int charX = gearBoxX + (gearBoxW - fontWidth) / 2;
    int charY = gearBoxY + (gearBoxH - fontHeight) / 2;

    /* Small fine-tuning for visual alignment */
    charX += 1;
    charY -= 1;

    if (gear == 0) {
        LCD_draw_char(charX, charY, 'N', CYAN, BLACK, fontSize);
    } else {
        LCD_draw_number(charX, charY, gear, CYAN, BLACK, fontSize);
    }

    /*-------------- Bottom Status Boxes -------------------*/
    int cubeY = 215;
    int cubeW = 106;
    int cubeH = 25;

    /* DRS */
    LCD_draw_rectangle(0, cubeY, cubeW, cubeH, WHITE);
    uint16_t drs_bg_color = BLACK;
    if (drs_a) {
        drs_bg_color = BLUE;
        LCD_fill_rectangle(0, cubeY, cubeW, cubeH, drs_bg_color);
    }
    LCD_draw_string(36, cubeY + 4, "DRS", WHITE, drs_bg_color, 2);

    /* PIT */
    LCD_draw_rectangle(cubeW + 1, cubeY, cubeW, cubeH, WHITE);
    uint16_t pit_bg_color = BLACK;
    if (pit_a) {
        pit_bg_color = GREEN;
        LCD_fill_rectangle(cubeW + 1, cubeY, cubeW, cubeH, pit_bg_color);
    }
    LCD_draw_string(cubeW + 36, cubeY + 4, "PIT", WHITE, pit_bg_color, 2);

    /* TEMP */
    LCD_draw_rectangle(2 * cubeW + 2, cubeY, cubeW, cubeH, WHITE);
    uint16_t temp_bg_color = BLACK;
    if (temp_alarm) {
        temp_bg_color = RED;
        LCD_fill_rectangle(2 * cubeW + 2, cubeY, cubeW, cubeH, temp_bg_color);
    }
    LCD_draw_string(2 * cubeW + 30, cubeY + 4, "TEMP", WHITE, temp_bg_color, 2);
}

/*==============================================================================
 *                          UI UART RENDERING
 *==============================================================================*/

/**
 * @brief Render the dynamic information on serial UART UI.
 *
 * @param btnmask       Buttons value bit mask.
 * @param pos           Rotary switch position.
 * @param raw_pos       ADC value of the Rotary switch.
 * @param clutch        Current clutch percentage (0–100%).
 * @param raw_clutch    ADC value of the Clutch.
 * @param LED1         	LED of the PIT limiter status.
 * @param LED2         	LED of the Temperature limiter status.
 * @param now_ms    	Actual tracking/run time ms.
 */
void ui_update( uint8_t 	btnmask,
				int 		pos,
				uint16_t 	raw_rot,
				float 		clutch,
				uint16_t 	raw_clutch,
				bool 		LED1,
				bool 		LED2,
				uint32_t 	now_ms,
				int   temp1,
				int   temp2,
				int   gear,
				bool  pit_a,
				bool  drs_a)
{
    /*---------------------Print Banner-------------------------*/
    HAL_UART_Printf("\r\n ==============================\r\n");
    HAL_UART_Printf("  F1 Steering Wheel - DEBUG UI\r\n");
    HAL_UART_Printf("  Target: NXP S32K118\r\n");
    HAL_UART_Printf("  UART:   115200 8N1\r\n");
    HAL_UART_Printf("  RUN Time: %u ms\r \n", now_ms);
    HAL_UART_Printf("==============================\r\n\r\n");

    /*--------------------Periodic Status-------------------------*/

    /*Buttons status*/
    HAL_UART_Printf(" ---BTNS---\r\n");
    HAL_UART_Printf("  Buttons: 0x%02X\r\n", btnmask);

    /*ADC*/
    HAL_UART_Printf (" ---ADC---\r\n");
    HAL_UART_Printf("  Rotary : %u -> %u\r \n", raw_rot, pos);

    HAL_UART_Printf(" Clutch : %u -> %u %% \r \n", raw_clutch, (int) clutch);

    /*Led Status*/
    HAL_UART_Printf("---LED STATUS---\r\n");
    HAL_UART_Printf("  Sent [%u] --> LED 1\r \n", LED1);
    HAL_UART_Printf(" Sent [%u] --> LED 2\r \n", LED2);

    /*CAN*/
    HAL_UART_Printf("---CAN STATUS---\r\n");
    HAL_UART_Printf( "  TEMPERATURE 1: %i° \r \n", temp1);
    HAL_UART_Printf( " TEMPERATURE 2: %i° \r \n", temp2);
    HAL_UART_Printf( " CAN: %s \r \n" , can_active ? "ACTIVE" : "INACTIVE");
    HAL_UART_Printf( " CAN TX: %u ms ago \r \n", now_ms - can_tx_time);

    HAL_UART_Printf( " CAN RX: %u ms ago \r \n", now_ms - can_rx_time);
    HAL_UART_Printf( " -GEAR: %u \r \n", gear);
    HAL_UART_Printf( " -PL: %i \r \n",pit_a);
    HAL_UART_Printf( " -DRS: %i \r \n",drs_a);


    HAL_UART_Printf( "\r -----------------------------\r \n");
}

/**
 * @brief Register configuration information for the serial UART UI.
 *
 */
void debug_dump(void)
{
    HAL_UART_Printf("\r\n=================== REGISTER DUMP ===================\r\n");

    /* --------------------------------------------------
     * WDOG
     * -------------------------------------------------- */
    HAL_UART_Printf("[WDOG]\r\n");
    HAL_UART_Printf("  CS     = 0x%08X\r\n", IP_WDOG->CS);
    HAL_UART_Printf("  CNT    = 0x%08X\r\n", IP_WDOG->CNT);
    HAL_UART_Printf("  TOVAL  = 0x%08X\r\n", IP_WDOG->TOVAL);

    /* --------------------------------------------------
     * CLOCKS (SCG)
     * -------------------------------------------------- */
    HAL_UART_Printf("\r\n[SCG]\r\n");
    HAL_UART_Printf("  CSR     = 0x%08X\r\n", IP_SCG->CSR);
    HAL_UART_Printf("  RCCR    = 0x%08X\r\n", IP_SCG->RCCR);
    HAL_UART_Printf("  SOSCCSR = 0x%08X\r\n", IP_SCG->SOSCCSR);
    HAL_UART_Printf("  SOSCDIV = 0x%08X\r\n", IP_SCG->SOSCDIV);
    HAL_UART_Printf("  SOSCCFG = 0x%08X\r\n", IP_SCG->SOSCCFG);
    HAL_UART_Printf("  FIRCCSR = 0x%08X\r\n", IP_SCG->FIRCCSR);
    HAL_UART_Printf("  FIRCDIV = 0x%08X\r\n", IP_SCG->FIRCDIV);
    //HAL_UART_Printf("  FIRCCTL = 0x%08X\r\n", IP_SCG->FIRCCTL);
    HAL_UART_Printf("  SIRCDIV = 0x%08X\r\n", IP_SCG->SIRCDIV);

    /* --------------------------------------------------
     * PCC (Clock gating)
     * -------------------------------------------------- */
    HAL_UART_Printf("\r\n[PCC]\r\n");
    HAL_UART_Printf("  PCC_PORTA    = 0x%08X\r\n", IP_PCC->PCCn[PCC_PORTA_INDEX]);
    HAL_UART_Printf("  PCC_PORTB    = 0x%08X\r\n", IP_PCC->PCCn[PCC_PORTB_INDEX]);
    HAL_UART_Printf("  PCC_PORTC    = 0x%08X\r\n", IP_PCC->PCCn[PCC_PORTC_INDEX]);
    HAL_UART_Printf("  PCC_LPUART0  = 0x%08X\r\n", IP_PCC->PCCn[PCC_LPUART0_INDEX]);
    HAL_UART_Printf("  PCC_LPSPI0   = 0x%08X\r\n", IP_PCC->PCCn[PCC_LPSPI0_INDEX]);
    HAL_UART_Printf("  PCC_ADC0     = 0x%08X\r\n", IP_PCC->PCCn[PCC_ADC0_INDEX]);
    HAL_UART_Printf("  PCC_FlexCAN0 = 0x%08X\r\n", IP_PCC->PCCn[PCC_FlexCAN0_INDEX]);

    /* --------------------------------------------------
     * PORT MUX (PCR) – PINs
     * -------------------------------------------------- */
    HAL_UART_Printf("\r\n[PORTA PCR 0..16]\r\n");
    for (int i = 0; i <= 16; i++) {
        HAL_UART_Printf("  PORTA->PCR[%2d] = 0x%08X\r\n", i, IP_PORTA->PCR[i]);
    }

    HAL_UART_Printf("\r\n[PORTB PCR 0..16]\r\n");
    for (int i = 0; i <= 16; i++) {
        HAL_UART_Printf("  PORTB->PCR[%2d] = 0x%08X\r\n", i, IP_PORTB->PCR[i]);
    }

    HAL_UART_Printf("\r\n[PORTC PCR 0..16]\r\n");
    for (int i = 0; i <= 16; i++) {
        HAL_UART_Printf("  PORTC->PCR[%2d] = 0x%08X\r\n", i, IP_PORTC->PCR[i]);
    }

    /* It is posible to check:
     * - PTA2/PTA3 -> UART (MUX=6)
     * - PTB0 -> TFT_CS (MUX=1)
     * - PTB3/PTB4 -> btns, etc.
     * - PTC8/PTC9 -> TFT DC/RST (MUX=1)
     * - PTC14/PTC15 -> ADC (MUX=0)
     * - PTC2/PTC3 -> CAN (MUX=3)
     */

    /* --------------------------------------------------
     * GPIO DIRECTIONS (LEDs, TFT, Buttons)
     * -------------------------------------------------- */
    HAL_UART_Printf("\r\n[GPIOA]\r\n");
    HAL_UART_Printf("  PDOR = 0x%08X\r\n", IP_PTA->PDOR);
    HAL_UART_Printf("  PDIR = 0x%08X\r\n", IP_PTA->PDIR);
    HAL_UART_Printf("  PDDR = 0x%08X\r\n", IP_PTA->PDDR);

    HAL_UART_Printf("\r\n[GPIOB]\r\n");
    HAL_UART_Printf("  PDOR = 0x%08X\r\n", IP_PTB->PDOR);
    HAL_UART_Printf("  PDIR = 0x%08X\r\n", IP_PTB->PDIR);
    HAL_UART_Printf("  PDDR = 0x%08X\r\n", IP_PTB->PDDR);

    HAL_UART_Printf("\r\n[GPIOC]\r\n");
    HAL_UART_Printf("  PDOR = 0x%08X\r\n", IP_PTC->PDOR);
    HAL_UART_Printf("  PDIR = 0x%08X\r\n", IP_PTC->PDIR);
    HAL_UART_Printf("  PDDR = 0x%08X\r\n", IP_PTC->PDDR);

    /* --------------------------------------------------
     * LPUART0
     * -------------------------------------------------- */
    HAL_UART_Printf("\r\n[LPUART0]\r\n");
    HAL_UART_Printf("  BAUD   = 0x%08X\r\n", IP_LPUART0->BAUD);
    HAL_UART_Printf("  CTRL   = 0x%08X\r\n", IP_LPUART0->CTRL);
    HAL_UART_Printf("  STAT   = 0x%08X\r\n", IP_LPUART0->STAT);

    /* --------------------------------------------------
     * LPSPI0
     * -------------------------------------------------- */
    HAL_UART_Printf("\r\n[LPSPI0]\r\n");
    HAL_UART_Printf("  CR     = 0x%08X\r\n", IP_LPSPI0->CR);
    HAL_UART_Printf("  SR     = 0x%08X\r\n", IP_LPSPI0->SR);
    HAL_UART_Printf("  IER    = 0x%08X\r\n", IP_LPSPI0->IER);
    HAL_UART_Printf("  DER    = 0x%08X\r\n", IP_LPSPI0->DER);
    HAL_UART_Printf("  CFGR0  = 0x%08X\r\n", IP_LPSPI0->CFGR0);
    HAL_UART_Printf("  CFGR1  = 0x%08X\r\n", IP_LPSPI0->CFGR1);
    HAL_UART_Printf("  TCR    = 0x%08X\r\n", IP_LPSPI0->TCR);
    HAL_UART_Printf("  CCR    = 0x%08X\r\n", IP_LPSPI0->CCR);

    /* --------------------------------------------------
     * ADC0
     * -------------------------------------------------- */
    HAL_UART_Printf("\r\n[ADC0]\r\n");
    HAL_UART_Printf("  SC1[0] = 0x%08X\r\n", IP_ADC0->SC1[0]);
    HAL_UART_Printf("  CFG1   = 0x%08X\r\n", IP_ADC0->CFG1);
    HAL_UART_Printf("  CFG2   = 0x%08X\r\n", IP_ADC0->CFG2);
    HAL_UART_Printf("  SC2    = 0x%08X\r\n", IP_ADC0->SC2);
    HAL_UART_Printf("  SC3    = 0x%08X\r\n", IP_ADC0->SC3);

    /* --------------------------------------------------
     * FLEXCAN0
     * -------------------------------------------------- */
    HAL_UART_Printf("\r\n[FLEXCAN0]\r\n");
    HAL_UART_Printf("  MCR     = 0x%08X\r\n", IP_FLEXCAN0->MCR);
    HAL_UART_Printf("  CTRL1   = 0x%08X\r\n", IP_FLEXCAN0->CTRL1);
    HAL_UART_Printf("  ECR     = 0x%08X\r\n", IP_FLEXCAN0->ECR);
    HAL_UART_Printf("  ESR1    = 0x%08X\r\n", IP_FLEXCAN0->ESR1);
    HAL_UART_Printf("  IMASK1  = 0x%08X\r\n", IP_FLEXCAN0->IMASK1);
    HAL_UART_Printf("  IFLAG1  = 0x%08X\r\n", IP_FLEXCAN0->IFLAG1);
    HAL_UART_Printf("  RXMGMASK= 0x%08X\r\n", IP_FLEXCAN0->RXMGMASK);

    HAL_UART_Printf("\r\n================= END REGISTER DUMP =================\r\n\r\n");
}

/*==============================================================================
 *                       BUTTONS CALLBACK DEFINITIONS
 *==============================================================================*/

/**
 * @brief Callback for Button 1 - Gear Up.
 * @param statebtn1 Current state of the button (true = pressed).
 */
void callback_Btn1(bool statebtn1) { 

    HAL_UART_Printf ( " [BTN] #1: UP -> Press[%u] \r \n" , statebtn1);
    msg="GEAR UP";
    Button_flag=true;
    msg_clear_counter=0;

}

/**
 * @brief Callback for Button 2 - Gear Down.
 */
void callback_Btn2(bool statebtn2) { 
   
    HAL_UART_Printf ( "[BTN] #2: DOWN-> Press[%u] \r \n" , statebtn2);
    msg="GEAR DOWN";
    Button_flag=true;
    msg_clear_counter=0;
}

/**
 * @brief Callback for Button 3 - DRS Activation.
 */
void callback_Btn3(bool statebtn3)
{
    if (statebtn3) {
    	HAL_UART_Printf(" [BTN-C]#3: DRS \r \n");
        msg = "DRS";
        Button_flag = true;
        msg_clear_counter = 0;
    } else {
    	HAL_UART_Printf(" [BTN-C]#3: Released\r\n");
    }
}

/**
 * @brief Callback for Button 4 - PIT Limiter.
 */
void callback_Btn4(bool statebtn4)
{
    if (statebtn4) {
    	HAL_UART_Printf(" [BTN-C]#4: PIT\r\n");
        msg = "PIT";
        Button_flag = true;
        msg_clear_counter = 0;
    } else {
    	HAL_UART_Printf(" [BTN-C]#4: Released\r\n");
    }
}



/*==============================================================================
 *                       MAIN APPLICATION FUNCTION
 *==============================================================================*/

/**
 * @brief Main application function for steering wheel firmware on S32K118.
 *
 * @details
 * Initializes all modules, enters the main control loop, manages CAN
 * transmission/reception, and updates the TFT display.
 */
void app_main(void)
{
    /* -------------------------- INITIALIZATION -------------------------------- */

    /* 1. Configura i PIN (incluso il Reset a livello Alto inizialmente) */
    HAL_GPIO_Init();

    /* 2. Inizializza i driver di input */
    buttons_init();
    hal_adc_init();
    clutch_Init();
    rotary_Init(10);

    /* 3. Inizializza SPI (Clock, MUX, Baudrate) */
    HAL_SPI_Init();

    /* 5. Inizializza il Display (Manda comandi SPI) */
    //LCD_display9341_init();

    /* Pulisci lo schermo per vedere se è vivo (sfondo Nero) */
    //LCD_fill_rectangle(0, 0, 320, 240, BLACK);
    //HAL_DelayMs(500);
    //LCD_fill_screen(RED);
    //HAL_DelayMs(500);
    //LCD_fill_screen(GREEN);
    //HAL_DelayMs(500);
    //LCD_fill_screen(BLUE);

    /* 6. Inizializza CAN */
    CAN_Init();

    /* Register button callbacks */
    buttons_registerCallback(0, callback_Btn1);
    buttons_registerCallback(1, callback_Btn2);
    buttons_registerCallback(2, callback_Btn3);
    buttons_registerCallback(3, callback_Btn4);

    debug_dump(); // Opzionale

    /*----------------------- MAIN LOOP VARIABLES --------------------------------*/
    /* (Le tue variabili restano uguali...) */
    SteeringWheelStatus_t status = {0};
    ECUStatus_t           ecu    = {0};

    uint32_t t_ms             = 0;
    uint32_t now_ms           = 0;
    uint32_t last_can_time    = 0;
    uint32_t last_display_time = 0;
    uint32_t last_ui_time     = 0;

    const uint32_t UI_PERIOD_MS      = 1000u;
    const uint32_t DISPLAY_PERIOD_MS = 10000u;
    const uint32_t CAN_PERIOD_MS     = 200u;
    const float    CLUTCH_THRESHOLD  = 10.0f;

    uint8_t rotary_prev   = 0xFF;
    float   clutch_prev   = -1.0f;
    float   clutch_filt   = 0.0f;
    const float clutch_alpha = 0.15f;

    uint8_t gear = 0;
    int     t1   = 0;
    int     t2   = 0;
    bool    pit_l   = false;
    bool    drs     = false;
    bool    LED1_PL = true;
    bool    LED2_T  = true;

    /*============================== MAIN LOOP =============================*/
    for (;;)
    {
        /* ------------------------ INPUT STATE UPDATE ---------------------------*/
        buttons_update();
        uint8_t s_button_val = buttons_getStable();

        uint16_t pos_adc = rotary_GetRawValue();
        uint8_t position = rotary_GetPosition();

        bool rotary_changed = (position != rotary_prev);
        if (rotary_changed) {
            rotary_prev = position;
        }

        uint16_t clutch_adc = clutch_GetRawValue();
        float clutch_raw = clutch_GetPercentage();
        clutch_filt = clutch_alpha * clutch_raw + (1.0f - clutch_alpha) * clutch_filt;
        float clutch_percentage = clutch_filt;

        bool clutch_changed = (fabsf(clutch_percentage - clutch_prev) > CLUTCH_THRESHOLD);
        if (clutch_changed) {
            clutch_prev = clutch_percentage;
        }

        /*------------------------------------ TIME LOGIC -----------------------------------*/
        t_ms += 16u;
        now_ms = t_ms;

        /*-------------------------------------- CAN TRANSMIT --------------------------------*/
        if (Button_flag || rotary_changed || clutch_changed) {
            status.button_state    = s_button_val;
            status.rotary_position = position;
            status.clutch_value    = (int)clutch_percentage;
            CAN_SendSteeringStatus(&status);

            Button_flag       = false;
            last_can_time     = now_ms;
            last_display_time = now_ms;
            can_tx_pulse = true;
            can_tx_time  = now_ms;
        }

        if ((now_ms - last_can_time) >= CAN_PERIOD_MS) {
            status.button_state    = s_button_val;
            status.rotary_position = position;
            status.clutch_value    = (int)clutch_percentage;
            CAN_SendSteeringStatus(&status);

            last_can_time = now_ms;
            can_tx_pulse = true;
            can_tx_time  = now_ms;
        }

        /*--------------------------------- CAN RECEIVE ---------------------------------*/
        if (CAN_ReceiveECUStatus(&ecu) == 1) {
            t1 = temp_rate_limit(t1, (int)ecu.temp1, 2);
            t2 = temp_rate_limit(t2, (int)ecu.temp2, 2);
            gear   = ecu.gear_actual;
            pit_l  = ecu.pit_limiter_active;
            drs    = ecu.drs_status;
            LED1_PL = ecu.led_pit;
            LED2_T  = ecu.led_temp;
            can_rx_pulse = true;
            can_rx_time  = now_ms;
        }

        if ((now_ms - can_rx_time) < 1000u) {
            can_active = true;
        } else {
            can_active = false;
        }

        /*------------------------------- LED CONTROL ----------------------------------*/
        HAL_GPIO_Write(GPIO_LED_S1, LED1_PL);
        HAL_GPIO_Write(GPIO_LED_S2, LED2_T);

        /* [CORREZIONE] RIMOSSE LE SCRITTURE FORZATE SU CS E DC QUI! */

        /*------------------------------- MESSAGE TIMEOUT ----------------------------*/
        msg_clear_counter++;
        if (msg_clear_counter > 50) {
            msg = "-";
            msg_clear_counter = 0;
        }

        if ((now_ms - can_tx_time) > 50u) can_tx_pulse = false;
        if ((now_ms - can_rx_time) > 50u) can_rx_pulse = false;

        /*---------------------------------SERIAL DEBUG UI-------------------------------*/
		if ((now_ms - last_ui_time) >= UI_PERIOD_MS) {
			ui_update(s_button_val, position, pos_adc, clutch_raw, clutch_adc, LED1_PL, LED2_T, now_ms, t1, t2, gear, pit_l, drs);
			last_ui_time = now_ms;
		}

        /*--------------------------------- DISPLAY LOGIC -------------------------------*/
        if ((now_ms - last_display_time) >= DISPLAY_PERIOD_MS) {
            /* Shutdown opzionale */
        } else {
            /* [CORREZIONE] DE-COMMENTATA LA FUNZIONE DI UPDATE */
           /* lcd_update_status(clutch_percentage,
                              (int)position,
                              t1,
                              t2,
                              (int)gear,
                              pit_l,
                              drs,
                              LED2_T, // Nota: Qui passavi LED2_T come temp_alarm
                              msg); */
        }

        HAL_DelayMs(16);
    }
}
