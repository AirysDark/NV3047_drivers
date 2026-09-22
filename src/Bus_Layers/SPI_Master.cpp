#include "SPI_Master.h"

#include <Arduino.h>

namespace {

spi_device_handle_t touch_device = nullptr;

} // namespace

bool SPI_Master::init() {
    if (touch_device) {
        return true;
    }

    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = Config::PIN_SPI_SCLK;
    buscfg.mosi_io_num = Config::PIN_SPI_MOSI;
    buscfg.miso_io_num = Config::PIN_SPI_MISO;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz =
        Config::SPI::MAX_TRANSFER_BYTES;

    esp_err_t result = spi_bus_initialize(
        Config::SPI::TOUCH_HOST,
        &buscfg,
        SPI_DMA_CH_AUTO);

    if (result != ESP_OK &&
        result != ESP_ERR_INVALID_STATE) {
        return false;
    }

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = Config::SPI::TOUCH_CLOCK_HZ;
    devcfg.mode = 0;
    devcfg.spics_io_num = Config::PIN_TOUCH_CS;
    devcfg.queue_size = Config::SPI::QUEUE_SIZE;
    devcfg.flags = 0;

    result = spi_bus_add_device(
        Config::SPI::TOUCH_HOST,
        &devcfg,
        &touch_device);

    if (result != ESP_OK) {
        touch_device = nullptr;
        return false;
    }

    return true;
}

spi_device_handle_t SPI_Master::touchDevice() {
    return touch_device;
}

SPIClass& SPI_Master::bus() {
    return SPI;
}
