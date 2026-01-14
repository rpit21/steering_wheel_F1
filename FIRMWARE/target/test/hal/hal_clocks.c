#include "device_registers.h" 	/* include peripheral declarations S32K118 */
#include "hal_clocks.h"

/* * NOTA: Non includiamo hal_uart.h qui per evitare dipendenze circolari
 * o l'uso improprio di printf nelle funzioni di clock critiche.
 */

void SOSC_init_20MHz(void)
{
	/*! SOSC Initialization (20 MHz External Crystal):
	 * ===============================================
     * Configurazione basata su cristallo esterno Y1 da 20.000 MHz.
	 * */

    /* 1. Unlock SOSC Control Status Register */
    while(IP_SCG->SOSCCSR & SCG_SOSCCSR_LK_MASK);

    /* 2. Disable SOSC to configure it */
    IP_SCG->SOSCCSR &= ~SCG_SOSCCSR_SOSCEN_MASK;

    /* 3. Configure Dividers */
	IP_SCG->SOSCDIV = SCG_SOSCDIV_SOSCDIV1(1)|
				      SCG_SOSCDIV_SOSCDIV2(1);  	/* Divide by 1 */

    /* 4. Configure Configuration Register */
	IP_SCG->SOSCCFG  =	SCG_SOSCCFG_RANGE(3)|		/* Range=3: High freq (8MHz - 40MHz) -> OK per 20MHz */
					    SCG_SOSCCFG_EREFS_MASK;		/* EREFS=1: Input is external XTAL (Oscillator) */

    /* 5. Enable SOSC */
    IP_SCG->SOSCCSR = SCG_SOSCCSR_SOSCEN_MASK; 		/* Enable oscillator */
                                                    /* Altri bit a 0 per default (Monitor disabled, etc.) */

    /* 6. Wait for SOSC valid */
    while(!(IP_SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK));
}

void RUN_mode_48MHz(void)
{
    /* * S32K118 Clock Tree Configuration:
     * Source: FIRC (Fast Internal Reference Clock) @ 48 MHz
     * Core:   48 MHz
     * Bus:    48 MHz
     * Slow:   24 MHz (Flash limit)
     */

    /* 1. Ensure FIRC is enabled and valid before switching */
    /* Unlock FIRC CSR if locked */
    while(IP_SCG->FIRCCSR & SCG_FIRCCSR_LK_MASK);

    /* Enable FIRC (se non lo fosse già) */
    IP_SCG->FIRCCSR |= SCG_FIRCCSR_FIRCEN_MASK;

    /* Wait for FIRC to be valid */
    while(!(IP_SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK));


    /* 2. Configure FIRC Dividers (Outputs to peripherals) */
    IP_SCG->FIRCDIV = SCG_FIRCDIV_FIRCDIV1(1) | SCG_FIRCDIV_FIRCDIV2(1); // Div by 1


    /* 3. Configure SIRC Dividers (Outputs to peripherals) - Optional but good practice */
    IP_SCG->SIRCDIV = SCG_SIRCDIV_SIRCDIV1(1) | SCG_SIRCDIV_SIRCDIV2(1);


    /* 4. Select FIRC (48MHz) as System Clock source */
    /* SCS = 3 (FIRC)
       DIVCORE = 0 (Div by 1) -> Core = 48 MHz
       DIVBUS  = 0 (Div by 1) -> Bus  = 48 MHz (Max for S32K118)
       DIVSLOW = 1 (Div by 2) -> Slow = 24 MHz (Max for Flash is 24-25MHz)
    */
    IP_SCG->RCCR = SCG_RCCR_SCS(3)
                 | SCG_RCCR_DIVCORE(0b00)
                 | SCG_RCCR_DIVBUS(0b00)
                 | SCG_RCCR_DIVSLOW(0b01);

    /* 5. Wait until FIRC is officially the system clock */
    while (((IP_SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT) != 3) {}
}
