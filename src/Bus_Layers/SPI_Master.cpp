#include "SPI_Master.h"

bool SPI_Master::init() {
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = Config::PIN_SHARED_SPI_SCLK;
    buscfg.mosi_io_num = Config::PIN_SHARED_SPI_MOSI;
    buscfg.miso_io_num = Config::PIN_SHARED_SPI_MISO;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = Config::SPI::MAX_TRANSFER_SIZE_BYTES;

    const esp_err_t result =
        spi_bus_initialize(Config::SPI_HOST_ID, &buscfg, SPI_DMA_CH_AUTO);

    return result == ESP_OK;
}

spi_device_handle_t SPI_Master::addDevice(int cs_pin, int clock_speed_hz) {
    if (cs_pin < 0 || clock_speed_hz <= 0) {
        return nullptr;
    }

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = clock_speed_hz;
    devcfg.mode = 0;
    devcfg.spics_io_num = cs_pin;
    devcfg.queue_size = Config::SPI::DEVICE_QUEUE_SIZE;

    // Keep normal turnaround timing. The current XPT2046 byte handling relies
    // on the Core 2.0.17 behaviour already validated by this project.
    devcfg.flags = 0;

    spi_device_handle_t handle = nullptr;
    if (spi_bus_add_device(Config::SPI_HOST_ID, &devcfg, &handle) != ESP_OK) {
        return nullptr;
    }

    return handle;
}
