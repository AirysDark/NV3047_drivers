#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../Config.h"

namespace Blitters {
    // Core Single Pixel Blit with full boundary clipping protection
    void drawPixel(uint16_t* buffer, int16_t x, int16_t y, uint16_t color);

    // Ultra-fast filled rectangle blitter using 32-bit row optimization
    void fillRect(uint16_t* buffer, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    // Optimized single-line blitters to minimize loop boundary checks
    void drawHLine(uint16_t* buffer, int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawVLine(uint16_t* buffer, int16_t x, int16_t y, int16_t h, uint16_t color);

    // High-performance outline rectangle blitter for empty boxes and borders
    void drawRect(uint16_t* buffer, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    // High-performance bitmap blitter to cleanly dump image/icon arrays to memory
    void drawBitmap(uint16_t* buffer, int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* bitmap);
}
