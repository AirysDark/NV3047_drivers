#pragma once

#include "../Config.h"
#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class TouchDriver {
private:
    spi_device_handle_t spi_handle = nullptr;

    uint16_t transfer16(uint8_t cmd);
    bool readRawPair(uint16_t &raw_x, uint16_t &raw_y);

    static constexpr uint16_t RAW_X_MIN = 300;
    static constexpr uint16_t RAW_X_MAX = 3850;
    static constexpr uint16_t RAW_Y_MIN = 250;
    static constexpr uint16_t RAW_Y_MAX = 3750;

    bool was_pressed_last_frame = false;
    bool is_pressed_current_frame = false;

public:
    bool init(spi_device_handle_t handle);

    bool getTouch(uint16_t &x, uint16_t &y);
    bool getRawTouch(uint16_t &raw_x, uint16_t &raw_y);

    bool isPressed();
    bool isNewPress();
    bool isReleased();
};
