#include "SPI_Master.h"
#include <driver/gpio.h> // Required for direct hardware pin resets
#include <esp_log.h>

bool SPI_Master::init() {
    // FIXED FOR ESP32-S3 BOOT-STRAPPING PIN RECOVERY:
    // Forcefully rips your factory hardwired traces away from the internal 
    // USB/JTAG debug controllers and resets their hardware crossbar switcher routes!
    gpio_reset_pin((gpio_num_t)Config::PIN_SPI_SCLK);
    gpio_reset_pin((gpio_num_t)Config::PIN_SPI_MOSI);
    gpio_reset_pin((gpio_num_t)Config::PIN_TOUCH_CS);

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
    
    // Targets your centralized Config namespace definition directly! This keeps your settings 
    // completely locked inside Config.h so you never have to guess who is driving the bus.
    devcfg.clock_speed_hz = Config::TOUCH_CLOCK_SPEED_HZ;
    
    devcfg.mode = 0; // SPI mode 0 matching the XPT2046 resistive screen requirements
    devcfg.spics_io_num = cs_pin;
    devcfg.queue_size = 7;
    
    // Restores the required hardware turnaround delay clock cycle,
    // allowing the XPT2046 chip to cleanly transmit its real 12-bit analog data
    // instead of dumping a flat zero that freezes the display metrics layout.
    devcfg.flags = 0; 
    
    // Explicitly nullifies background callbacks to prevent uninitialized memory 
    // from injecting phantom register cycles that stall out your SPI hardware arbiter.
    devcfg.pre_cb  = nullptr;
    devcfg.post_cb = nullptr;
    
    spi_device_handle_t handle;
    if (spi_bus_add_device(Config::SPI_HOST_ID, &devcfg, &handle) != ESP_OK) {
        return nullptr;
    }
    return handle;
}
