#include "framebuffer.h"
#include "blitters.h"

#include <esp_lcd_panel_ops.h>
#include <freertos/FreeRTOS.h>
#include <string.h>
#include <freertos/task.h>

Framebuffer::Framebuffer()
    : panel_handle(nullptr),
      cached_draw_buffer(nullptr),
      last_swap_micros(0),
      last_frame_time_us(0),
      frame_count(0) {}

bool Framebuffer::init(esp_lcd_panel_handle_t panelHandle) {
    if (!panelHandle) return false;

    panel_handle = panelHandle;
    last_swap_micros = 0;
    last_frame_time_us = 0;
    frame_count = 0;
    cached_draw_buffer = nullptr;

    if (!memory.init()) {
        panel_handle = nullptr;
        return false;
    }

    if (!refreshDrawBuffer()) {
        memory.release();
        panel_handle = nullptr;
        return false;
    }

    // Start with a known-black front buffer on the panel.
    esp_err_t result = esp_lcd_panel_draw_bitmap(
        panel_handle,
        0,
        0,
        Config::SCREEN_WIDTH,
        Config::SCREEN_HEIGHT,
        memory.getFrontBuffer());

    if (result != ESP_OK) {
        memory.release();
        cached_draw_buffer = nullptr;
        panel_handle = nullptr;
        return false;
    }

    return true;
}

bool Framebuffer::refreshDrawBuffer() {
    cached_draw_buffer = memory.getDrawBuffer();
    return cached_draw_buffer != nullptr;
}

bool Framebuffer::swap() {
    if (!isReady() || !cached_draw_buffer) return false;

    const uint32_t target_interval = Config::Framebuffer::FRAME_CADENCE_US;

    if (last_swap_micros != 0 && target_interval > 0) {
        uint32_t elapsed = micros() - last_swap_micros;

        if (elapsed < target_interval) {
            uint32_t remaining = target_interval - elapsed;

            if (remaining > 2000) {
                uint32_t delay_ms = remaining / 1000;
                if (delay_ms > 1) {
                    vTaskDelay(pdMS_TO_TICKS(delay_ms - 1));
                }
            }

            elapsed = micros() - last_swap_micros;
            if (elapsed < target_interval) {
                delayMicroseconds(target_interval - elapsed);
            }
        }
    }

    const uint32_t present_start = micros();
    if (last_swap_micros != 0) {
        last_frame_time_us = present_start - last_swap_micros;
    }
    last_swap_micros = present_start;

    esp_err_t result = esp_lcd_panel_draw_bitmap(
        panel_handle,
        0,
        0,
        Config::SCREEN_WIDTH,
        Config::SCREEN_HEIGHT,
        cached_draw_buffer);

    if (result != ESP_OK) {
        return false;
    }

    memory.swapBuffers();

    // The draw-buffer role changes only at presentation. Resolve the new
    // pointer once here; all drawing calls for the next frame use this cache.
    if (!refreshDrawBuffer()) {
        return false;
    }

    ++frame_count;
    return true;
}

void Framebuffer::clear(uint16_t color) {
    uint16_t* draw_buffer = cached_draw_buffer;
    if (!draw_buffer) return;

    const size_t pixel_count =
        static_cast<size_t>(Config::SCREEN_WIDTH) * Config::SCREEN_HEIGHT;

    // Common full-screen clears can use the optimized byte-fill path.
    // 0x0000 and 0xFFFF are byte-uniform, so memset preserves RGB565 values.
    if (color == 0x0000U) {
        memset(draw_buffer, 0x00, pixel_count * sizeof(uint16_t));
        return;
    }

    if (color == 0xFFFFU) {
        memset(draw_buffer, 0xFF, pixel_count * sizeof(uint16_t));
        return;
    }

    if (Config::Framebuffer::USE_32BIT_CLEAR) {
        const uint32_t double_pixel =
            (static_cast<uint32_t>(color) << 16) | static_cast<uint32_t>(color);
        uint32_t* long_p = reinterpret_cast<uint32_t*>(draw_buffer);

        const size_t pair_count = pixel_count / 2;
        for (size_t i = 0; i < pair_count; ++i) {
            long_p[i] = double_pixel;
        }

        if (pixel_count & 1U) {
            draw_buffer[pixel_count - 1] = color;
        }
    } else {
        for (size_t i = 0; i < pixel_count; ++i) {
            draw_buffer[i] = color;
        }
    }
}

void Framebuffer::drawPixel(int16_t x, int16_t y, uint16_t color) {
    Blitters::drawPixel(cached_draw_buffer, x, y, color);
}

void Framebuffer::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    Blitters::fillRect(cached_draw_buffer, x, y, w, h, color);
}

void Framebuffer::drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    Blitters::drawHLine(cached_draw_buffer, x, y, w, color);
}

void Framebuffer::drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    Blitters::drawVLine(cached_draw_buffer, x, y, h, color);
}

void Framebuffer::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    Blitters::drawRect(cached_draw_buffer, x, y, w, h, color);
}

void Framebuffer::drawBitmap(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    const uint16_t* bitmap) {
    Blitters::drawBitmap(cached_draw_buffer, x, y, w, h, bitmap);
}

bool Framebuffer::isReady() const {
    return panel_handle != nullptr && memory.isReady();
}

float Framebuffer::getApproxFPS() const {
    if (last_frame_time_us == 0) return 0.0f;
    return 1000000.0f / static_cast<float>(last_frame_time_us);
}
