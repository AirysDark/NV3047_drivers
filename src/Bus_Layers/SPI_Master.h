#pragma once

#include "../Config.h"
#include <SPI.h>

class SPI_Master {
public:
    // Initializes the single shared peripheral SPI bus used by touch + TF.
    static bool init();

    // Returns the same Arduino SPI bus instance to every shared-bus peripheral.
    static SPIClass& bus();
};
