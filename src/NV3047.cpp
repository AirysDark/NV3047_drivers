#include "NV3047.h"

bool NV3047::init() {
    if (!spiMaster.init()) return false;

    spi_device_handle_t touch_handle =
        spiMaster.addDevice(
            Config::PIN_TOUCH_CS,
            Config::SPI::TOUCH_CLOCK_HZ);

    if (!touch_handle) return false;
    if (!touch.init(touch_handle)) return false;

    if (!rgbBus.init()) return false;
    if (!display.init(rgbBus.getHandle())) return false;
    if (!fb.init(rgbBus.getHandle())) return false;

    return true;
}

DisplayDriver& NV3047::getDisplay() {
    return display;
}

TouchDriver& NV3047::getTouch() {
    return touch;
}

Framebuffer& NV3047::getCanvas() {
    return fb;
}
