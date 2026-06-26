#pragma once

#include <Arduino_GFX_Library.h>
#include <lvgl.h>

class DDriver
{
public:

    // =========================
    // DISPLAY OBJECTS
    // =========================

    Arduino_ESP32RGBPanel *bus;
    Arduino_RPi_DPI_RGBPanel *gfx;

    // =========================
    // LVGL OBJECTS
    // =========================

    lv_disp_draw_buf_t draw_buf;

    lv_color_t* buf;

    lv_disp_drv_t disp_drv;

    // =========================
    // CONSTRUCTOR
    // =========================

    DDriver();

    // =========================
    // BASIC DISPLAY
    // =========================

    void begin();

    void setBacklight(bool state);

    void testPattern();

    void printText(
        const char* text,
        int x,
        int y,
        uint16_t color = WHITE,
        int size = 2
    );

    // =========================
    // LVGL
    // =========================

    void beginLVGL();

    static void lvglFlush(
        lv_disp_drv_t *disp,
        const lv_area_t *area,
        lv_color_t *color_p
    );
};

// =========================
// GLOBAL INSTANCE
// =========================

extern DDriver Display;