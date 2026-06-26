#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// DISPLAY MANAGER API (WIDESCREEN 480x272 LAYER ENGINE)
// ======================================================

/**
 * @brief Allocates the 480x272 DMA memory matrix pool and flushes a clean initial frame.
 * @param panel Structured pointer address reference targeting your NV3047 instance handler.
 * @return ESP_OK if memory bounds setup completes flawlessly.
 */
esp_err_t display_manager_init(esp_lcd_panel_handle_t panel);

/**
 * @brief Fills the widescreen active memory map with a uniform color.
 */
void display_clear(uint16_t color, esp_lcd_panel_handle_t panel);

/**
 * @brief Draws a vertical Red, Green, and Blue test matrix grid across the 480-pixel landscape surface.
 */
void display_test_pattern(esp_lcd_panel_handle_t panel);

#ifdef __cplusplus
}
#endif
