#include "SPI_Master.h"
#include <esp_log.h>

bool SPI_Master::init() {
    // Stripped away all dead-weight sdkconfig overrides and register hijacks!
    // The touch driver's internal vTaskDelay(1) completely handles the bus balancing natively.
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = Config::PIN_SPI_SCLK;
    buscfg.mosi_io_num = Config::PIN_SPI_MOSI;
    buscfg.miso_io_num = Config::PIN_SPI_MISO; // Safely passes -1 from Config.h
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    
    // Kept small to completely unburden internal cache lines and SRAM boundaries
    buscfg.max_transfer_sz = 32; 

    // Initialise standard hardware bus engine using automated scheduling channels
    esp_err_t ret = spi_bus_initialize(Config::SPI_HOST_ID, &buscfg, SPI_DMA_CH_AUTO);
    return (ret == ESP_OK);
}

spi_device_handle_t SPI_Master::addDevice(int cs_pin, int clock_speed_hz) {
    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = clock_speed_hz;
    devcfg.mode = 0; // SPI mode 0 matching the XPT2046 resistive screen requirements
    devcfg.spics_io_num = cs_pin;
    devcfg.queue_size = 7;
    
    // FIXED: Removed the SPI_DEVICE_NO_DUMMY flag entirely!
    // This restores the required hardware turnaround delay clock cycle,
    // allowing the XPT2046 chip to cleanly transmit its real 12-bit analog data
    // instead of dumping a flat zero that freezes the display metrics layout.
    devcfg.flags = 0; 
    
    spi_device_handle_t handle;
    if (spi_bus_add_device(Config::SPI_HOST_ID, &devcfg, &handle) != ESP_OK) {
        return nullptr;
    }
    return handle;
}
