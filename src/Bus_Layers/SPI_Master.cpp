#include "SPI_Master.h"

#include <Arduino.h>

namespace {

spi_device_handle_t legacy_touch_device = nullptr;
bool legacy_bus_initialized = false;

bool initLegacyTouchBus() {
    if (legacy_touch_device) {
        return true;
    }

    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = Config::PIN_SPI_SCLK;
    buscfg.mosi_io_num = Config::PIN_SPI_MOSI;
    buscfg.miso_io_num = Config::PIN_SPI_MISO;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz =
        Config::SPI::LEGACY_MAX_TRANSFER_BYTES;

    esp_err_t result = spi_bus_initialize(
        Config::SPI::LEGACY_TOUCH_HOST,
        &buscfg,
        SPI_DMA_CH_AUTO);

    if (result != ESP_OK &&
        result != ESP_ERR_INVALID_STATE) {
        return false;
    }

    legacy_bus_initialized = true;

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = Config::SPI::TOUCH_CLOCK_HZ;
    devcfg.mode = 0;
    devcfg.spics_io_num = Config::PIN_TOUCH_CS;
    devcfg.queue_size = Config::SPI::LEGACY_QUEUE_SIZE;

    // Old main's final working path used the normal turnaround timing.
    devcfg.flags = 0;

    result = spi_bus_add_device(
        Config::SPI::LEGACY_TOUCH_HOST,
        &devcfg,
        &legacy_touch_device);

    if (result != ESP_OK) {
        legacy_touch_device = nullptr;
        return false;
    }

    return true;
}

bool initV21SharedBus() {
    pinMode(Config::PIN_TOUCH_CS, OUTPUT);
    digitalWrite(Config::PIN_TOUCH_CS, HIGH);

    if (Config::SDCard::ENABLED &&
        Config::PIN_SD_CS >= 0) {
        pinMode(Config::PIN_SD_CS, OUTPUT);
        digitalWrite(Config::PIN_SD_CS, HIGH);
    }

    SPI.begin(
        Config::PIN_SPI_SCLK,
        Config::PIN_SPI_MISO,
        Config::PIN_SPI_MOSI,
        -1);

    return SPI.bus() != nullptr;
}

} // namespace

bool SPI_Master::init() {
    if (Config::RGB_V21_MATRIX_ACTIVE) {
        return initV21SharedBus();
    }

    return initLegacyTouchBus();
}

SPIClass& SPI_Master::bus() {
    return SPI;
}

spi_device_handle_t SPI_Master::legacyTouchDevice() {
    return legacy_touch_device;
}
