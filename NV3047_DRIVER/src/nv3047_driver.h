#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h" // Ensures esp_err_t is properly compiled in Arduino/IDF environments
// EXPOSE SUBSYSTEMS TO THE MAIN SKETCH
#include "config.h"      // Now visible to your main sketch file!
#include "spi_master.h"  // Now visible to your main sketch file!


#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// NV3047 DRIVER CORE API (PUBLIC ONLY)
// ======================================================

void nv3047_driver_init(void);
void nv3047_driver_tick(void);
void nv3047_driver_draw(void);

void nv3047_driver_fill(uint16_t color);
void nv3047_driver_test_pattern(void);

// ======================================================
// INTERNAL HARDWARE LAYER (SPI BUS & CONTROLLER BACKEND)
// ======================================================

// Native NV3047 setup stubs called during engine spin-up
esp_err_t nv3047_init(void);
void nv3047_fill(uint16_t color);
void nv3047_test_pattern(void);

// Consolidated Low-Level SPI Hooks mapped directly into the library
void spi_master_init_bus(void);
void spi_master_send_command(uint8_t cmd);
void spi_master_send_data(const uint8_t* data, size_t length);
void spi_master_stream_pixels(const uint16_t* colors, size_t pixel_count);

#ifdef __cplusplus
}
#endif

// ======================================================
// C++ UI LAYER
// ======================================================

#ifdef __cplusplus

#include "ui.h"
#include "ui_screen.h"
#include "ui_render.h"
#include "ui_widget.h"
#include "ui_button.h"
#include "ui_theme.h"

#endif
