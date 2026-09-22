#include "framebuffer.h"
#include "blitters.h"          // Added: Connects your isolated blitting primitive engine
#include <string.h>
#include <esp_heap_caps.h>
#include <esp_lcd_panel_ops.h> // Fixed: Required for esp_lcd_panel_draw_bitmap in Core 2.0.17
#include <freertos/FreeRTOS.h> // Added: Necessary for scheduling functions
#include <freertos/task.h>     // Added: Necessary for task delay management primitives

Framebuffer::Framebuffer() 
    : panel_handle(nullptr), buffer_a(nullptr), buffer_b(nullptr), draw_buffer(nullptr), using_buffer_a(true), last_swap_micros(0) {}

Framebuffer::~Framebuffer() {
    if (buffer_a) heap_caps_free(buffer_a);
    if (buffer_b) heap_caps_free(buffer_b);
}

bool Framebuffer::init(esp_lcd_panel_handle_t panelHandle) {
    if (!panelHandle) return false;
    panel_handle = panelHandle;

    // Allocate both frames strictly inside external PSRAM using centralized size configurations
    buffer_a = (uint16_t*)heap_caps_aligned_alloc(64, Config::Framebuffer::BUFFER_SIZE_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    buffer_b = (uint16_t*)heap_caps_aligned_alloc(64, Config::Framebuffer::BUFFER_SIZE_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (!buffer_a || !buffer_b) {
        if (buffer_a) heap_caps_free(buffer_a);
        if (buffer_b) heap_caps_free(buffer_b);
        return false;
    }

    // Zero out memory spaces to establish clean initial canvases
    memset(buffer_a, 0, Config::Framebuffer::BUFFER_SIZE_BYTES);
    memset(buffer_b, 0, Config::Framebuffer::BUFFER_SIZE_BYTES);

    // Establish drawing tracking states
    draw_buffer = buffer_b; 
    using_buffer_a = true;
    last_swap_micros = 0; // Reset cadence timeline marker on init

    // Direct registration of your starting canvas target frame to display glass
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, buffer_a);

    return true;
}

void Framebuffer::swap() {
    if (!panel_handle) return;

    // --- HARDWARE SYNCHRONIZED HARMONIC CADENCE LOCK WITH OVERHEAD OFFSET ---
    // Shortening the target window from 16666 to 16546 compensates for the internal
    // register setup overhead of esp_lcd. This snaps the frame swap right inside the 
    // blanking zone, eliminating that last remaining tiny bit of fracturing!
    if (last_swap_micros != 0) {
        uint32_t elapsed = micros() - last_swap_micros;
        if (elapsed < 16546) {
            uint32_t remaining = 16546 - elapsed;
            
            // If we have plenty of time remaining, yield nicely to other core tasks first
            if (remaining > 2000) {
                vTaskDelay(pdMS_TO_TICKS(remaining / 1000 - 1));
            }
            
            // Recalculate and apply clean, microsecond-accurate micro-delay tracking
            elapsed = micros() - last_swap_micros;
            if (elapsed < 16546) {
                delayMicroseconds(16546 - elapsed);
            }
        }
    }

    // Capture the absolute start timestamp of the fresh hardware transmission cycle
    last_swap_micros = micros();

    // Shift hardware pipeline focus seamlessly over to your freshly completed canvas block
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, draw_buffer);

    // Trade buffer identities cleanly to prevent racing or active screen tearing artifacts
    if (using_buffer_a) {
        draw_buffer = buffer_a; 
        using_buffer_a = false;
    } else {
        draw_buffer = buffer_b; 
        using_buffer_a = true;
    }
}

void Framebuffer::clear(uint16_t color) {
    if (!draw_buffer) return;

    // If optimized 32-bit blocks are requested via config, merge pixels to cut bus cycles in half
    if (Config::Framebuffer::USE_32BIT_CLEAR) {
        size_t pixel_count = Config::SCREEN_WIDTH * Config::SCREEN_HEIGHT;
        uint32_t double_pixel = (color << 16) | color;
        uint32_t* long_p = (uint32_t*)draw_buffer;

        for (size_t i = 0; i < pixel_count / 2; ++i) {
            long_p[i] = double_pixel;
        }
    } else {
        // Fallback basic 16-bit word byte filling loop if 32-bit logic is turned off
        size_t pixel_count = Config::SCREEN_WIDTH * Config::SCREEN_HEIGHT;
        for (size_t i = 0; i < pixel_count; ++i) {
            draw_buffer[i] = color;
        }
    }
}

// --- FIXED PARAMETER SIGNATURE FOR CLIPPING SAFETY ---
void Framebuffer::drawPixel(int16_t x, int16_t y, uint16_t color) {
    // Forwarded to Blitters namespace to take advantage of complete boundary checking protection
    Blitters::drawPixel(draw_buffer, x, y, color);
}

// --- OPTIMIZED GEOMETRIC PRIMITIVES FORWARDING BLOCKS ---
// Forwards standard canvas actions directly to your high-efficiency isolated Blitter utilities

void Framebuffer::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    Blitters::fillRect(draw_buffer, x, y, w, h, color);
}

void Framebuffer::drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    Blitters::drawHLine(draw_buffer, x, y, w, color);
}

void Framebuffer::drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    Blitters::drawVLine(draw_buffer, x, y, h, color);
}

void Framebuffer::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    Blitters::drawRect(draw_buffer, x, y, w, h, color);
}

void Framebuffer::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* bitmap) {
    Blitters::drawBitmap(draw_buffer, x, y, w, h, bitmap);
}
