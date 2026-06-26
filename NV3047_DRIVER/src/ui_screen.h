#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// UI SCREEN CALLBACK TYPES
// ======================================================

typedef void (*ui_screen_draw_t)(void);
typedef void (*ui_screen_event_t)(int16_t x, int16_t y, bool pressed);

// ======================================================
// UI SCREEN STRUCTURE
// ======================================================

typedef struct {
    ui_screen_draw_t draw;
    ui_screen_event_t event;
} ui_screen_t;

// ======================================================
// UI SCREEN API
// ======================================================

/**
 * @brief Set active UI screen
 */
void ui_screen_set(ui_screen_t *screen);

/**
 * @brief Draw current active screen
 */
void ui_screen_draw(void);

/**
 * @brief Send touch/input event to active screen
 */
void ui_screen_event(int16_t x, int16_t y, bool pressed);

#ifdef __cplusplus
}
#endif