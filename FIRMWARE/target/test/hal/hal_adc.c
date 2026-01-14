// This file contains the Hardware Abstraction Layer (HAL) implementation for the
// Analog-to-Digital Converter (ADC) specifically for the NXP S32K118 target microcontroller.
//
// NOTE: This implementation is currently a placeholder/skeleton. The code lines
// are commented out and serve as a template for what needs to be implemented
// using the specific SDK or register definitions for the S32K microcontroller.

#include "device_registers.h"
#include "hal_adc.h"
#include "hal_uart.h"

/* ============================================================================
 * 							ADC PIN DEFINITIONS
 * ========================================================================== */
/* Configuration for S32K118:
 * - Rotary Switch: PTC14 ---> Correspond to ADC0_SE12 (Channel 12)
 * - Clutch:        PTC15 ---> Correspond to ADC0_SE13 (Channel 13)
 */

// Definition for the Rotary Switch (PTC14)
#define ADC_ROTARY_PORT   IP_PORTC
#define ADC_ROTARY_PIN    14

// Definition for the Clutch (PTC15)
#define ADC_CLUTCH_PORT   IP_PORTC
#define ADC_CLUTCH_PIN    15


void hal_adc_init(void)
{

	/*--- ADC INPUT CONFIGURATION (ANALOG PINS) ---*/

	//Enable PORTC clock
	IP_PCC -> PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK; 	/* Enable clock to PORT C */

	//Set MUX = 0 → analog function (disconnect digital GPIO logic) */
	ADC_CLUTCH_PORT -> PCR[ADC_CLUTCH_PIN] &= ~PORT_PCR_MUX_MASK;
	ADC_CLUTCH_PORT -> PCR[ADC_CLUTCH_PIN] |= PORT_PCR_MUX(0);

	ADC_ROTARY_PORT -> PCR[ADC_ROTARY_PIN] &= ~PORT_PCR_MUX_MASK;
	ADC_ROTARY_PORT -> PCR[ADC_ROTARY_PIN] |= PORT_PCR_MUX(0);
	    /* Note:
	     * - Disable pull-up/pull-down: the signal is from a potentiometer/sensor.
	     * - Port: Direction= Input (by default). Is not necessary configure PDDR
	     */



    /*--- Clock configuration for the ADC0 module ADC0 (PCC)---*/
     /*
     * Without clock the module is disable.
     * PCC = Peripheral Clock Controller
     * PCC_ADC0_INDEX is the index for the ADC0.
     */

    IP_PCC->PCCn[PCC_ADC0_INDEX] &= ~PCC_PCCn_CGC_MASK;  /* Disable clocks to modify PCS (default)*/

    //Selection of the source clock and enable
    IP_PCC->PCCn[PCC_ADC0_INDEX] |=	PCC_PCCn_PCS(3)   	| /* PCS(3) Clock source = FIRC_DIV2 (48 MHz) */
									PCC_PCCn_CGC_MASK;    /* Clock Gate Control: clock enabled */



    /*--- Disable the ADC module for configuration---*/
    /*
     * Put all  1 in ADCH for disabling the conversion on SC1.
     * SC1[0] is used for the SW trigger modality.
     */
    IP_ADC0->SC1[0] = ADC_SC1_ADCH(0x1F);


    /*----Configuration of ADC_CFG1: resolution, internal clock, prescaler.
     *
     * MODE(1) → 12-bit (per S32K1xx, 00=8bit, 01=12bit, 10=10bit nei reference; verifica nel RM).
     * ADICLK(0) → clock bus/FIRC_DIV2 (0 default).
     * ADIV(0) → divide by 1 (non prescaler).
     */
    IP_ADC0->CFG1 =
            ADC_CFG1_MODE(1)   |   /* 12-bit conversion */
            ADC_CFG1_ADICLK(0) |   /* Input clock: default clock source (48 MHz) */
            ADC_CFG1_ADIV(0);      /* Clock divider: /1 */


    /*--- Configuration ADC_CFG2: sample time ---*/
    /*
     * SMPLTS = 0 → default sample time (13 cycles of ADCK=48MHz)
     * For signals with high impedance you could lengthen it. :contentReference[oaicite:3]{index=3}
     */
    IP_ADC0->CFG2 =
            ADC_CFG2_SMPLTS(12);    /* 12+1=13 ADCK sample time */


    /*---Configuration SC2: trigger and reference
     *  - ADTRG = 0 → software trigger
     *  - REFSEL = 0 → VREFH / VREFL external as reference
     *  - Without comparison, without DMA.
     */
    IP_ADC0->SC2 = 0x00000000u;

    /*---Configuration SC3: non averaging, non automatic calibration---*/
    /*The simpler behavior
     */
    IP_ADC0->SC3 = 0x00000000u;

    /* From this moment ADC0 can receive conversions commands
     * transmit the value on SC1[0].
     */
    HAL_UART_Printf(
        "[ADC] ADC0 init: CFG1=0x%08lX CFG2=0x%08lX SC2=0x%08lX SC3=0x%08lX\r\n",
        (uint32_t)IP_ADC0->CFG1,
        (uint32_t)IP_ADC0->CFG2,
        (uint32_t)IP_ADC0->SC2,
        (uint32_t)IP_ADC0->SC3);
}

// `hal_adc_read`: Starts an ADC conversion on a specific channel, waits for it to
// complete, and returns the digital result.
//   `channel`: The ADC input channel number to read from.
// Returns a 16-bit unsigned integer representing the raw digital value.
uint16_t hal_adc_read(uint8_t channel)
{
    /*---Channel selection and conversion start---*/
     /*
     * Write in SC1[0] do two things:
     *  - Put ADCH = channel → input analog selection
     *  - Start a new conversion (ADTRG=0 → SW trigger)
     */
    IP_ADC0->SC1[0] = ADC_SC1_ADCH(channel);

    /*---Wait until the conversion is finished---*/
    /*
     *  COCO (Conversion Complete) in SC1[0] is 1 when the result is already on R[0]
     * This is “busy-wait”: CPU block until the conversion is finished
     */
    while ((IP_ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0u)
    {
        /* wait until conversion complete */
    }

    /*---Read of the data register----*/
    uint16_t result = (uint16_t)IP_ADC0->R[0];  /* R[0] contains the value of the conversion.*/

    return result;
}
