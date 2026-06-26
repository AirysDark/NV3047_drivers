#include "DDriver.h"

DDriver Display;

// =====================================================
// CONSTRUCTOR
// =====================================================

DDriver::DDriver()
{
    // =================================================
    // RGB BUS
    // =================================================

    bus = new Arduino_ESP32RGBPanel(
        GFX_NOT_DEFINED,
        GFX_NOT_DEFINED,
        GFX_NOT_DEFINED,

        40, // DE
        41, // VSYNC
        39, // HSYNC
        42, // PCLK

        45,
        48,
        47,
        21,
        14,

        5,
        6,
        7,
        15,
        16,
        4,

        8,
        3,
        46,
        9,
        1
    );

    // =================================================
    // PANEL
    // =================================================

    gfx = new Arduino_RPi_DPI_RGBPanel(
        bus,

        480,
        0,
        8,
        4,
        43,

        272,
        0,
        8,
        4,
        12,

        1,
        7000000,
        true
    );

    buf = nullptr;
}

// =====================================================
// BASIC DISPLAY INIT
// =====================================================

void DDriver::begin()
{
    // LCD ENABLE
    pinMode(38, OUTPUT);
    digitalWrite(38, LOW);

    // BACKLIGHT
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);

    delay(200);

    gfx->begin();

    gfx->fillScreen(BLACK);
}

// =====================================================
// BACKLIGHT
// =====================================================

void DDriver::setBacklight(bool state)
{
    digitalWrite(2, state ? HIGH : LOW);
}

// =====================================================
// TEST PATTERN
// =====================================================

void DDriver::testPattern()
{
    gfx->fillScreen(RED);
    delay(500);

    gfx->fillScreen(GREEN);
    delay(500);

    gfx->fillScreen(BLUE);
    delay(500);

    gfx->fillScreen(BLACK);
}

// =====================================================
// TEXT DRAW
// =====================================================

void DDriver::printText(
    const char* text,
    int x,
    int y,
    uint16_t color,
    int size
)
{
    gfx->setCursor(x, y);

    gfx->setTextColor(color);

    gfx->setTextSize(size);

    gfx->println(text);
}

// =====================================================
// LVGL FLUSH CALLBACK
// =====================================================

void DDriver::lvglFlush(
    lv_disp_drv_t *disp,
    const lv_area_t *area,
    lv_color_t *color_p
)
{
    uint32_t w =
        area->x2 - area->x1 + 1;

    uint32_t h =
        area->y2 - area->y1 + 1;

    Display.gfx->draw16bitRGBBitmap(
        area->x1,
        area->y1,
        (uint16_t *)&color_p->full,
        w,
        h
    );

    lv_disp_flush_ready(disp);
}

// =====================================================
// LVGL INIT
// =====================================================

void DDriver::beginLVGL()
{
    lv_init();

    // =============================================
    // DRAW BUFFER
    // =============================================

    uint32_t bufferSize =
        480 * 40;

    buf = (lv_color_t*)ps_malloc(
        sizeof(lv_color_t) * bufferSize
    );

    lv_disp_draw_buf_init(
        &draw_buf,
        buf,
        NULL,
        bufferSize
    );

    // =============================================
    // DISPLAY DRIVER
    // =============================================

    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = 480;

    disp_drv.ver_res = 272;

    disp_drv.flush_cb = lvglFlush;

    disp_drv.draw_buf = &draw_buf;

    lv_disp_drv_register(&disp_drv);
}