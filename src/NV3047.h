#pragma once

#include "Bus_Layers/SPI_Master.h"
#include "Peripherals_HAL/TouchDriver.h"
#include "Bus_Layers/RGB.h"
#include "Peripherals_HAL/DisplayDriver.h"
#include "Peripherals_HAL/SDCardDriver.h"
#include "Core_Matrices/framebuffer.h"

class NV3047 {
private:
    SPI_Master spiMaster;
    TouchDriver touch;
    SDCardDriver sdCard;
    RGB rgbBus;
    DisplayDriver display;
    Framebuffer fb;

public:
    bool init();

    DisplayDriver& getDisplay();
    TouchDriver& getTouch();
    SDCardDriver& getSDCard();
    const SDCardDriver& getSDCard() const;
    Framebuffer& getCanvas();
};
