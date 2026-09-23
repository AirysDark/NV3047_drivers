#pragma once

#include "NV3047.h"

class NV3047_Driver {
private:
    NV3047* hardware = nullptr;
    Framebuffer* canvas = nullptr;

public:
    bool begin(NV3047* hw_instance);

    bool isReady() const { return hardware != nullptr && canvas != nullptr; }
    bool isExternalMemoryManagerActive() const;

    void setBrightness(uint8_t percentage);
    void sleep();
    void wake();

    inline void pushPixels(
        int x,
        int y,
        int w,
        int h,
        const uint16_t* data) {

        if (canvas) canvas->drawBitmap(x, y, w, h, data);
    }

    inline void clear(uint16_t color = Config::COLOR_BLACK) {
        if (canvas) canvas->clear(color);
    }

    inline void fillScreen(uint16_t color) {
        if (!canvas) return;
        canvas->clear(color);
        canvas->swap();
    }

    inline bool present() {
        return canvas && canvas->swap();
    }

    inline void drawPixel(
        int16_t x,
        int16_t y,
        uint16_t color) {

        if (canvas) canvas->drawPixel(x, y, color);
    }

    inline void fillRect(
        int16_t x,
        int16_t y,
        int16_t w,
        int16_t h,
        uint16_t color) {

        if (canvas) canvas->fillRect(x, y, w, h, color);
    }

    inline void drawHLine(
        int16_t x,
        int16_t y,
        int16_t w,
        uint16_t color) {

        if (canvas) canvas->drawHLine(x, y, w, color);
    }

    inline void drawVLine(
        int16_t x,
        int16_t y,
        int16_t h,
        uint16_t color) {

        if (canvas) canvas->drawVLine(x, y, h, color);
    }

    inline void drawRect(
        int16_t x,
        int16_t y,
        int16_t w,
        int16_t h,
        uint16_t color) {

        if (canvas) canvas->drawRect(x, y, w, h, color);
    }

    bool getTouch(uint16_t &x, uint16_t &y);

    bool isSDMounted() const;
    SDCardDriver* getSDCard();
    const SDCardDriver* getSDCard() const;

    inline Framebuffer* getCanvas() { return canvas; }
    inline const Framebuffer* getCanvas() const { return canvas; }
};
