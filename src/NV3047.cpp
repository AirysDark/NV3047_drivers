#include "NV3047.h"
#include <Arduino.h>

bool NV3047::init() {
    // 1. Initialize SPI Bus and attach Touch Device
    if (!spiMaster.init()) return false;
    spi_device_handle_t touch_handle = spiMaster.addDevice(Config::PIN_TOUCH_CS, 1000000);
    if (!touch.init(touch_handle)) return false;

    // 2. Initialize RGB Parallel Bus
    if (!rgbBus.init()) return false;

    // 3. Initialize Display Subsystem
    if (!display.init(rgbBus.getHandle())) return false;

    // 4. Bind the Integrated Dual Hardware Framebuffers to the Canvas Manager
    if (!fb.init(rgbBus.getHandle())) return false;

    return true;
}

DisplayDriver& NV3047::getDisplay() { return display; }
TouchDriver& NV3047::getTouch() { return touch; }
Framebuffer& NV3047::getCanvas() { return fb; }
