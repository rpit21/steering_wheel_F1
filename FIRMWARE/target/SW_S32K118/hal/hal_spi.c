#include "hal_spi.h"
#include "hal_gpio.h"
#include "device_registers.h"
#include "hal_uart.h"

#define LPSPI_PCC_INDEX   PCC_LPSPI0_INDEX
#define SCK_PIN   2   /* PTB2 */
#define SOUT_PIN  1   /* PTB1 */

void HAL_SPI_Init(void) {

    /* 1. MUXING */
    IP_PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK;
    IP_PORTB->PCR[SCK_PIN] &= ~PORT_PCR_MUX_MASK;
    IP_PORTB->PCR[SCK_PIN] |= PORT_PCR_MUX(3);
    IP_PORTB->PCR[SOUT_PIN] &= ~PORT_PCR_MUX_MASK;
    IP_PORTB->PCR[SOUT_PIN] |= PORT_PCR_MUX(3);

    /* 2. CLOCK */
    IP_PCC->PCCn[LPSPI_PCC_INDEX] &= ~PCC_PCCn_CGC_MASK;
    IP_PCC->PCCn[LPSPI_PCC_INDEX] &= ~PCC_PCCn_PCS_MASK;
    IP_PCC->PCCn[LPSPI_PCC_INDEX] |= PCC_PCCn_PCS(1); /* SOSC */
    IP_PCC->PCCn[LPSPI_PCC_INDEX] |= PCC_PCCn_CGC_MASK;

    /* 3. RESET & CONFIG (MEN=0) */
    IP_LPSPI0->CR = 0x00000000;          // Reset Totale
    IP_LPSPI0->CR = LPSPI_CR_RST_MASK;   // Fuori dal Reset, ma MEN=0 (per scrivere CFGR1)

    while(IP_LPSPI0->CR & LPSPI_CR_MEN_MASK); // Sicurezza

    IP_LPSPI0->CFGR1 = LPSPI_CFGR1_MASTER_MASK;

    /* Baud 3 MHz */
    IP_LPSPI0->CCR = LPSPI_CCR_SCKDIV(5) | LPSPI_CCR_DBT(20) | LPSPI_CCR_SCKPCS(20) | LPSPI_CCR_PCSSCK(20);

    /* 4. ENABLE (CRUCIALE: MEN=1 AND RST=1) */
    /* Nel tuo codice mancava LPSPI_CR_RST_MASK qui sotto! */
    IP_LPSPI0->CR = LPSPI_CR_MEN_MASK | LPSPI_CR_DBGEN_MASK | LPSPI_CR_RST_MASK;

    /* 5. TDR CONFIG */
    IP_LPSPI0->TCR = LPSPI_TCR_PRESCALE(0) | LPSPI_TCR_FRAMESZ(7);

    IP_LPSPI0 -> FCR &= ~LPSPI_FCR_RXWATER_MASK;  				/* RXWATER = 0 The Receive Data Flag is set whenever the number of words in the receive FIFO is greater than 0 */
    IP_LPSPI0 -> FCR &= ~LPSPI_FCR_TXWATER_MASK;  				/* TXWATER = 0 The Transmit Data Flag is set whenever the number of words in the transmit FIFO is equal or less than 0 */
    HAL_UART_Printf("[SPI] Init Done. CFGR1=0x%X CR=0x%X\r\n", IP_LPSPI0->CFGR1, IP_LPSPI0->CR);
}

void HAL_SPI_WriteCommand(uint8_t cmd) {
    /* SWAP GIA' GESTITO IN HAL_GPIO_WRITE: TFT_DC ora punta al pin fisico corretto */
    HAL_GPIO_Write(GPIO_TFT_DC, 0);
    HAL_SPI_TransmitByte(cmd);
}

void HAL_SPI_WriteData(const uint8_t *data, size_t length) {
    HAL_GPIO_Write(GPIO_TFT_DC, 1); /* Aggiunto per sicurezza */
    for (size_t i = 0; i < length; i++) {
        HAL_SPI_TransmitByte(data[i]);
    }
}

void HAL_SPI_TransmitByte(uint8_t byte)
{

	/* Wait for Tx FIFO available */
	while((IP_LPSPI0 -> SR & LPSPI_SR_TDF_MASK) >> LPSPI_SR_TDF_SHIFT == 0);

	IP_LPSPI0 -> TDR = byte;              					/* Transmit data */
	IP_LPSPI0 -> SR |= LPSPI_SR_TDF_MASK; 					/* Clear TDF flag */

	/* Wait at least one RxFIFO entry */
	//while((IP_LPSPI0 -> SR & LPSPI_SR_RDF_MASK) >> LPSPI_SR_RDF_SHIFT == 0);

	//receive = IP_LPSPI0 -> RDR;            				/* Read received data */
	//IP_LPSPI0 -> SR |= LPSPI_SR_RDF_MASK; 					/* Clear RDF flag */

}
