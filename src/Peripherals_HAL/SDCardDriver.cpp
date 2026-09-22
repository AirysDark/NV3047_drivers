#include "SDCardDriver.h"

SDCardDriver::~SDCardDriver() {
    end();
}

bool SDCardDriver::init() {
    if (mounted) {
        return true;
    }

    // Arduino-ESP32 core 2.0.17:
    // On ESP32-S3, the global SPI object uses FSPI.
    // The current touch driver uses SPI3_HOST separately, so this SD bus
    // does not reuse or disturb the touch controller bus.
    SPI.begin(
        Config::PIN_SD_CLK,
        Config::PIN_SD_MISO,
        Config::PIN_SD_MOSI,
        Config::PIN_SD_CS);

    const bool started = SD.begin(
        Config::PIN_SD_CS,
        SPI,
        Config::SDCard::CLOCK_HZ,
        Config::SDCard::MOUNT_POINT,
        Config::SDCard::MAX_OPEN_FILES,
        Config::SDCard::FORMAT_IF_MOUNT_FAILED);

    if (!started || SD.cardType() == CARD_NONE) {
        SD.end();

        if (Config::SDCard::END_SPI_ON_UNMOUNT) {
            SPI.end();
        }

        mounted = false;
        return false;
    }

    mounted = true;
    return true;
}

void SDCardDriver::end() {
    if (!mounted) {
        return;
    }

    SD.end();
    mounted = false;

    if (Config::SDCard::END_SPI_ON_UNMOUNT) {
        SPI.end();
    }
}

sdcard_type_t SDCardDriver::cardType() const {
    return mounted ? SD.cardType() : CARD_NONE;
}

uint64_t SDCardDriver::cardSizeBytes() const {
    return mounted ? SD.cardSize() : 0;
}

uint64_t SDCardDriver::totalBytes() const {
    return mounted ? SD.totalBytes() : 0;
}

uint64_t SDCardDriver::usedBytes() const {
    return mounted ? SD.usedBytes() : 0;
}

bool SDCardDriver::exists(const char* path) const {
    return mounted && path && SD.exists(path);
}

fs::File SDCardDriver::open(const char* path, const char* mode) {
    if (!mounted || !path || !mode) {
        return fs::File();
    }

    return SD.open(path, mode);
}

bool SDCardDriver::mkdir(const char* path) {
    return mounted && path && SD.mkdir(path);
}

bool SDCardDriver::remove(const char* path) {
    return mounted && path && SD.remove(path);
}

bool SDCardDriver::rename(const char* from, const char* to) {
    return mounted && from && to && SD.rename(from, to);
}

bool SDCardDriver::rmdir(const char* path) {
    return mounted && path && SD.rmdir(path);
}
