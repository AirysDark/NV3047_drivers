#pragma once
#include "../Config.h"
#include <driver/spi_master.h>

class SPI_Master {
public:
    // Main lifecycle hooks remain completely pristine and unaltered
    static bool init();
    static spi_device_handle_t addDevice(int cs_pin, int clock_speed_hz);
};
