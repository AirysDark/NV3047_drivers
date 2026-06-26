#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// UI DRAWING PRIMITIVES
// ======================================================

/**
 * @brief Fill a rectangular area on the framebuffer
 *
 * @param x      Top-left X coordinate
 * @param y      Top-left Y coordinate
 * @param w      Width of rectangle
 * @param h      Height of rectangle
 * @param color  16-bit RGB565 color
 */
void ui_fill(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

#ifdef __cplusplus
}
#endif