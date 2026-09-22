#pragma once

#include <stdint.h>
#include <esp_lcd_panel_rgb.h>
#include <Arduino.h> // Added: Required to expose high-resolution micros() timing hooks
#include "../Config.h"

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    // Core Lifecycle
    // FIXED SIGNATURE: Accepts the pre-allocated bare-metal dual PSRAM video canvas frame buffers
    // with safe default parameters to prevent type conflicts during library handshakes!
    bool init(esp_lcd_panel_handle_t panelHandle, uint16_t* buf1 = nullptr, uint16_t* buf2 = nullptr);
    void swap();

    // Drawing Helpers
    void clear(uint16_t color = Config::COLOR_BLACK);
    
    // Core Single Pixel Blit with clipping safety bounds checking
    void drawPixel(int16_t x, int16_t y, uint16_t color);

    // Optimized Geometric Primitives
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    // High-performance image array blitter
    void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* bitmap);

    // Direct Pointer access (for fast rendering libraries like LVGL or custom blitters)
    uint16_t* getDrawBuffer() const { return draw_buffer; }

private:
    esp_lcd_panel_handle_t panel_handle;
    
    // Memory Pointers
    uint16_t* buffer_a;
    uint16_t* buffer_b;
    uint16_t* draw_buffer;

    bool using_buffer_a;

    // --- AUTOMATIC CADENCE ENGINE STORAGE ---
    // Tracks previous frame exit timestamps to self-regulate loop cycles internally
    uint32_t last_swap_micros; 
};
