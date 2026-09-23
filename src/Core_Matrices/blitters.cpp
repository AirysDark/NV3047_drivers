#include "blitters.h"

#include <stdint.h>
#include <string.h>

namespace Blitters {
namespace {

inline bool rectFullyInside(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h) {

    return
        w > 0 &&
        h > 0 &&
        x >= 0 &&
        y >= 0 &&
        (static_cast<int32_t>(x) + w) <= Config::SCREEN_WIDTH &&
        (static_cast<int32_t>(y) + h) <= Config::SCREEN_HEIGHT;
}

inline void fillPixels(
    uint16_t* ptr,
    size_t count,
    uint16_t color) {

    if (!ptr || count == 0) return;

    if (count >=
        Config::Framebuffer::SPAN_MEMSET_THRESHOLD_PIXELS) {

        if (color == 0x0000U) {
            memset(ptr, 0x00, count * sizeof(uint16_t));
            return;
        }

        if (color == 0xFFFFU) {
            memset(ptr, 0xFF, count * sizeof(uint16_t));
            return;
        }
    }

    // Align the bulk writer to 32 bits. A framebuffer pixel is naturally
    // 16-bit aligned, so at most one leading pixel needs scalar handling.
    if ((reinterpret_cast<uintptr_t>(ptr) & 0x3U) != 0U) {
        *ptr++ = color;
        --count;
    }

    if (count == 0) return;

    const uint32_t packed_color =
        (static_cast<uint32_t>(color) << 16) |
        static_cast<uint32_t>(color);

    size_t pair_count = count >> 1;
    uint32_t* pair_ptr = reinterpret_cast<uint32_t*>(ptr);

    // Four 32-bit stores write eight RGB565 pixels per bulk iteration.
    while (pair_count >= 4U) {
        pair_ptr[0] = packed_color;
        pair_ptr[1] = packed_color;
        pair_ptr[2] = packed_color;
        pair_ptr[3] = packed_color;
        pair_ptr += 4;
        pair_count -= 4U;
    }

    while (pair_count > 0U) {
        *pair_ptr++ = packed_color;
        --pair_count;
    }

    if (count & 1U) {
        *reinterpret_cast<uint16_t*>(pair_ptr) = color;
    }
}

inline void fillSpan(
    uint16_t* ptr,
    int16_t count,
    uint16_t color) {

    if (count <= 0) return;

    fillPixels(
        ptr,
        static_cast<size_t>(count),
        color);
}

inline void fillRectUnchecked(
    uint16_t* buffer,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t color) {

    uint16_t* row_ptr =
        buffer +
        (static_cast<int32_t>(y) * Config::SCREEN_WIDTH) +
        x;

    // A complete full-width band is contiguous in the row-major framebuffer.
    if (x == 0 && w == Config::SCREEN_WIDTH) {
        fillPixels(
            row_ptr,
            static_cast<size_t>(Config::SCREEN_WIDTH) *
                static_cast<size_t>(h),
            color);
        return;
    }

    for (int16_t row = 0; row < h; ++row) {
        fillSpan(row_ptr, w, color);
        row_ptr += Config::SCREEN_WIDTH;
    }
}

inline void drawHLineUnchecked(
    uint16_t* buffer,
    int16_t x,
    int16_t y,
    int16_t w,
    uint16_t color) {

    fillSpan(
        buffer +
            (static_cast<int32_t>(y) * Config::SCREEN_WIDTH) +
            x,
        w,
        color);
}

inline void drawVLineUnchecked(
    uint16_t* buffer,
    int16_t x,
    int16_t y,
    int16_t h,
    uint16_t color) {

    uint16_t* ptr =
        buffer +
        (static_cast<int32_t>(y) * Config::SCREEN_WIDTH) +
        x;

    for (int16_t i = 0; i < h; ++i) {
        *ptr = color;
        ptr += Config::SCREEN_WIDTH;
    }
}

inline void drawVerticalPairUnchecked(
    uint16_t* buffer,
    int16_t left_x,
    int16_t right_x,
    int16_t y,
    int16_t h,
    uint16_t color) {

    uint16_t* left =
        buffer +
        (static_cast<int32_t>(y) * Config::SCREEN_WIDTH) +
        left_x;

    uint16_t* right =
        left + (right_x - left_x);

    for (int16_t row = 0; row < h; ++row) {
        *left = color;
        *right = color;

        left += Config::SCREEN_WIDTH;
        right += Config::SCREEN_WIDTH;
    }
}

} // namespace

void drawPixel(uint16_t* buffer, int16_t x, int16_t y, uint16_t color) {
    if (!buffer ||
        x < 0 || x >= Config::SCREEN_WIDTH ||
        y < 0 || y >= Config::SCREEN_HEIGHT) {
        return;
    }

    buffer[
        static_cast<int32_t>(y) *
        Config::SCREEN_WIDTH +
        x] = color;
}

void fillRect(
    uint16_t* buffer,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t color) {

    if (!buffer || w <= 0 || h <= 0) return;

    if (rectFullyInside(x, y, w, h)) {
        fillRectUnchecked(buffer, x, y, w, h, color);
        return;
    }

    int32_t x0 = x;
    int32_t y0 = y;
    int32_t x1 = x0 + w;
    int32_t y1 = y0 + h;

    if (x1 <= 0 ||
        y1 <= 0 ||
        x0 >= Config::SCREEN_WIDTH ||
        y0 >= Config::SCREEN_HEIGHT) {
        return;
    }

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > Config::SCREEN_WIDTH) x1 = Config::SCREEN_WIDTH;
    if (y1 > Config::SCREEN_HEIGHT) y1 = Config::SCREEN_HEIGHT;

    const int16_t clipped_w =
        static_cast<int16_t>(x1 - x0);
    const int16_t clipped_h =
        static_cast<int16_t>(y1 - y0);

    if (clipped_w <= 0 || clipped_h <= 0) return;

    fillRectUnchecked(
        buffer,
        static_cast<int16_t>(x0),
        static_cast<int16_t>(y0),
        clipped_w,
        clipped_h,
        color);
}

void drawHLine(
    uint16_t* buffer,
    int16_t x,
    int16_t y,
    int16_t w,
    uint16_t color) {

    if (!buffer ||
        w <= 0 ||
        y < 0 ||
        y >= Config::SCREEN_HEIGHT) {
        return;
    }

    if (x >= 0 &&
        (static_cast<int32_t>(x) + w) <=
            Config::SCREEN_WIDTH) {

        drawHLineUnchecked(buffer, x, y, w, color);
        return;
    }

    int32_t x0 = x;
    int32_t x1 = x0 + w;

    if (x1 <= 0 || x0 >= Config::SCREEN_WIDTH) return;

    if (x0 < 0) x0 = 0;
    if (x1 > Config::SCREEN_WIDTH) x1 = Config::SCREEN_WIDTH;

    const int16_t clipped_w =
        static_cast<int16_t>(x1 - x0);

    if (clipped_w <= 0) return;

    drawHLineUnchecked(
        buffer,
        static_cast<int16_t>(x0),
        y,
        clipped_w,
        color);
}

void drawVLine(
    uint16_t* buffer,
    int16_t x,
    int16_t y,
    int16_t h,
    uint16_t color) {

    if (!buffer ||
        h <= 0 ||
        x < 0 ||
        x >= Config::SCREEN_WIDTH) {
        return;
    }

    if (y >= 0 &&
        (static_cast<int32_t>(y) + h) <=
            Config::SCREEN_HEIGHT) {

        drawVLineUnchecked(buffer, x, y, h, color);
        return;
    }

    int32_t y0 = y;
    int32_t y1 = y0 + h;

    if (y1 <= 0 || y0 >= Config::SCREEN_HEIGHT) return;

    if (y0 < 0) y0 = 0;
    if (y1 > Config::SCREEN_HEIGHT) y1 = Config::SCREEN_HEIGHT;

    const int16_t clipped_h =
        static_cast<int16_t>(y1 - y0);

    if (clipped_h <= 0) return;

    drawVLineUnchecked(
        buffer,
        x,
        static_cast<int16_t>(y0),
        clipped_h,
        color);
}

void drawRect(
    uint16_t* buffer,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t color) {

    if (!buffer || w <= 0 || h <= 0) return;

    if (rectFullyInside(x, y, w, h)) {
        drawHLineUnchecked(buffer, x, y, w, color);

        if (h > 1) {
            drawHLineUnchecked(
                buffer,
                x,
                static_cast<int16_t>(y + h - 1),
                w,
                color);
        }

        if (h > 2) {
            const int16_t edge_y =
                static_cast<int16_t>(y + 1);
            const int16_t edge_h =
                static_cast<int16_t>(h - 2);

            if (w > 1) {
                drawVerticalPairUnchecked(
                    buffer,
                    x,
                    static_cast<int16_t>(x + w - 1),
                    edge_y,
                    edge_h,
                    color);
            } else {
                drawVLineUnchecked(
                    buffer,
                    x,
                    edge_y,
                    edge_h,
                    color);
            }
        }

        return;
    }

    const int32_t left = x;
    const int32_t top = y;
    const int32_t right = left + w - 1;
    const int32_t bottom = top + h - 1;

    if (right < 0 ||
        bottom < 0 ||
        left >= Config::SCREEN_WIDTH ||
        top >= Config::SCREEN_HEIGHT) {
        return;
    }

    const int32_t visible_left =
        left < 0 ? 0 : left;

    const int32_t visible_right =
        right >= Config::SCREEN_WIDTH
            ? Config::SCREEN_WIDTH - 1
            : right;

    const int32_t visible_top =
        top < 0 ? 0 : top;

    const int32_t visible_bottom =
        bottom >= Config::SCREEN_HEIGHT
            ? Config::SCREEN_HEIGHT - 1
            : bottom;

    if (top >= 0 && top < Config::SCREEN_HEIGHT) {
        fillPixels(
            buffer +
                (top * Config::SCREEN_WIDTH) +
                visible_left,
            static_cast<size_t>(
                visible_right - visible_left + 1),
            color);
    }

    if (bottom != top &&
        bottom >= 0 &&
        bottom < Config::SCREEN_HEIGHT) {

        fillPixels(
            buffer +
                (bottom * Config::SCREEN_WIDTH) +
                visible_left,
            static_cast<size_t>(
                visible_right - visible_left + 1),
            color);
    }

    const bool left_visible =
        left >= 0 &&
        left < Config::SCREEN_WIDTH;

    const bool right_visible =
        right != left &&
        right >= 0 &&
        right < Config::SCREEN_WIDTH;

    const int16_t edge_y =
        static_cast<int16_t>(visible_top);

    const int16_t edge_h =
        static_cast<int16_t>(
            visible_bottom - visible_top + 1);

    if (left_visible && right_visible) {
        drawVerticalPairUnchecked(
            buffer,
            static_cast<int16_t>(left),
            static_cast<int16_t>(right),
            edge_y,
            edge_h,
            color);
    } else if (left_visible) {
        drawVLineUnchecked(
            buffer,
            static_cast<int16_t>(left),
            edge_y,
            edge_h,
            color);
    } else if (right_visible) {
        drawVLineUnchecked(
            buffer,
            static_cast<int16_t>(right),
            edge_y,
            edge_h,
            color);
    }
}

void drawBitmap(
    uint16_t* buffer,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    const uint16_t* bitmap) {

    if (!buffer || !bitmap || w <= 0 || h <= 0) return;

    if (rectFullyInside(x, y, w, h)) {
        if (x == 0 && w == Config::SCREEN_WIDTH) {
            memcpy(
                buffer +
                    (static_cast<int32_t>(y) *
                     Config::SCREEN_WIDTH),
                bitmap,
                static_cast<size_t>(w) *
                    static_cast<size_t>(h) *
                    sizeof(uint16_t));
            return;
        }

        uint16_t* dest_row =
            buffer +
            (static_cast<int32_t>(y) *
             Config::SCREEN_WIDTH) +
            x;

        const uint16_t* src_row = bitmap;

        const size_t row_bytes =
            static_cast<size_t>(w) * sizeof(uint16_t);

        for (int16_t row = 0; row < h; ++row) {
            memcpy(dest_row, src_row, row_bytes);
            dest_row += Config::SCREEN_WIDTH;
            src_row += w;
        }

        return;
    }

    int32_t x0 = x;
    int32_t y0 = y;
    int32_t x1 = x0 + w;
    int32_t y1 = y0 + h;

    if (x1 <= 0 ||
        y1 <= 0 ||
        x0 >= Config::SCREEN_WIDTH ||
        y0 >= Config::SCREEN_HEIGHT) {
        return;
    }

    int16_t src_offset_x = 0;
    int16_t src_offset_y = 0;
    const int16_t original_w = w;

    if (x0 < 0) {
        src_offset_x = static_cast<int16_t>(-x0);
        x0 = 0;
    }

    if (y0 < 0) {
        src_offset_y = static_cast<int16_t>(-y0);
        y0 = 0;
    }

    if (x1 > Config::SCREEN_WIDTH) x1 = Config::SCREEN_WIDTH;
    if (y1 > Config::SCREEN_HEIGHT) y1 = Config::SCREEN_HEIGHT;

    const int16_t clipped_w =
        static_cast<int16_t>(x1 - x0);

    const int16_t clipped_h =
        static_cast<int16_t>(y1 - y0);

    if (clipped_w <= 0 || clipped_h <= 0) return;

    uint16_t* dest_row =
        buffer +
        (x0 + (y0 * Config::SCREEN_WIDTH));

    const uint16_t* src_row =
        bitmap +
        (static_cast<int32_t>(src_offset_y) *
         original_w) +
        src_offset_x;

    const size_t bytes_to_copy =
        static_cast<size_t>(clipped_w) *
        sizeof(uint16_t);

    for (int16_t row = 0; row < clipped_h; ++row) {
        memcpy(dest_row, src_row, bytes_to_copy);
        dest_row += Config::SCREEN_WIDTH;
        src_row += original_w;
    }
}

} // namespace Blitters
