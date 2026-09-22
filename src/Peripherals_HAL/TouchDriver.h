#pragma once

#include "../Config.h"

#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class TouchDriver {
private:
    SPIClass* spi_bus = nullptr;

    bool transfer16(uint8_t cmd, uint16_t &value);
    bool readRawPair(uint16_t &raw_x, uint16_t &raw_y);

    bool was_pressed_last_frame = false;
    bool is_pressed_current_frame = false;

public:
    bool init(SPIClass& bus);

    bool getTouch(uint16_t &x, uint16_t &y);
    bool getRawTouch(uint16_t &raw_x, uint16_t &raw_y);

    bool isPressed();
    bool isNewPress();
    bool isReleased();
};
