#include "blitters.h"
#include <string.h>

namespace Blitters {

void drawPixel(uint16_t* buffer, int16_t x, int16_t y, uint16_t color) {
    // Hard boundary clipping check to completely protect system memory loops
    if (!buffer || x < 0 || x >= Config::SCREEN_WIDTH || y < 0 || y >= Config::SCREEN_HEIGHT) return;
    buffer[y * Config::SCREEN_WIDTH + x] = color;
}

void fillRect(uint16_t* buffer, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!buffer || w <= 0 || h <= 0) return;

    // --- HARDWARE BOUNDS CLIPPING GUARD ---
    if (x >= Config::SCREEN_WIDTH || y >= Config::SCREEN_HEIGHT) return;
    
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    
    if (x + w > Config::SCREEN_WIDTH)  w = Config::SCREEN_WIDTH - x;
    if (y + h > Config::SCREEN_HEIGHT) h = Config::SCREEN_HEIGHT - y;
    
    if (w <= 0 || h <= 0) return;

    // Compute pointer address offsets matching our starting coordinates
    uint16_t* row_ptr = buffer + (y * Config::SCREEN_WIDTH) + x;

    // PERFORMANCE OPTIMIZATION: Pack two 16-bit pixels into one 32-bit word
    uint32_t packed_color = (color << 16) | color;
    size_t dwords_per_row = w / 2;
    bool has_odd_pixel = (w & 1) != 0;

    for (int16_t row = 0; row < h; ++row) {
        uint32_t* dword_ptr = (uint32_t*)row_ptr;
        
        // Blast out 32-bit pairs down the bus line
        for (size_t i = 0; i < dwords_per_row; ++i) {
            dword_ptr[i] = packed_color;
        }
        
        // Handle the trailing single 16-bit pixel if the width was an odd number
        if (has_odd_pixel) {
            row_ptr[w - 1] = color;
        }

        // Advance to the exact start location index of the subsequent row
        row_ptr += Config::SCREEN_WIDTH;
    }
}

void drawHLine(uint16_t* buffer, int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (!buffer || w <= 0 || y < 0 || y >= Config::SCREEN_HEIGHT) return;

    if (x < 0) { w += x; x = 0; }
    if (x + w > Config::SCREEN_WIDTH) w = Config::SCREEN_WIDTH - x;
    if (w <= 0) return;

    uint16_t* ptr = buffer + (y * Config::SCREEN_WIDTH) + x;
    
    uint32_t packed_color = (color << 16) | color;
    size_t dwords = w / 2;
    uint32_t* dword_ptr = (uint32_t*)ptr;

    for (size_t i = 0; i < dwords; ++i) {
        dword_ptr[i] = packed_color;
    }
    if (w & 1) {
        ptr[w - 1] = color;
    }
}

void drawVLine(uint16_t* buffer, int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (!buffer || h <= 0 || x < 0 || x >= Config::SCREEN_WIDTH) return;

    if (y < 0) { h += y; y = 0; }
    if (y + h > Config::SCREEN_HEIGHT) h = Config::SCREEN_HEIGHT - y;
    if (h <= 0) return;

    uint16_t* ptr = buffer + (y * Config::SCREEN_WIDTH) + x;

    for (int16_t i = 0; i < h; ++i) {
        *ptr = color;
        ptr += Config::SCREEN_WIDTH; // Step down exactly one row in RAM memory layout
    }
}

void drawRect(uint16_t* buffer, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!buffer || w <= 0 || h <= 0) return;

    // Direct high-efficiency edge distribution leveraging your line blitters
    drawHLine(buffer, x, y, w, color);             // Top border horizontal strip
    drawHLine(buffer, x, y + h - 1, w, color);     // Bottom border horizontal strip
    drawVLine(buffer, x, y, h, color);             // Left boundary vertical strip
    drawVLine(buffer, x + w - 1, y, h, color);     // Right boundary vertical strip
}

void drawBitmap(uint16_t* buffer, int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* bitmap) {
    if (!buffer || !bitmap || w <= 0 || h <= 0) return;

    // Hardware Bounds Edge Clipping Protection
    if (x >= Config::SCREEN_WIDTH || y >= Config::SCREEN_HEIGHT) return;

    int16_t src_offset_x = 0;
    int16_t src_offset_y = 0;
    int16_t original_w = w; // Store original asset line width for stride math

    if (x < 0) { src_offset_x = -x; w += x; x = 0; }
    if (y < 0) { src_offset_y = -y; h += y; y = 0; }

    if (x + w > Config::SCREEN_WIDTH)  w = Config::SCREEN_WIDTH - x;
    if (y + h > Config::SCREEN_HEIGHT) h = Config::SCREEN_HEIGHT - y;

    if (w <= 0 || h <= 0) return;

    // Compute pointer layouts for destination RAM and source asset flash arrays
    uint16_t* dest_row = buffer + (y * Config::SCREEN_WIDTH) + x;
    const uint16_t* src_row = bitmap + (src_offset_y * original_w) + src_offset_x;

    size_t bytes_to_copy = w * sizeof(uint16_t);

    // High-speed block memory streaming loops
    for (int16_t row = 0; row < h; ++row) {
        memcpy(dest_row, src_row, bytes_to_copy);
        dest_row += Config::SCREEN_WIDTH; // Advance screen destination pointer row
        src_row += original_w;            // Advance source asset pointer row
    }
}

} // namespace Blitters
