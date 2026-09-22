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
    // Memory arrays are now handled cleanly by your central NV3047_Memory manager class module!
}

// FIXED IMPLEMENTATION SIGNATURE: Accepts the pre-allocated bare-metal dual PSRAM buffers 
// directly from the memory manager setup loop to completely kill duplicate allocation collisions!
bool Framebuffer::init(esp_lcd_panel_handle_t panelHandle, uint16_t* buf1, uint16_t* buf2) {
    if (!panelHandle) return false;
    panel_handle = panelHandle;

    // LATCH TO THE POINTERS PASSED FROM NV3047_MEMORY 
    // Strips out the local heap_caps_aligned_alloc blocks completely so the display 
    // driver and memory engine aren't duplicate allocating inside the same heap slots!
    buffer_a = buf1;
    buffer_b = buf2;

    if (!buffer_a || !buffer_b) {
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
