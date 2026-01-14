/*
// hal_spi_target.c
#include "hal_spi.h"
#include "S32K118.h"
#include "lpspi_driver.h"
#include "clock_manager.h"

#define SPI_INSTANCE      0U  // Usaremos LPSPI0

// Estructura de configuración
static lpspi_master_config_t spiConfig;

void HAL_SPI_Init(void) {
    // Configuración base
    LPSPI_MasterGetDefaultConfig(&spiConfig);
    spiConfig.bitsPerSec = 8000000U;    // 8 MHz
    spiConfig.whichPcs = LPSPI_PCS0;    // Chip select 0
    spiConfig.cpol = kLPSPI_ClockPolarity_ActiveHigh;
    spiConfig.cpha = kLPSPI_ClockPhase_FirstEdge;
    spiConfig.direction = kLPSPI_MsbFirst;

    // Inicializa LPSPI0
    LPSPI_MasterInit(LPSPI0, &spiConfig, CLOCK_GetFreq(kCLOCK_BusClk));
}

void HAL_SPI_WriteCommand(uint8_t cmd) {
    lpspi_transfer_t xfer;
    xfer.txData = &cmd;
    xfer.rxData = NULL;
    xfer.dataSize = 1;
    xfer.configFlags = LPSPI_MASTER_PCS(0) | LPSPI_MASTER_PCS_CONTINUOUS;
    LPSPI_MasterTransferBlocking(LPSPI0, &xfer);
}

void HAL_SPI_WriteData(const uint8_t *data, size_t length) {
    lpspi_transfer_t xfer;
    xfer.txData = (uint8_t *)data;
    xfer.rxData = NULL;
    xfer.dataSize = length;
    xfer.configFlags = LPSPI_MASTER_PCS(0);
    LPSPI_MasterTransferBlocking(LPSPI0, &xfer);
}

void HAL_SPI_TransmitByte(uint8_t byte) {
    HAL_SPI_WriteData(&byte, 1);
}
    */