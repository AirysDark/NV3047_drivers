#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_ops.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// NV3047 PANEL DRIVER API
// ======================================================

esp_err_t nv3047_new_panel(
    const esp_lcd_panel_io_handle_t io,
    esp_lcd_panel_handle_t *panel
);

// low-level driver
esp_err_t nv3047_init(void);

// graphics API
void nv3047_fill(uint16_t color);
void nv3047_test_pattern(void);

#ifdef __cplusplus
}
#endif