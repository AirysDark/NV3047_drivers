#pragma once

#include <stdint.h>
#include <esp_lcd_panel_rgb.h>
#include <Arduino.h>

#include "../Config.h"
#include "MemoryManager.h"

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer() = default;

    bool init(esp_lcd_panel_handle_t panelHandle);
    bool swap();

    void clear(uint16_t color = Config::COLOR_BLACK);

    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* bitmap);

    // Returns the frame-local cached draw pointer. The cache is refreshed
    // only after a successful buffer-role swap, avoiding provider delegation
    // on every drawing primitive.
    uint16_t* getDrawBuffer() const { return cached_draw_buffer; }
    MemoryManager& getMemoryManager() { return memory; }
    const MemoryManager& getMemoryManager() const { return memory; }

    bool isReady() const;
    uint32_t getFrameCount() const { return frame_count; }
    uint32_t getLastFrameTimeUs() const { return last_frame_time_us; }
    float getApproxFPS() const;

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

private:
    bool refreshDrawBuffer();

    esp_lcd_panel_handle_t panel_handle;
    MemoryManager memory;
    uint16_t* cached_draw_buffer;

    uint32_t last_swap_micros;
    uint32_t last_frame_time_us;
    uint32_t frame_count;
};
