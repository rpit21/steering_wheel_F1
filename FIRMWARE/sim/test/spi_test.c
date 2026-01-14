#include "hal_spi.h"

void spi_test(void) {
    HAL_SPI_Init();

    HAL_SPI_WriteCommand(0x2C); // RAMWR
    uint8_t buffer[] = { 0xAA, 0xBB, 0xCC, 0xFF };
    HAL_SPI_WriteData(buffer, sizeof(buffer));
}