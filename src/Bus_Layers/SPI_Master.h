#pragma once

#include "../Config.h"
#include <SPI.h>

class SPI_Master {
public:
    // Initializes the active profile's touch SPI bus. V2.1 shares it with TF.
    static bool init();

    // Returns the active Arduino SPI bus instance.
    static SPIClass& bus();
};
