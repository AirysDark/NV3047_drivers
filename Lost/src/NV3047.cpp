#include "NV3047.h"
#include <Arduino.h>

bool NV3047::init() {
    // =========================================================================
    //   CRITICAL ENTRY POINT: ALLOCATE SILICON CHUNKS BEFORE HARDWARE BOOTS
    // =========================================================================
    // Activates your clean dynamic memory allocation layer first.
    // It safely checks for available hardware domains and structures buffers 
    // without executing hardware-level register reset loops.
    if (!memoryManager.allocate()) {
        return false; // Safely drops out if the physical allocation fails
    }
    // =========================================================================

    // 1. Initialize SPI Bus and attach Touch Device
    if (!spiMaster.init()) return false;
    
    // Cleaned up the duplicate clock speed assignment! 
    // Since SPI_Master::addDevice now ignores this second parameter and pulls TOUCH_CLOCK_SPEED_HZ 
    // directly from Config.h internally, we pass 0 here to keep the API footprint clean.
    spi_device_handle_t touch_handle = spiMaster.addDevice(Config::PIN_TOUCH_CS, 0);
    
    // FIXED LINKAGE BOUNDS: Explicitly hands your dynamic internal RAM touch memory slot
    // straight to the hardware driver configuration layer to initialize communications.
    if (!touch.init(touch_handle, memoryManager.getTouchBuffer())) return false;

    // 2. Initialize RGB Parallel Bus
    if (!rgbBus.init()) return false;

    // 3. Initialize Display Subsystem
    if (!display.init(rgbBus.getHandle())) return false;

    // FIXED DISPLAY MATRIX LINKAGE: Pass the clean, pre-allocated memory pointers
    // straight to your Framebuffer initialization wrapper to un-blank your screen glass!
    if (!fb.init(rgbBus.getHandle(), memoryManager.getDisplayBuffer1(), memoryManager.getDisplayBuffer2())) {
        return false;
    }

    return true;
}

DisplayDriver& NV3047::getDisplay() { return display; }
TouchDriver& NV3047::getTouch() { return touch; }
Framebuffer& NV3047::getCanvas() { return fb; }
