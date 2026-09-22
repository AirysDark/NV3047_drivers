#pragma once
#include <stdint.h>
#include <esp_lcd_panel_rgb.h>
#include "../Config.h"

class RGB {
private:
    esp_lcd_panel_handle_t panel_handle = nullptr;

public:
    // Uses the fixed hardware layout declared in Config.h.
    bool init();
    
    esp_lcd_panel_handle_t getHandle();
    uint16_t getColor(const char* colorName);
};
