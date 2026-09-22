#pragma once

#include "../Config.h"

#include <SPI.h>
#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class TouchDriver {
private:
    SPIClass* spi_bus = nullptr;
    spi_device_handle_t legacy_spi_handle = nullptr;

    bool transfer16(uint8_t cmd, uint16_t &value);
    bool readRawPair(uint16_t &raw_x, uint16_t &raw_y);
    bool transportReady() const;

    bool was_pressed_last_frame = false;
    bool is_pressed_current_frame = false;

public:
    // V2.1 shared Arduino SPI transport.
    bool init(SPIClass& bus);

    // LEGACY_WORKING dedicated ESP-IDF SPI transport.
    bool init(spi_device_handle_t handle);

    bool getTouch(uint16_t &x, uint16_t &y);
    bool getRawTouch(uint16_t &raw_x, uint16_t &raw_y);

    bool isPressed();
    bool isNewPress();
    bool isReleased();
};
