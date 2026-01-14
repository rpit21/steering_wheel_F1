/*

#include "hal_gpio.h"
#include "S32K118.h"

#define PORTB_CLK   PCC_PORTB_INDEX
#define PORTC_CLK   PCC_PORTC_INDEX

#define TFT_CS_PIN   6
#define TFT_DC_PIN   7
#define TFT_RST_PIN  8
#define LED_PIN      0

#define BTN1_PORT  PORTC
#define BTN1_GPIO  PTC
#define BTN1_PIN   3
#define BTN2_PORT  PORTC
#define BTN2_GPIO  PTC
#define BTN2_PIN   4
#define BTN3_PORT  PORTC
#define BTN3_GPIO  PTC
#define BTN3_PIN   5
#define BTN4_PORT  PORTC
#define BTN4_GPIO  PTC
#define BTN4_PIN   6

void HAL_GPIO_Init(void) {
    // --- Habilita relojes ---
    PCC->PCCn[PORTB_CLK] |= PCC_PCCn_CGC_MASK;
    PCC->PCCn[PORTC_CLK] |= PCC_PCCn_CGC_MASK;

    // --- Salidas ---
    PORTB->PCR[TFT_CS_PIN]  = PORT_PCR_MUX(1);
    PORTB->PCR[TFT_DC_PIN]  = PORT_PCR_MUX(1);
    PORTB->PCR[TFT_RST_PIN] = PORT_PCR_MUX(1);
    PORTC->PCR[LED_PIN]     = PORT_PCR_MUX(1);

    PTB->PDDR |= (1<<TFT_CS_PIN) | (1<<TFT_DC_PIN) | (1<<TFT_RST_PIN);
    PTC->PDDR |= (1<<LED_PIN);

    // --- Entradas (botones con pull-up) ---
    BTN1_PORT->PCR[BTN1_PIN] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    BTN2_PORT->PCR[BTN2_PIN] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    BTN3_PORT->PCR[BTN3_PIN] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    BTN4_PORT->PCR[BTN4_PIN] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

    BTN1_GPIO->PDDR &= ~(1<<BTN1_PIN);
    BTN2_GPIO->PDDR &= ~(1<<BTN2_PIN);
    BTN3_GPIO->PDDR &= ~(1<<BTN3_PIN);
    BTN4_GPIO->PDDR &= ~(1<<BTN4_PIN);
}

void HAL_GPIO_Write(GPIO_Pin_t pin, uint8_t value) {
    switch (pin) {
        case GPIO_TFT_CS:  value ? (PTB->PSOR |= (1<<TFT_CS_PIN)) : (PTB->PCOR |= (1<<TFT_CS_PIN)); break;
        case GPIO_TFT_DC:  value ? (PTB->PSOR |= (1<<TFT_DC_PIN)) : (PTB->PCOR |= (1<<TFT_DC_PIN)); break;
        case GPIO_TFT_RST: value ? (PTB->PSOR |= (1<<TFT_RST_PIN)) : (PTB->PCOR |= (1<<TFT_RST_PIN)); break;
        case GPIO_LED_STATUS: value ? (PTC->PSOR |= (1<<LED_PIN)) : (PTC->PCOR |= (1<<LED_PIN)); break;
        default: break;
    }
}

void HAL_GPIO_Toggle(GPIO_Pin_t pin) {
    switch (pin) {
        case GPIO_LED_STATUS: PTC->PTOR = (1<<LED_PIN); break;
        default: break;
    }
}

// --- Interna: lectura para botones ---
uint8_t HAL_GPIO_Read(GPIO_Pin_t pin) {
    switch (pin) {
        case GPIO_BTN_1: return !((BTN1_GPIO->PDIR >> BTN1_PIN) & 1);
        case GPIO_BTN_2: return !((BTN2_GPIO->PDIR >> BTN2_PIN) & 1);
        case GPIO_BTN_3: return !((BTN3_GPIO->PDIR >> BTN3_PIN) & 1);
        case GPIO_BTN_4: return !((BTN4_GPIO->PDIR >> BTN4_PIN) & 1);
        default: return 0;
    }
}


*/