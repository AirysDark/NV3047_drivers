#pragma once

#include "../Config.h"

#include <SPI.h>
#include <driver/spi_master.h>

class SPI_Master {
public:
    // Initializes the selected profile's touch transport.
    // LEGACY_WORKING: dedicated ESP-IDF SPI device, matching old main.
    // V21_MATRIX_TEST: Arduino SPIClass shared by touch + TF.
    static bool init();

    // V2.1 shared Arduino SPI bus.
    static SPIClass& bus();

    // LEGACY_WORKING dedicated XPT2046 device handle.
    static spi_device_handle_t legacyTouchDevice();
};
