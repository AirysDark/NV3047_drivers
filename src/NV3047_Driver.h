#pragma once

#include "NV3047.h"

class NV3047_Driver {
private:
    NV3047* hardware = nullptr;

public:
    bool begin(NV3047* hw_instance);

    bool isReady() const { return hardware != nullptr; }

    void setBrightness(uint8_t percentage);
    void sleep();
    void wake();

    void pushPixels(int x, int y, int w, int h, const uint16_t* data);
    void clear(uint16_t color = Config::COLOR_BLACK);
    void fillScreen(uint16_t color);
    bool present();

    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    bool getTouch(uint16_t &x, uint16_t &y);

    bool isSDMounted() const;
    SDCardDriver* getSDCard();
    const SDCardDriver* getSDCard() const;

    Framebuffer* getCanvas();
    const Framebuffer* getCanvas() const;
};
