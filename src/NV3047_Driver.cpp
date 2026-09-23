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

