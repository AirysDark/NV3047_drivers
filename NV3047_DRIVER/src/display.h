#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Global pointer reference tracking the active NV3047 panel engine instance.
 *        Exposed publicly so your UI rendering pipeline can tap into standard Espressif LCD functions.
 */
extern esp_lcd_panel_handle_t panel_handle;

/**
 * @brief Allocates and initializes the hardware SPI panel IO wrapper engine interface.
 * @return Fully populated esp_lcd_panel_io_handle_t pointer slot, or NULL if initialization fails.
 */
esp_lcd_panel_io_handle_t lcd_spi_init(void);

/**
 * @brief Unified public entry point to spin up, attach, reset, and turn on the NV3047 screen panel subsystem.
 */
void display_init(void);

#ifdef __cplusplus
}
#endif
