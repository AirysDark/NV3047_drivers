#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// NV3047 HARDWARE COMMAND SET (REGISTRY LATCHES)
// ======================================================

#define NV3047_CMD_SLEEP_OUT      0x11
#define NV3047_CMD_DISPLAY_ON     0x29
#define NV3047_CMD_DISPLAY_OFF    0x28
#define NV3047_CMD_PIXEL_FORMAT   0x3A
#define NV3047_CMD_MADCTL         0x36
#define NV3047_CMD_CASET          0x2A
#define NV3047_CMD_RASET          0x2B
#define NV3047_CMD_RAMWR          0x2C

// Manufacturer Extensions (Unlocked configurations)
#define NV3047_CMD_EXT_UNLOCK     0xFF
#define NV3047_CMD_FRAME_RATE     0xB1
#define NV3047_CMD_INTERFACE_CTRL 0xB4

// ======================================================
// LOW LEVEL DRIVER API
// ======================================================

// Lifecycle (RAW driver hooks implemented across the library modules)
esp_err_t nv3047_init(void);
esp_err_t nv3047_sleep_out(void);
esp_err_t nv3047_display_on(void);
esp_err_t nv3047_display_off(void);

// Configuration Matrix
esp_err_t nv3047_set_rotation(uint8_t rotation);
esp_err_t nv3047_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// Pixel Stream Data Buffer I/O
esp_err_t nv3047_write_pixels(const uint16_t *data, size_t len);

// ======================================================
// ESP LCD WRAPPER API (COMPATIBILITY LAYER)
// ======================================================

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"

/**
 * @brief Instantiates a unified esp_lcd driver instance for the NV3047 layout.
 * @param io Hardware communication handle pointing to the bus setup
 * @param panel_out Target reference container for the structured panel engine output
 * @return ESP_OK if instantiation completes successfully without timing faults
 */
esp_err_t nv3047_new_panel(
    const esp_lcd_panel_io_handle_t io,
    esp_lcd_panel_handle_t *panel_out
);

#ifdef __cplusplus
}
#endif
