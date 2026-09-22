#include "NV3047_Driver.h"

bool NV3047_Driver::begin(NV3047* hw_instance) {
    hardware = nullptr;

    if (!hw_instance) return false;
    if (!hw_instance->init()) return false;

    hardware = hw_instance;
    setBrightness(Config::Display::DEFAULT_BRIGHTNESS_PERCENT);
    return true;
}

void NV3047_Driver::setBrightness(uint8_t percentage) {
    if (hardware) {
        hardware->getDisplay().setBrightness(percentage);
    }
}

void NV3047_Driver::sleep() {
    if (hardware) {
        hardware->getDisplay().sleep();
    }
}

void NV3047_Driver::wake() {
    if (hardware) {
        hardware->getDisplay().wake();
    }
}

void NV3047_Driver::pushPixels(int x, int y, int w, int h, const uint16_t* data) {
    if (hardware) {
        hardware->getCanvas().drawBitmap(x, y, w, h, data);
    }
}

void NV3047_Driver::clear(uint16_t color) {
    if (hardware) {
        hardware->getCanvas().clear(color);
    }
}

void NV3047_Driver::fillScreen(uint16_t color) {
    if (!hardware) return;

    hardware->getCanvas().clear(color);
    hardware->getCanvas().swap();
}

bool NV3047_Driver::present() {
    return hardware && hardware->getCanvas().swap();
}

void NV3047_Driver::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (hardware) hardware->getCanvas().drawPixel(x, y, color);
}

void NV3047_Driver::fillRect(
    int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (hardware) hardware->getCanvas().fillRect(x, y, w, h, color);
}

void NV3047_Driver::drawHLine(
    int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (hardware) hardware->getCanvas().drawHLine(x, y, w, color);
}

void NV3047_Driver::drawVLine(
    int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (hardware) hardware->getCanvas().drawVLine(x, y, h, color);
}

void NV3047_Driver::drawRect(
    int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (hardware) hardware->getCanvas().drawRect(x, y, w, h, color);
}

bool NV3047_Driver::getTouch(uint16_t &x, uint16_t &y) {
    return hardware && hardware->getTouch().getTouch(x, y);
}

Framebuffer* NV3047_Driver::getCanvas() {
    return hardware ? &hardware->getCanvas() : nullptr;
}

const Framebuffer* NV3047_Driver::getCanvas() const {
    return hardware ? &hardware->getCanvas() : nullptr;
}
