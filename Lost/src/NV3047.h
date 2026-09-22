#pragma once
#include "Bus_Layers/SPI_Master.h"
#include "Peripherals_HAL/TouchDriver.h"
#include "Bus_Layers/RGB.h"
#include "Peripherals_HAL/DisplayDriver.h"
#include "Core_Matrices/framebuffer.h"

// --- LINK YOUR NEW INDEPENDENT MEMORY MANAGER SUB-MODULE ---
#include "Manager/NV3047_Memory.h"

class NV3047 {
private:
    NV3047_Memory memoryManager; // Private memory instance container
    SPI_Master spiMaster;
    TouchDriver touch;
    RGB rgbBus;
    DisplayDriver display;
    Framebuffer fb; // Centralised draw canvas management instance

public:
    bool init();
    DisplayDriver& getDisplay();
    TouchDriver& getTouch();
    Framebuffer& getCanvas(); // Exposes background buffer drawing actions directly
};
