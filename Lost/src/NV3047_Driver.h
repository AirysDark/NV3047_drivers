#pragma once
#include "NV3047.h"

class NV3047_Driver {
private:
    // Tracks your active global hardware manager via a safe reference pointer
    NV3047* hardware;

public:
    // Dynamic lifecycle entry: Binds the live running hardware stack directly into the driver engine
    void begin(NV3047* hw_instance);
    
    // Panel power management pipeline
    void setBrightness(uint8_t brightness);
    
    // High-performance canvas streaming methods
    void pushPixels(int x, int y, int w, int h, const uint16_t* data);
    void fillScreen(uint16_t color);
    
    // Calibrated touch tracking engine proxy
    bool getTouch(uint16_t &x, uint16_t &y);
};
