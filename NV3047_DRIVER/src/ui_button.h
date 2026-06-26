#pragma once

#include "ui_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// UI BUTTON API
// ======================================================

/**
 * @brief Initialise a UI button widget
 * @param w Pointer to generic UI widget structure
 */
void ui_button_init(ui_widget_t *w);

/**
 * @brief Handle button update logic (press detection, state update)
 * @param w Pointer to UI widget
 * @param x Touch X coordinate
 * @param y Touch Y coordinate
 * @param pressed Touch state
 */
void ui_button_update(ui_widget_t *w, int16_t x, int16_t y, bool pressed);

/**
 * @brief Check if button is currently pressed
 */
bool ui_button_is_pressed(ui_widget_t *w);

#ifdef __cplusplus
}
#endif