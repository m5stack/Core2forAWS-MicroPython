#include "esp32_hal_spi.h"
#include "driver/gpio.h"
#include "driver/spi_common_internal.h"
#include "driver/periph_ctrl.h"
#include "hal/spi_ll.h"
#include "esp_log.h"
#include "esp_err.h"

#define MSB_32_SET(var, val) { uint8_t * d = (uint8_t *)&(val); (var) = d[3] | (d[2] << 8) | (d[1] << 16) | (d[0] << 24); }
#define MSB_24_SET(var, val) { uint8_t * d = (uint8_t *)&(val); (var) = d[2] | (d[1] << 8) | (d[0] << 16); }
#define MSB_16_SET(var, val) { (var) = (((val) & 0xFF00) >> 8) | (((val) & 0xFF) << 8); }
#define MSB_PIX_SET(var, val) { uint8_t * d = (uint8_t *)&(val); (var) = d[1] | (d[0] << 8) | (d[3] << 16) | (d[2] << 24); }

#define SPI_SPEED 40 * 1000 * 1000
#define DMA_CHANNEL 2

spi_host_device_t spi_host = HSPI_HOST;
spi_bus_config_t buscfg;
spi_device_interface_config_t devcfg;
spi_device_handle_t spi_handle;

// intr_handle_t intr;

// void IRAM_ATTR testIntr(void* arg) {
//     spi_dev_t *dev = (spi_dev_t *)arg;
//     dev->slave.trans_done = 0;
// }

spi_device_handle_t spiBusInit(int8_t mosi, int8_t miso, int8_t clk, int8_t cs) {
	buscfg.mosi_io_num = mosi;			// set SPI MOSI pin
    buscfg.miso_io_num = miso;			// set SPI MISO pin
	buscfg.sclk_io_num = clk;			// set SPI CLK pin
	buscfg.quadwp_io_num = -1;
	buscfg.quadhd_io_num = -1;
	buscfg.max_transfer_sz = 1024 * 20;
    buscfg.intr_flags = 0; 

	devcfg.clock_speed_hz = SPI_SPEED;    // Initial clock
	devcfg.mode = 0;                          // SPI mode 0
	devcfg.spics_io_num = cs;			        // we will use software cs select
	devcfg.queue_size = 1;
	devcfg.flags = SPI_DEVICE_NO_DUMMY;                          

    esp_err_t ret;
    ret = spi_bus_initialize(spi_host, &buscfg, DMA_CHANNEL);
    ret |= spi_bus_add_device(spi_host, &devcfg, &spi_handle);

    // esp_err_t err = esp_intr_alloc(ETS_SPI2_INTR_SOURCE, ESP_INTR_FLAG_SHARED | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_INTRDISABLED, 
    //                                 &testIntr, (void *)spi_handle->host->hal.hw, &intr);
    // if (err == ESP_OK) {
    //     esp_intr_enable(intr);
    // } else {
    //     printf("spi malloc isr share errrrrror %x\r\n", err);
    // }

    spiInit(spi_handle, SPI_MSBFIRST);
    return spi_handle;
}

void spiInit(spi_device_t* spi, uint8_t bit_order) {
    spi->host->hal.hw->user1.usr_addr_bitlen = 0;
    spi->host->hal.hw->user2.usr_command_bitlen = 0;
    spi->host->hal.hw->user.usr_addr = 0;
    spi->host->hal.hw->user.usr_command = 0;
    spi->host->hal.hw->user.usr_mosi_highpart = 0;
    spi->host->hal.hw->user.usr_miso_highpart = 0;
    spi->host->hal.hw->user.usr_mosi = 1;
    spi->host->hal.hw->user.usr_miso = 1;
    if (bit_order == SPI_MSBFIRST) {
        spi->host->hal.hw->ctrl.wr_bit_order = 0;
        spi->host->hal.hw->ctrl.rd_bit_order = 0;
    } else {
        spi->host->hal.hw->ctrl.wr_bit_order = 1;
        spi->host->hal.hw->ctrl.rd_bit_order = 1;
    }
}

void spiWriteBytes(spi_device_t* spi, const uint8_t *data_in, uint32_t len, uint8_t wait_finish) {
    if (spi == NULL || len == 0) {
        return;
    }

    uint32_t* data = (uint32_t*)data_in;
    uint32_t lens = (len + 63) / 64 - 1;
    spi->host->hal.hw->miso_dlen.usr_miso_dbitlen = 0;
    spi->host->hal.hw->mosi_dlen.usr_mosi_dbitlen = 64 * 8 - 1;
    while (lens--) {
        for (uint8_t i = 0; i < 16; i++) {
            spi->host->hal.hw->data_buf[i] = *data;
            data += 1;
        }
        spi->host->hal.hw->cmd.usr = 1;
        while (spi->host->hal.hw->cmd.usr);
    }

    lens = (len - 1) % 64 + 1;
    spi->host->hal.hw->mosi_dlen.usr_mosi_dbitlen = lens * 8 - 1;
    lens = (lens + 3) / 4;
    for (uint8_t i = 0; i < lens; i++) {
        spi->host->hal.hw->data_buf[i] = *data;
        data += 1;
    }
    spi->host->hal.hw->cmd.usr = 1;
    if (wait_finish) {
        while (spi->host->hal.hw->cmd.usr);
    }
}

void spiWriteBytesDMA(spi_device_t* spi, const uint8_t *data, uint32_t len) {
    if (spi == NULL || len == 0) {
        return;
    }

    while(spi->host->hal.hw->cmd.usr); // Wait for SPI bus ready
    spicommon_dmaworkaround_idle(DMA_CHANNEL);
    spi_ll_reset_dma(spi->host->hal.hw);

    lldesc_setup_link(spi->host->hal.dmadesc_tx, data, len, false);
    spi->host->hal.hw->dma_out_link.addr = SPI_OUTLINK_START | ((int)(&spi->host->hal.dmadesc_tx[0]) & 0xFFFFF);
    spi->host->hal.hw->dma_out_link.start = 1;
    spi->host->hal.hw->mosi_dlen.usr_mosi_dbitlen = (len * 8) - 1;
    spi->host->hal.hw->miso_dlen.usr_miso_dbitlen = 0;
    spi->host->hal.hw->cmd.usr = 1;
}

void IRAM_ATTR spiWriteU8NoWait(spi_device_t* spi, uint8_t data) {
    if (spi == NULL) {
        return;
    }
    spi->host->hal.hw->mosi_dlen.usr_mosi_dbitlen = 7;
    spi->host->hal.hw->miso_dlen.usr_miso_dbitlen = 0;
    spi->host->hal.hw->data_buf[0] = data;
    spi->host->hal.hw->cmd.usr = 1;
}

void IRAM_ATTR spiWriteU8(spi_device_t* spi, uint8_t data) {
    if (spi == NULL) {
        return;
    }
    spi->host->hal.hw->mosi_dlen.usr_mosi_dbitlen = 7;
    spi->host->hal.hw->miso_dlen.usr_miso_dbitlen = 0;
    spi->host->hal.hw->data_buf[0] = data;
    spi->host->hal.hw->cmd.usr = 1;
    while(spi->host->hal.hw->cmd.usr);
}

void IRAM_ATTR spiWriteU16(spi_device_t* spi, uint16_t data) {
    if (spi == NULL) {
        return;
    }
    if (!spi->host->hal.hw->ctrl.wr_bit_order) {
        MSB_16_SET(data, data);
    }
    spi->host->hal.hw->mosi_dlen.usr_mosi_dbitlen = 15;
    spi->host->hal.hw->miso_dlen.usr_miso_dbitlen = 0;
    spi->host->hal.hw->data_buf[0] = data;
    spi->host->hal.hw->cmd.usr = 1;
    while(spi->host->hal.hw->cmd.usr);
}

void IRAM_ATTR spiWriteU32(spi_device_t* spi, uint32_t data) {
    if (spi == NULL) {
        return;
    }
    if (!spi->host->hal.hw->ctrl.wr_bit_order) {
        MSB_32_SET(data, data);
    }
    spi->host->hal.hw->mosi_dlen.usr_mosi_dbitlen = 31;
    spi->host->hal.hw->miso_dlen.usr_miso_dbitlen = 0;
    spi->host->hal.hw->data_buf[0] = data;
    spi->host->hal.hw->cmd.usr = 1;
    while(spi->host->hal.hw->cmd.usr);
}

uint8_t spiTransferU8(spi_device_t* spi, uint8_t data) {
    if (spi == NULL) {
        return 0x00;
    }
    spi->host->hal.hw->mosi_dlen.usr_mosi_dbitlen = 7;
    spi->host->hal.hw->miso_dlen.usr_miso_dbitlen = 7;
    spi->host->hal.hw->data_buf[0] = data;
    spi->host->hal.hw->cmd.usr = 1;
    while(spi->host->hal.hw->cmd.usr);
    return spi->host->hal.hw->data_buf[0] & 0xff;
}

uint16_t spiTransferU16(spi_device_t* spi, uint16_t data) {
    if (spi == NULL) {
        return 0x00;
    }
    if (!spi->host->hal.hw->ctrl.wr_bit_order) {
        MSB_16_SET(data, data);
    }
    spi->host->hal.hw->mosi_dlen.usr_mosi_dbitlen = 15;
    spi->host->hal.hw->miso_dlen.usr_miso_dbitlen = 15;
    spi->host->hal.hw->data_buf[0] = data;
    spi->host->hal.hw->cmd.usr = 1;
    while(spi->host->hal.hw->cmd.usr);
    data = spi->host->hal.hw->data_buf[0] & 0xffff;
    if (!spi->host->hal.hw->ctrl.wr_bit_order) {
        MSB_16_SET(data, data);
    }
    return data;
}

uint32_t spiTransferU32(spi_device_t* spi, uint32_t data) {
    if (spi == NULL) {
        return 0x00;
    }
    if (!spi->host->hal.hw->ctrl.wr_bit_order) {
        MSB_32_SET(data, data);
    }
    spi->host->hal.hw->mosi_dlen.usr_mosi_dbitlen = 31;
    spi->host->hal.hw->miso_dlen.usr_miso_dbitlen = 31;
    spi->host->hal.hw->data_buf[0] = data;
    spi->host->hal.hw->cmd.usr = 1;
    while(spi->host->hal.hw->cmd.usr);
    data = spi->host->hal.hw->data_buf[0];
    if (!spi->host->hal.hw->ctrl.wr_bit_order) {
        MSB_32_SET(data, data);
    }
    return data;
}

void spiTransferBytes(spi_device_t* spi, uint8_t * data, uint8_t * out, uint32_t size) {
    if (spi == NULL) {
        return;
    }

}

void spiTransferBits(spi_device_t* spi, uint32_t data, uint32_t * out, uint8_t bits) {
    if (spi == NULL) {
        return;
    }
}
