#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// UI EVENT STRUCTURE
// ======================================================

typedef struct {
    int16_t x;
    int16_t y;
    bool pressed;
} ui_event_t;

// ======================================================
// UI CORE API
// ======================================================

/**
 * @brief Initialise UI system
 */
void ui_init(void);

/**
 * @brief UI main tick (handles input + logic)
 * @param event Pointer to latest input event
 */
void ui_tick(ui_event_t *event);

/**
 * @brief Render UI to framebuffer
 */
void ui_draw(void);

/**
 * @brief Optional helper: check if UI is active
 */
bool ui_is_active(void);

#ifdef __cplusplus
}
#endif