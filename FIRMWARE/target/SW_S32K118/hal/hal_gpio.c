#include "device_registers.h" 	/* Peripheral declarations for S32K118 */
#include "hal_gpio.h"
#include "hal_uart.h"


/* ============================================================================
 *                           CLOCK CONTROL CONSTANTS
 * ========================================================================== */
#define PORTB_CLK   PCC_PORTB_INDEX
#define PORTC_CLK   PCC_PORTC_INDEX
#define PORTA_CLK	PCC_PORTA_INDEX


/* ============================================================================
 *                           TFT DISPLAY PIN DEFINITIONS
 * ========================================================================== */

#define TFT_CS_PIN   0	//PTB0
#define TFT_DC_PIN   8	//PTC8
#define TFT_RST_PIN  9	//PTC9

/* ============================================================================
 *                              LED PIN DEFINITIONS
 * ========================================================================== */

#define LED_Y_PIN	1	//PTA1
#define LED_R_PIN	5	//PTB5

/* ============================================================================
 *                           BUTTON PIN DEFINITIONS
 * ========================================================================== */
/*
 * Buttons are wired to GND and use the MCU's internal pull-up resistors.
 *
 * Electrical wiring:
 *
 *      VCC
 *       ↑   (internal pull-up enabled)
 *       |
 *   Pin ----[ BUTTON ]---- GND
 *
 * Electrical behavior:
 *   Released  → pin = 1 (HIGH)
 *   Pressed   → pin = 0 (LOW)
 *
 * Firmware logic:
 * We invert the input in HAL_GPIO_Read() so the application receives:
 *
 *   Released  → 0
 *   Pressed   → 1
 *
 * This provides "active-high" behavior at the application level,
 * while keeping the usual pull-up wiring on hardware.
 */
#define BTN1_PORT  IP_PORTC
#define BTN1_GPIO  IP_PTC
#define BTN1_PIN   1

#define BTN2_PORT  IP_PORTB
#define BTN2_GPIO  IP_PTB
#define BTN2_PIN   4

#define BTN3_PORT  IP_PORTC
#define BTN3_GPIO  IP_PTC
#define BTN3_PIN   16

#define BTN4_PORT  IP_PORTB
#define BTN4_GPIO  IP_PTB
#define BTN4_PIN   3


void HAL_GPIO_Init(void) {

	/*---Enable PORT clock gating (required before configuring PCR)---*/
	IP_PCC->PCCn[PORTA_CLK] |= PCC_PCCn_CGC_MASK;	/* Enable clock to PORT A  */
    IP_PCC->PCCn[PORTB_CLK] |= PCC_PCCn_CGC_MASK; 	/* Enable clock to PORT B  */
    IP_PCC->PCCn[PORTC_CLK] |= PCC_PCCn_CGC_MASK; 	/* Enable clock to PORT C */


    /*--- TFT DISPLAY OUTPUT CONFIGURATION---*/

    /* Configure directions */
    IP_PTB->PDDR |= (1<<TFT_CS_PIN); 					// Port B (CS) as output
    IP_PTC->PDDR |= (1<<TFT_DC_PIN) | (1<<TFT_RST_PIN); /* Port C (DC & RST) as output */

    /* Configure Port PCR as GPIO */
    IP_PORTB->PCR[TFT_CS_PIN]  = PORT_PCR_MUX(1);
    IP_PORTC->PCR[TFT_DC_PIN]  = PORT_PCR_MUX(1);
    IP_PORTC->PCR[TFT_RST_PIN] = PORT_PCR_MUX(1);

    /* Initialize outputs to LOW to avoid glitches*/
    IP_PTB->PSOR |= (1 << TFT_CS_PIN);
    IP_PTC->PCOR |= (1 << TFT_DC_PIN);   /* DC LOW  */
    IP_PTC->PSOR |= (1 << TFT_RST_PIN);  /* RST HIGH */


    /*--- LED OUTPUT CONFIGURATION ---*/

    IP_PTA->PDDR |= (1<<LED_Y_PIN); 			/* Port A (Led Yellow): Data Direction= output */
    IP_PORTA->PCR[LED_Y_PIN] = PORT_PCR_MUX(1); // Ports: MUX = GPIO
    IP_PTA->PCOR |= (1 << LED_Y_PIN);   		/* LED initially OFF */

    IP_PTB->PDDR |= (1<<LED_R_PIN); 			/* Port B (Led RED): Data Direction= output */
    IP_PORTB->PCR[LED_R_PIN] = PORT_PCR_MUX(1); // Ports: MUX = GPIO
    IP_PTB->PCOR |= (1 << LED_R_PIN);   		/* LED initially OFF */


    /* -------  BUTTON INPUT CONFIGURATION ------*/

    // Port: Direction= Input (default)
    BTN1_GPIO->PDDR &= ~(1<<BTN1_PIN);
    BTN2_GPIO->PDDR &= ~(1<<BTN2_PIN);
    BTN3_GPIO->PDDR &= ~(1<<BTN3_PIN);
    BTN4_GPIO->PDDR &= ~(1<<BTN4_PIN);

    // Ports: MUX = GPIO, Pull Enable= On, Pull UP
    BTN1_PORT->PCR[BTN1_PIN] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS(1);
    BTN2_PORT->PCR[BTN2_PIN] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS(1);
    BTN3_PORT->PCR[BTN3_PIN] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS(1);
    BTN4_PORT->PCR[BTN4_PIN] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS(1);

    HAL_UART_Printf(
        "[GPIO] TFT: PORTB_PCR[%d]=0x%08lX, PORTC_PCR[%d]=0x%08lX, PORTC_PCR[%d]=0x%08lX\r\n",
        TFT_CS_PIN,  (uint32_t)IP_PORTB->PCR[TFT_CS_PIN],
        TFT_DC_PIN,  (uint32_t)IP_PORTC->PCR[TFT_DC_PIN],
        TFT_RST_PIN, (uint32_t)IP_PORTC->PCR[TFT_RST_PIN]);

    HAL_UART_Printf(
        "[GPIO] BTN PCR: B1=0x%08lX B2=0x%08lX B3=0x%08lX B4=0x%08lX\r\n",
        (uint32_t)BTN1_PORT->PCR[BTN1_PIN],
        (uint32_t)BTN2_PORT->PCR[BTN2_PIN],
        (uint32_t)BTN3_PORT->PCR[BTN3_PIN],
        (uint32_t)BTN4_PORT->PCR[BTN4_PIN]);

    HAL_UART_Printf(
        "[GPIO] LED PCR: Y=0x%08lX R=0x%08lX\r\n",
        (uint32_t)IP_PORTA->PCR[LED_Y_PIN],
        (uint32_t)IP_PORTB->PCR[LED_R_PIN]);

}

/* ============================================================================
 *                           PIN OUTPUT OPERATIONS
 * ========================================================================== */

void HAL_GPIO_Write(GPIO_Pin_t pin, uint8_t value) {
    switch (pin) {
        case GPIO_TFT_CS:  	value ? (IP_PTB->PSOR |= (1<<TFT_CS_PIN)) 	: (IP_PTB->PCOR |= (1<<TFT_CS_PIN)); break;
        case GPIO_TFT_DC:  	value ? (IP_PTC->PSOR |= (1<<TFT_DC_PIN)) 	: (IP_PTC->PCOR |= (1<<TFT_DC_PIN)); break;
        case GPIO_TFT_RST: 	value ? (IP_PTC->PSOR |= (1<<TFT_RST_PIN)) 	: (IP_PTC->PCOR |= (1<<TFT_RST_PIN)); break;
        case GPIO_LED_S1: 	value ? (IP_PTA->PSOR |= (1<<LED_Y_PIN)) 	: (IP_PTA->PCOR |= (1<<LED_Y_PIN)); break;
        case GPIO_LED_S2: 	value ? (IP_PTB->PSOR |= (1<<LED_R_PIN)) 	: (IP_PTB->PCOR |= (1<<LED_R_PIN)); break;
        default: break;
    }
}

void HAL_GPIO_Toggle(GPIO_Pin_t pin) {
    switch (pin) {
        case GPIO_LED_S1: IP_PTA->PTOR = (1<<LED_Y_PIN); break;
        case GPIO_LED_S2: IP_PTB->PTOR = (1<<LED_R_PIN); break;
        default: break;
    }
}

/* ============================================================================
 *                              INPUT READING
 * ========================================================================== */
uint8_t HAL_GPIO_Read(GPIO_Pin_t pin) {
    switch (pin) {
        case GPIO_BTN_1: return ((BTN1_GPIO->PDIR >> BTN1_PIN) & 1);
        case GPIO_BTN_2: return ((BTN2_GPIO->PDIR >> BTN2_PIN) & 1);
        case GPIO_BTN_3: return ((BTN3_GPIO->PDIR >> BTN3_PIN) & 1);
        case GPIO_BTN_4: return ((BTN4_GPIO->PDIR >> BTN4_PIN) & 1);
        default: return 0;
    }
}

