#include "NV3047_Driver.h"

bool NV3047_Driver::begin(NV3047* hw_instance) {
    hardware = nullptr;
    canvas = nullptr;

    if (!hw_instance) return false;
    if (!hw_instance->init()) return false;

    hardware = hw_instance;
    canvas = &hardware->getCanvas();

    if (!canvas || !canvas->isReady()) {
        hardware = nullptr;
        canvas = nullptr;
        return false;
    }

    setBrightness(Config::Display::DEFAULT_BRIGHTNESS_PERCENT);
    return true;
}

bool NV3047_Driver::isExternalMemoryManagerActive() const {
    return
        canvas &&
        canvas->getMemoryManager().
            isExternalProviderActive();
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
    if (canvas) {
        canvas->drawBitmap(x, y, w, h, data);
    }
}

void NV3047_Driver::clear(uint16_t color) {
    if (canvas) {
        canvas->clear(color);
    }
}

void NV3047_Driver::fillScreen(uint16_t color) {
    if (!canvas) return;

    canvas->clear(color);
    canvas->swap();
}

bool NV3047_Driver::present() {
    return canvas && canvas->swap();
}

void NV3047_Driver::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (canvas) canvas->drawPixel(x, y, color);
}

void NV3047_Driver::fillRect(
    int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (canvas) canvas->fillRect(x, y, w, h, color);
}

void NV3047_Driver::drawHLine(
    int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (canvas) canvas->drawHLine(x, y, w, color);
}

void NV3047_Driver::drawVLine(
    int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (canvas) canvas->drawVLine(x, y, h, color);
}

void NV3047_Driver::drawRect(
    int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (canvas) canvas->drawRect(x, y, w, h, color);
}

bool NV3047_Driver::getTouch(uint16_t &x, uint16_t &y) {
    return hardware && hardware->getTouch().getTouch(x, y);
}

bool NV3047_Driver::isSDMounted() const {
    return hardware && hardware->getSDCard().isMounted();
}

SDCardDriver* NV3047_Driver::getSDCard() {
    return hardware ? &hardware->getSDCard() : nullptr;
}

const SDCardDriver* NV3047_Driver::getSDCard() const {
    return hardware ? &hardware->getSDCard() : nullptr;
}

Framebuffer* NV3047_Driver::getCanvas() {
    return canvas;
}

const Framebuffer* NV3047_Driver::getCanvas() const {
    return canvas;
}
