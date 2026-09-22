#include "SDCardDriver.h"

#include "../Bus_Layers/SPI_Master.h"

SDCardDriver::~SDCardDriver() {
    end();
}

bool SDCardDriver::init() {
    if (mounted) {
        return true;
    }

    if (!isConfigured()) {
        safe_to_remove = true;
        return false;
    }

    // Ensure touch + TF are using the exact same SPIClass instance.
    // Core 2.0.17's SD implementation calls beginTransaction()/endTransaction()
    // on this SPI object, so it shares the same bus mutex as TouchDriver.
    if (!SPI_Master::init()) {
        safe_to_remove = true;
        return false;
    }

    safe_to_remove = false;

    SPIClass& shared_bus = SPI_Master::bus();

    const bool started = SD.begin(
        Config::PIN_SD_CS,
        shared_bus,
        Config::SDCard::CLOCK_HZ,
        Config::SDCard::MOUNT_POINT,
        Config::SDCard::MAX_OPEN_FILES,
        Config::SDCard::FORMAT_IF_MOUNT_FAILED);

    if (!started || SD.cardType() == CARD_NONE) {
        SD.end();
        mounted = false;
        safe_to_remove = true;
        return false;
    }

    mounted = true;
    safe_to_remove = false;
    return true;
}

void SDCardDriver::end() {
    prepareForRemoval();
}

bool SDCardDriver::prepareForRemoval() {
    if (!mounted) {
        safe_to_remove = true;
        return true;
    }

    // Application-owned files must be flushed/closed before this call.
    SD.end();
    mounted = false;

    // The SPI bus remains running because XPT2046 touch shares it.
    safe_to_remove = true;
    return true;
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

fs::File SDCardDriver::open(
    const char* path,
    const char* mode) {

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

bool SDCardDriver::rename(
    const char* from,
    const char* to) {

    return mounted && from && to &&
           SD.rename(from, to);
}

bool SDCardDriver::rmdir(const char* path) {
    return mounted && path && SD.rmdir(path);
}
