#pragma once
#include "../Config.h"
#include <esp_lcd_panel_ops.h>

class DisplayDriver {
private:
    esp_lcd_panel_handle_t handle;
    uint8_t current_brightness;    // Stores active backlight state to allow restore after wake cycles
    bool is_sleeping;              // Tracks physical backlight power state

public:
    bool init(esp_lcd_panel_handle_t rgb_handle);
    void setBacklight(uint8_t brightness);
    void drawBitmap(int x, int y, int w, int h, const uint16_t *bitmap);
    void fillScreen(uint16_t color);

    // --- LIFE CYCLE & POWER MANAGEMENT INTERFACES ---
    void setBrightness(uint8_t percentage); // Maps clean 0-100% inputs to the low-level registers
    void sleep();                            // Drops backlight lines to zero immediately
    void wake();                             // Returns backlight directly to previous percentage state
};
