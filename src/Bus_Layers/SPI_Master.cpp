#include "SPI_Master.h"

bool SPI_Master::init() {
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = Config::PIN_SPI_SCLK;
    buscfg.mosi_io_num = Config::PIN_SPI_MOSI;
    buscfg.miso_io_num = Config::PIN_SPI_MISO;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 32;

    const esp_err_t result =
        spi_bus_initialize(Config::SPI_HOST_ID, &buscfg, SPI_DMA_CH_AUTO);

    return result == ESP_OK;
}

spi_device_handle_t SPI_Master::addDevice(int cs_pin, int clock_speed_hz) {
    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = clock_speed_hz;
    devcfg.mode = 0;
    devcfg.spics_io_num = cs_pin;

    // Touch uses polling transfers, so a single queued transaction is sufficient.
    devcfg.queue_size = 1;

    // Intentionally leave SPI_DEVICE_NO_DUMMY disabled. The working XPT2046 setup
    // under Arduino-ESP32 2.0.17 relies on the normal turnaround behaviour.
    devcfg.flags = 0;

    spi_device_handle_t handle = nullptr;
    if (spi_bus_add_device(Config::SPI_HOST_ID, &devcfg, &handle) != ESP_OK) {
        return nullptr;
    }

    return handle;
}
