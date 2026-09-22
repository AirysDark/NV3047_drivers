#pragma once

#include "../Config.h"
#include <driver/spi_master.h>

class SPI_Master {
public:
    // Initializes the board's shared peripheral SPI bus.
    // Touch is the active device today; TF/microSD is expected to share the
    // same SCLK/MOSI/MISO lines once its separate CS pin is confirmed.
    static bool init();

    // Adds one SPI device to the already-initialized shared bus.
    static spi_device_handle_t addDevice(int cs_pin, int clock_speed_hz);
};
