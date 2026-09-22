#pragma once

#include "../Config.h"

#include <SPI.h>
#include <driver/spi_master.h>

class SPI_Master {
public:
    // Initializes the fixed dedicated XPT2046 touch SPI transport.
    static bool init();

    // Dedicated touch device handle used by TouchDriver.
    static spi_device_handle_t touchDevice();

    // Retained for the SDCardDriver API. The current fixed hardware
    // configuration leaves SD disabled.
    static SPIClass& bus();
};
