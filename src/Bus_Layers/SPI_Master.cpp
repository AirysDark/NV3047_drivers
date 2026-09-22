#include "SPI_Master.h"

#include <Arduino.h>

bool SPI_Master::init() {
    // Keep the active profile's touch chip-select inactive before SPI starts.
    pinMode(Config::PIN_TOUCH_CS, OUTPUT);
    digitalWrite(Config::PIN_TOUCH_CS, HIGH);

    // TF exists only in the V2.1 profile. Do not touch GPIO10 in legacy mode.
    if (Config::SDCard::ENABLED && Config::PIN_SD_CS >= 0) {
        pinMode(Config::PIN_SD_CS, OUTPUT);
        digitalWrite(Config::PIN_SD_CS, HIGH);
    }

    SPI.begin(
        Config::PIN_SPI_SCLK,
        Config::PIN_SPI_MISO,
        Config::PIN_SPI_MOSI,
        -1);

    return SPI.bus() != nullptr;
}

SPIClass& SPI_Master::bus() {
    return SPI;
}
