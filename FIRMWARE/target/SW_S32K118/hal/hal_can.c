#include "hal_can.h"
#include "device_registers.h"
#include <stddef.h>
#include "hal_uart.h"



#define CAN_PORT	IP_PORTC
#define CAN_RX_PIN 	2
#define CAN_TX_PIN 	3

#define MSG_BUF_SIZE  4		/* Msg Buffer Size. (CAN 2.0AB: 1 Cfg + 1 ID + 2 data= 4 words) */
#define TX_MB_IDX  0
#define RX_MB_IDX  1

int hal_can_init(const char* interface_name)
{
    (void)interface_name;
    HAL_UART_Printf("CAN init: 1-\r\n");

    /* 1. Config PIN */
    IP_PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;
    CAN_PORT->PCR[CAN_RX_PIN] &= ~PORT_PCR_MUX_MASK;
    CAN_PORT->PCR[CAN_RX_PIN] |= PORT_PCR_MUX(3);
    CAN_PORT->PCR[CAN_TX_PIN] &= ~PORT_PCR_MUX_MASK;
    CAN_PORT->PCR[CAN_TX_PIN] |= PORT_PCR_MUX(3);
    HAL_UART_Printf("CAN init: 2- Mux PIN\r\n");


    /* ==== 2. Select CAN Clock Source (48 MHz bus clock) ==== */
    IP_PCC->PCCn[PCC_FlexCAN0_INDEX] &= ~PCC_PCCn_PCS_MASK;
    IP_PCC->PCCn[PCC_FlexCAN0_INDEX] |=  PCC_PCCn_PCS(0b00);   // Bus clock (48MHz)
    IP_PCC->PCCn[PCC_FlexCAN0_INDEX] |=  PCC_PCCn_CGC_MASK;    // Enable clock
    HAL_UART_Printf("PCC CAN = 0x%08X\r\n", IP_PCC->PCCn[PCC_FlexCAN0_INDEX]);


    /* ==== 3. Disable module before changing CLKSRC ==== */
    IP_FLEXCAN0->MCR |= FLEXCAN_MCR_MDIS_MASK;			// Disable the module
    IP_FLEXCAN0->CTRL1 |= FLEXCAN_CTRL1_CLKSRC_MASK; 	// CLKSRC=1 → SYSCLK = 48MHz
    IP_FLEXCAN0->MCR &= ~FLEXCAN_MCR_MDIS_MASK;      	// Enable module (FRZ+HALT set)

    HAL_UART_Printf("CAN init: 3-MDIS cleared\r\n");

    /* Wait for freeze acknowledge */
    while (!(IP_FLEXCAN0->MCR & FLEXCAN_MCR_FRZACK_MASK)) {}
    HAL_UART_Printf("CAN init: 4-FRZACK=1\r\n");

    /* ==== 4. Configure Bit Timing for 48 MHz → 500 kbps ==== */
        /*
         * CALCOLI DETTAGLIATI:
         * Clock Source = 48 MHz
         * Target Baud  = 500 kHz
         * Ratio        = 48M / 500k = 96 clock cycles per bit
         *
         * Usiamo un Prescaler di 6 (brp=6):
         * TQ Clock = 48 MHz / 6 = 8 MHz (1 TQ = 125 ns)
         * Total TQ = 96 / 6 = 16 TQ per bit
         *
         * Configurazione Segmenti (da specifica adapter):
         * Sync Seg = 1 TQ (fisso)
         * Prop Seg = 6 TQ
         * Pseg 1   = 7 TQ
         * Pseg 2   = 2 TQ
         * Total    = 1 + 6 + 7 + 2 = 16 TQ
         *
         * Sample Point = (Sync + Prop + Pseg1) / Total
         * = (1 + 6 + 7) / 16 = 14 / 16 = 0.875 (87.5%) -> MATCH EXACT!
         *
         * NOTA REGISTRI: FlexCAN richiede (Valore - 1)
         */
        IP_FLEXCAN0->CTRL1 =
              FLEXCAN_CTRL1_CLKSRC_MASK      /* Usa bus clock (48 MHz) */
            | FLEXCAN_CTRL1_SMP(0)           /* Triple Sampling (raccomandato per basse velocità/rumore, opzionale 0) */
            | FLEXCAN_CTRL1_PRESDIV(5)       /* Div by 6 (5+1)  -> TQ = 125ns */
            | FLEXCAN_CTRL1_PROPSEG(5)       /* 6 TQ   (5+1) */
            | FLEXCAN_CTRL1_PSEG1(6)         /* 7 TQ   (6+1) */
            | FLEXCAN_CTRL1_PSEG2(1)         /* 2 TQ   (1+1) */
            | FLEXCAN_CTRL1_RJW(0);          /* SJW = 1 TQ (0+1) - Resync jump width */

        HAL_UART_Printf("CAN init: 6-CTRL1 set for 500k 87.5%%\r\n");
    // --- REFERENCE: Bit timing para FlexCAN0 con F_CAN = 20 MHz, 500 kbit/s ---
    // if it is use CAN a with a soource of 20 MHz:
    //
    // IP_FLEXCAN0->CTRL1 =
    //       FLEXCAN_CTRL1_PRESDIV(3)   // PRESDIV = 3 → divide 4
    //     | FLEXCAN_CTRL1_PSEG1(5)     // PSEG1 = 5
    //     | FLEXCAN_CTRL1_PSEG2(2)     // PSEG2 = 2
    //     | FLEXCAN_CTRL1_PROPSEG(2)   // PROPSEG = 2
    //     | FLEXCAN_CTRL1_RJW(1)       // RJW = 1 (2 TQ)
    //     | FLEXCAN_CTRL1_SMP(1)       // triple sampling

    HAL_UART_Printf("CAN init: 6-CTRL1 set\r\n");

    /* Configuration of the quantity of mailboxes (0..15 → 16 MBs) */
    IP_FLEXCAN0->MCR &= ~FLEXCAN_MCR_MAXMB_MASK;
    IP_FLEXCAN0->MCR |= FLEXCAN_MCR_MAXMB(15); // actually we are using 2 MB (1 Tx_MB y 1 RX_MB)

    /* ==== 5. Clear RAM ==== */
    for(int i = 0; i < 128; i++)
    {
    	IP_FLEXCAN0->RAMn[i] = 0;
    }
    HAL_UART_Printf("CAN init: 7-RAM cleared\r\n");

    /* Accept all IDs DON'T CARE */
    for(int i = 0; i < 16; i++)
    {
    	IP_FLEXCAN0->RXIMR[i] = 0x00000000;
    }

    HAL_UART_Printf("CAN init: 8-Masks set\r\n");


    IP_FLEXCAN0->RXMGMASK = 0x00000000;					/* Global acceptance mask: Don't check check all ID bits 	*/

    /* ==== 6. Configure RX Mailbox (MB1) ==== */
    IP_FLEXCAN0->RAMn[RX_MB_IDX * MSG_BUF_SIZE+ 0] = 0x04000000;  // RX, CODE=4
    IP_FLEXCAN0->RAMn[RX_MB_IDX * MSG_BUF_SIZE + 1] = 0x00000000; // ID = 0; the mask 0, it accept all the IDs

    HAL_UART_Printf("CAN init: 9-RX MB configured\r\n");

    /* ==== 7. Configure TX Mailbox (MB0) ==== */
    IP_FLEXCAN0->RAMn[TX_MB_IDX * MSG_BUF_SIZE] = 0x08000000;      // INACTIVE
    HAL_UART_Printf("CAN init: 10-TX MB configured\r\n");

    /* ==== 8. Exit freeze ==== */
    IP_FLEXCAN0->MCR &= ~(FLEXCAN_MCR_FRZ_MASK | FLEXCAN_MCR_HALT_MASK);

    //while ((IP_FLEXCAN0->MCR && FLEXCAN_MCR_FRZACK_MASK) >> FLEXCAN_MCR_FRZACK_SHIFT)  {}
      /* Good practice: wait for FRZACK to clear (not in freeze mode) */

    //while ((IP_FLEXCAN0->MCR && FLEXCAN_MCR_NOTRDY_MASK) >> FLEXCAN_MCR_NOTRDY_SHIFT)  {}
    	/* Good practice: wait for NOTRDY to clear (module ready) */

    while (IP_FLEXCAN0->MCR & FLEXCAN_MCR_FRZACK_MASK) {}
    while (IP_FLEXCAN0->MCR & FLEXCAN_MCR_NOTRDY_MASK) {}
    HAL_UART_Printf("CAN init: 11-READY\r\n");

    return 0;
}



/* ... (Le funzioni hal_can_send, receive, shutdown restano uguali) ... */
/* INCOLLA QUI SOTTO IL RESTO DEL FILE ORIGINALE SE NECESSARIO */
int hal_can_send(uint32_t id, const uint8_t* data, uint8_t len)
{
    if (len > 8) return -1;

    /* 1. Prepare Buffer: Code=8 (Inactive) for write secure */
    IP_FLEXCAN0->RAMn[TX_MB_IDX*MSG_BUF_SIZE + 0] = 0x08000000;

    /* 2. Write the ID (Standard 11-bit << 18) */
    IP_FLEXCAN0->RAMn[TX_MB_IDX*MSG_BUF_SIZE + 1] = (id << 18);

    /* Prepare data of 2word=4Bytes=32-bit */
    uint32_t data_word0 = 0;
    uint32_t data_word1 = 0;

    if(len > 0) data_word0 |= ((uint32_t)data[0] << 24);
    if(len > 1) data_word0 |= ((uint32_t)data[1] << 16);
    if(len > 2) data_word0 |= ((uint32_t)data[2] << 8);
    if(len > 3) data_word0 |= ((uint32_t)data[3]);

    if(len > 4) data_word1 |= ((uint32_t)data[4] << 24);
    if(len > 5) data_word1 |= ((uint32_t)data[5] << 16);
    if(len > 6) data_word1 |= ((uint32_t)data[6] << 8);
    if(len > 7) data_word1 |= ((uint32_t)data[7]);

    IP_FLEXCAN0->RAMn[TX_MB_IDX*MSG_BUF_SIZE + 2] = data_word0;
    IP_FLEXCAN0->RAMn[TX_MB_IDX*MSG_BUF_SIZE + 3] = data_word1;
    HAL_UART_Printf(" [CAN TX] Sending ID=%03X, len=%u\r\n",id,len);
    /* 4. Activate the Transmission: Code=0xC, DLC=len */
    IP_FLEXCAN0->RAMn[TX_MB_IDX*MSG_BUF_SIZE + 0] = (0xC << 24) | ((uint32_t)len << 16);

    return 0;
}

int hal_can_receive(uint32_t* id, uint8_t* data, uint8_t* len)
{
    /* Check the flag interrupt (New Data) for the MB 1 */
    if (IP_FLEXCAN0->IFLAG1 & (1 << RX_MB_IDX))
    {
        uint32_t cs = IP_FLEXCAN0->RAMn[RX_MB_IDX*MSG_BUF_SIZE + 0];

        /* Read DLC and ID */
        *len = (cs >> 16) & 0x0F;
        *id = (IP_FLEXCAN0->RAMn[RX_MB_IDX*MSG_BUF_SIZE + 1] >> 18) & 0x7FF;

        /* Read Data */
        uint32_t w0 = IP_FLEXCAN0->RAMn[RX_MB_IDX*MSG_BUF_SIZE + 2];
        uint32_t w1 = IP_FLEXCAN0->RAMn[RX_MB_IDX*MSG_BUF_SIZE + 3];

        data[0] = (w0 >> 24) & 0xFF;
        data[1] = (w0 >> 16) & 0xFF;
        data[2] = (w0 >> 8)  & 0xFF;
        data[3] = (w0)       & 0xFF;
        data[4] = (w1 >> 24) & 0xFF;
        data[5] = (w1 >> 16) & 0xFF;
        data[6] = (w1 >> 8)  & 0xFF;
        data[7] = (w1)       & 0xFF;

        /* Clean the flag and unlock mailbox by reading the timer */
        IP_FLEXCAN0->IFLAG1 = (1 << RX_MB_IDX);
        volatile uint32_t dummy = IP_FLEXCAN0->TIMER;
        (void)dummy;

        IP_FLEXCAN0->RAMn[RX_MB_IDX*MSG_BUF_SIZE] = 0x04000000; // Code=4 (Active/Empty)

        return 1; // Message received
    }

    return 0; // No message
}

void hal_can_shutdown(void)
{
	/* Enter MDIS (module disabled) */
    IP_FLEXCAN0->MCR |= FLEXCAN_MCR_MDIS_MASK;

    /* Disable PCC clock gate */
    IP_PCC->PCCn[PCC_FlexCAN0_INDEX] &= ~PCC_PCCn_CGC_MASK;
}
