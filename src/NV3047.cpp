#include "NV3047.h"

bool NV3047::init() {
    // Bring up the selected hardware profile's touch SPI wiring first.
    // LEGACY_WORKING preserves main's GPIO20/19 + GPIO18 CS map.
    // V21_MATRIX_TEST uses GPIO12/11/13 + GPIO0 CS and also de-asserts SD CS.
    if (!spiMaster.init()) return false;
    if (!touch.init(spiMaster.bus())) return false;

    if (!rgbBus.init()) return false;
    if (!display.init(rgbBus.getHandle())) return false;
    if (!fb.init(rgbBus.getHandle())) return false;

    // V21_MATRIX_TEST represents the complete V2.1 peripheral profile.
    // Try to mount TF automatically, but do not make LCD/touch startup depend
    // on a card being physically inserted.
    if (Config::SDCard::AUTO_MOUNT_WITH_ACTIVE_PROFILE &&
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
