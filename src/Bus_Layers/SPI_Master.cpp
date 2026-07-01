#include "SPI_Master.h"
#include <esp_log.h>

bool SPI_Master::init() {
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = Config::PIN_SPI_SCLK;
    buscfg.mosi_io_num = Config::PIN_SPI_MOSI;
    buscfg.miso_io_num = Config::PIN_SPI_MISO;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    // Increased max_transfer_sz to 4096 to provide headroom for 
    // larger data packets during touch controller operations.
    buscfg.max_transfer_sz = 4096;

    esp_err_t ret = spi_bus_initialize(Config::SPI_HOST_ID, &buscfg, SPI_DMA_CH_AUTO);
    return (ret == ESP_OK);
}

spi_device_handle_t SPI_Master::addDevice(int cs_pin, int clock_speed_hz) {
    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = clock_speed_hz;
    devcfg.mode = 0; // SPI mode 0
    devcfg.spics_io_num = cs_pin;
    devcfg.queue_size = 7;
    
    spi_device_handle_t handle;
    if (spi_bus_add_device(Config::SPI_HOST_ID, &devcfg, &handle) != ESP_OK) {
        return nullptr;
    }
    return handle;
}
