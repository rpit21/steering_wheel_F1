#include "device_registers.h"

/* Definizioni per la tua Custom Board */
#define PTB5 (5)    /* LED 1 */
#define PTA1 (1)    /* LED 2 */
#define PTC1 (1)    /* Pulsante */

int main(void)
{
    int counter = 0;

    /* NOTA: Il Watchdog è già disabilitato in 'SystemInit' (startup code).
     * Non serve disabilitarlo di nuovo qui. */

    /* --------------------------------------------------------- */
    /* 1. ABILITAZIONE CLOCK (Passaggio Fondamentale)            */
    /* --------------------------------------------------------- */
    /* Attiviamo TUTTE le porte che useremo: A, B e C.
       Senza PCC_PORTA_INDEX, il chip va in crash appena tocchi PTA1 */
    IP_PCC->PCCn[PCC_PORTA_INDEX] = PCC_PCCn_CGC_MASK;
    IP_PCC->PCCn[PCC_PORTB_INDEX] = PCC_PCCn_CGC_MASK;
    IP_PCC->PCCn[PCC_PORTC_INDEX] = PCC_PCCn_CGC_MASK;

    /* --------------------------------------------------------- */
    /* 2. CONFIGURAZIONE PIN                                     */
    /* --------------------------------------------------------- */

    /* Configura Pulsante su PTC1 (Input con Pull-Up) */
    IP_PTC->PDDR &= ~(1 << PTC1);
    IP_PORTC->PCR[PTC1] = PORT_PCR_MUX(1) | PORT_PCR_PFE_MASK | PORT_PCR_PE_MASK | PORT_PCR_PS(1);

    /* Configura LED su PTB5 (Output) */
    IP_PTB->PDDR |= (1 << PTB5);
    IP_PORTB->PCR[PTB5] = PORT_PCR_MUX(1);

    /* Configura LED su PTA1 (Output) */
    IP_PTA->PDDR |= (1 << PTA1);
    IP_PORTA->PCR[PTA1] = PORT_PCR_MUX(1);

    /* Spegni i LED all'inizio (Reset stato noto) */
    IP_PTB->PCOR |= (1 << PTB5); /* PCOR = Clear = Spegni (se Active High) */
    IP_PTA->PCOR |= (1 << PTA1);

    /* --------------------------------------------------------- */
    /* 3. CICLO INFINITO                                         */
    /* --------------------------------------------------------- */
    for(;;)
    {
        /* Fai lampeggiare PTB5 (Toggle) ogni ciclo per vedere che il codice gira */
        IP_PTB->PTOR |= (1 << PTB5);

        /* Logica Pulsante su PTA1 */
        /* Se il pin è ALTO (pull-up attivo, pulsante NON premuto se chiude a massa) */
        if (IP_PTC->PDIR & (1 << PTC1)) {
            IP_PTA->PSOR |= (1 << PTA1); /* Accendi LED A */
        } else {
            IP_PTA->PCOR |= (1 << PTA1); /* Spegni LED A */
        }

        /* Ritardo software semplice per vedere il lampeggio */
        for(volatile int i = 0; i < 1000000; i++) {
            __asm("nop");
        }

        counter++;
    }
}
