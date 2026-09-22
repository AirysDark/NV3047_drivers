#include "SPI_Master.h"

#include <Arduino.h>

bool SPI_Master::init() {
    // Keep every chip-select inactive before the shared bus starts.
    pinMode(Config::PIN_TOUCH_CS, OUTPUT);
    digitalWrite(Config::PIN_TOUCH_CS, HIGH);

    pinMode(Config::PIN_SD_CS, OUTPUT);
    digitalWrite(Config::PIN_SD_CS, HIGH);

    SPI.begin(
        Config::PIN_SHARED_SPI_SCLK,
        Config::PIN_SHARED_SPI_MISO,
        Config::PIN_SHARED_SPI_MOSI,
        -1);

    return SPI.bus() != nullptr;
}

SPIClass& SPI_Master::bus() {
    return SPI;
}
