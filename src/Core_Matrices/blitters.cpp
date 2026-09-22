#include "blitters.h"

#include <stdint.h>
#include <string.h>

namespace Blitters {
namespace {

inline void fillSpan(uint16_t* ptr, int16_t count, uint16_t color) {
    if (!ptr || count <= 0) return;

    // A 16-bit pixel beginning on an odd x coordinate is only 2-byte aligned.
    // Write that first pixel separately before switching to 32-bit pair writes.
    if ((reinterpret_cast<uintptr_t>(ptr) & 0x3U) != 0U) {
        *ptr++ = color;
        --count;
    }

    const uint32_t packed_color =
        (static_cast<uint32_t>(color) << 16) | static_cast<uint32_t>(color);

    const int16_t pair_count = count / 2;
    uint32_t* pair_ptr = reinterpret_cast<uint32_t*>(ptr);

    for (int16_t i = 0; i < pair_count; ++i) {
        pair_ptr[i] = packed_color;
    }

    ptr = reinterpret_cast<uint16_t*>(pair_ptr + pair_count);
    if (count & 1) {
        *ptr = color;
    }
}

} // namespace

void drawPixel(uint16_t* buffer, int16_t x, int16_t y, uint16_t color) {
    if (!buffer ||
        x < 0 || x >= Config::SCREEN_WIDTH ||
        y < 0 || y >= Config::SCREEN_HEIGHT) {
        return;
    }

    buffer[y * Config::SCREEN_WIDTH + x] = color;
}

void fillRect(uint16_t* buffer, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!buffer || w <= 0 || h <= 0) return;
    if (x >= Config::SCREEN_WIDTH || y >= Config::SCREEN_HEIGHT) return;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }

    if (x + w > Config::SCREEN_WIDTH) {
        w = Config::SCREEN_WIDTH - x;
    }
    if (y + h > Config::SCREEN_HEIGHT) {
        h = Config::SCREEN_HEIGHT - y;
    }

    if (w <= 0 || h <= 0) return;

    uint16_t* row_ptr = buffer + (y * Config::SCREEN_WIDTH) + x;
    for (int16_t row = 0; row < h; ++row) {
        fillSpan(row_ptr, w, color);
        row_ptr += Config::SCREEN_WIDTH;
    }
}

void drawHLine(uint16_t* buffer, int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (!buffer || w <= 0 || y < 0 || y >= Config::SCREEN_HEIGHT) return;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (x + w > Config::SCREEN_WIDTH) {
        w = Config::SCREEN_WIDTH - x;
    }
    if (w <= 0) return;

    uint16_t* ptr = buffer + (y * Config::SCREEN_WIDTH) + x;
    fillSpan(ptr, w, color);
}

void drawVLine(uint16_t* buffer, int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (!buffer || h <= 0 || x < 0 || x >= Config::SCREEN_WIDTH) return;

    if (y < 0) {
        h += y;
        y = 0;
    }
    if (y + h > Config::SCREEN_HEIGHT) {
        h = Config::SCREEN_HEIGHT - y;
    }
    if (h <= 0) return;

    uint16_t* ptr = buffer + (y * Config::SCREEN_WIDTH) + x;
    for (int16_t i = 0; i < h; ++i) {
        *ptr = color;
        ptr += Config::SCREEN_WIDTH;
    }
}

void drawRect(uint16_t* buffer, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!buffer || w <= 0 || h <= 0) return;

    drawHLine(buffer, x, y, w, color);
    drawHLine(buffer, x, y + h - 1, w, color);
    drawVLine(buffer, x, y, h, color);
    drawVLine(buffer, x + w - 1, y, h, color);
}

void drawBitmap(
    uint16_t* buffer,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    const uint16_t* bitmap) {
    if (!buffer || !bitmap || w <= 0 || h <= 0) return;
    if (x >= Config::SCREEN_WIDTH || y >= Config::SCREEN_HEIGHT) return;

    int16_t src_offset_x = 0;
    int16_t src_offset_y = 0;
    const int16_t original_w = w;

    if (x < 0) {
        src_offset_x = -x;
        w += x;
        x = 0;
    }
    if (y < 0) {
        src_offset_y = -y;
        h += y;
        y = 0;
    }

    if (x + w > Config::SCREEN_WIDTH) {
        w = Config::SCREEN_WIDTH - x;
    }
    if (y + h > Config::SCREEN_HEIGHT) {
        h = Config::SCREEN_HEIGHT - y;
    }

    if (w <= 0 || h <= 0) return;

    uint16_t* dest_row = buffer + (y * Config::SCREEN_WIDTH) + x;
    const uint16_t* src_row =
        bitmap + (src_offset_y * original_w) + src_offset_x;
    const size_t bytes_to_copy = static_cast<size_t>(w) * sizeof(uint16_t);

    for (int16_t row = 0; row < h; ++row) {
        memcpy(dest_row, src_row, bytes_to_copy);
        dest_row += Config::SCREEN_WIDTH;
        src_row += original_w;
    }
}

} // namespace Blitters
