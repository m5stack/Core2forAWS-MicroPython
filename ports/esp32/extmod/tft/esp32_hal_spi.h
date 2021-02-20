#include "spi_type.h"

#define SPI_LSBFIRST 0
#define SPI_MSBFIRST 1

struct spi_struct_t;
typedef struct spi_struct_t spi_t;

static inline void spiWaitFinish(spi_device_t* spi) {
    while(spi->host->hal.hw->cmd.usr);
}

spi_device_handle_t spiBusInit(int8_t mosi, int8_t miso, int8_t clk, int8_t cs);
void spiInit(spi_device_t* spi, uint8_t bit_order);

void spiWriteBytesDMA(spi_device_t* spi, const uint8_t *data, uint32_t len);

void spiWriteBytes(spi_device_t* spi, const uint8_t *data_in, uint32_t len, uint8_t wait_finish);
void spiWriteU8(spi_device_t* spi, uint8_t data);
void spiWriteU16(spi_device_t* spi, uint16_t data);
void spiWriteU32(spi_device_t* spi, uint32_t data);

void spiWriteU8NoWait(spi_device_t* spi, uint8_t data);

void spiTransfer(spi_device_t* spi, uint32_t *out, uint8_t len);
uint8_t spiTransferU8(spi_device_t* spi, uint8_t data);
uint16_t spiTransferU16(spi_device_t* spi, uint16_t data);
uint32_t spiTransferU32(spi_device_t* spi, uint32_t data);
void spiTransferBytes(spi_device_t* spi, uint8_t * data, uint8_t * out, uint32_t size);
void spiTransferBits(spi_device_t* spi, uint32_t data, uint32_t * out, uint8_t bits);
void spiWaitFinish(spi_device_t* spi);
