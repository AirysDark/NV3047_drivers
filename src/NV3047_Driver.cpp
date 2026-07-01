#include "NV3047_Driver.h"

void NV3047_Driver::begin(NV3047* hw_instance) {
    // Bind the global running hardware stack pointer to this driver proxy instance
    hardware = hw_instance;
    
    if (hardware) {
        hardware->init();
        setBrightness(128); // Matches your original bare-metal setup level cleanly
    }
}

void NV3047_Driver::setBrightness(uint8_t brightness) {
    if (hardware) {
        // Redirects straight to your optimized backlight power engine percentages
        hardware->getDisplay().setBrightness(brightness);
    }
}

void NV3047_Driver::pushPixels(int x, int y, int w, int h, const uint16_t* data) {
    if (hardware) {
        // Streams the data block array safely onto your canvas primitive layers
        hardware->getCanvas().drawBitmap(x, y, w, h, data);
    }
}

void NV3047_Driver::fillScreen(uint16_t color) {
    if (hardware) {
        // Leverages your high-performance 32-bit clearing blocks directly
        hardware->getCanvas().clear(color);
        
        // --- FIXED: AUTOMATIC SWAP FOR BARE-METAL SKETCHES ---
        // Forces the double-buffered canvas to flip straight to the display glass.
        // This immediately releases the memory lock and opens the SPI clock lines wide 
        // so your updated self-throttling touch driver can read your finger normally!
        hardware->getCanvas().swap();
    }
}

bool NV3047_Driver::getTouch(uint16_t &x, uint16_t &y) {
    if (!hardware) return false;
    
    // Pulls from your precise multi-sampled orientation remapping matrix
    return hardware->getTouch().getTouch(x, y);
}
