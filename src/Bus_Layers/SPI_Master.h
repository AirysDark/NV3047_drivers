#pragma once
#include "../Config.h"

class SPI_Master {
public:
    static bool init();
    static spi_device_handle_t addDevice(int cs_pin, int clock_speed_hz);
};
