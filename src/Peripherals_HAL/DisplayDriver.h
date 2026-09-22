#pragma once

#include "../Config.h"
#include <esp_lcd_panel_ops.h>

class DisplayDriver {
private:
    esp_lcd_panel_handle_t handle = nullptr;
    uint8_t current_brightness = 0;
    bool is_sleeping = true;
    uint16_t* fill_buffer = nullptr;

public:
    DisplayDriver() = default;
    ~DisplayDriver();

    bool init(esp_lcd_panel_handle_t rgb_handle);

    // Raw 8-bit PWM duty. Normal application code should prefer setBrightness(0-100).
    void setBacklight(uint8_t brightness);

    void drawBitmap(int x, int y, int w, int h, const uint16_t *bitmap);
    void fillScreen(uint16_t color);

    void setBrightness(uint8_t percentage);
    void sleep();
    void wake();

    DisplayDriver(const DisplayDriver&) = delete;
    DisplayDriver& operator=(const DisplayDriver&) = delete;
};
