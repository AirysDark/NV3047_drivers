#include "NV3047.h"

bool NV3047::init() {
    // Bring up the fixed dedicated XPT2046 touch transport.
    if (!spiMaster.init()) return false;
    if (!touch.init(spiMaster.touchDevice())) return false;

    if (!rgbBus.init()) return false;
    if (!display.init(rgbBus.getHandle())) return false;
    if (!fb.init(rgbBus.getHandle())) return false;

    if (Config::SDCard::AUTO_MOUNT &&
        sdCard.isConfigured()) {
        sdCard.init();
    }

    return true;
}

DisplayDriver& NV3047::getDisplay() {
    return display;
}

TouchDriver& NV3047::getTouch() {
    return touch;
}

SDCardDriver& NV3047::getSDCard() {
    return sdCard;
}

const SDCardDriver& NV3047::getSDCard() const {
    return sdCard;
}

Framebuffer& NV3047::getCanvas() {
    return fb;
}
