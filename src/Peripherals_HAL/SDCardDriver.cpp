#include "SDCardDriver.h"

SDCardDriver::~SDCardDriver() {
    end();
}

bool SDCardDriver::init() {
    if (mounted) {
        return true;
    }

    // Do not guess the TF chip-select. GPIO10 is PCB-confirmed TP_CS.
    if (!isConfigured()) {
        safe_to_remove = true;
        return false;
    }

    // The board is expected to share CLK/MOSI/MISO between touch and TF.
    // The current Arduino SD backend creates its own SPI ownership, so it is
    // deliberately blocked for the shared-bus configuration. Once TF_CS is
    // identified, this backend should be completed using the already-owned
    // Config::SPI_HOST_ID bus rather than starting a competing SPI controller.
    if (Config::SDCard::SHARES_TOUCH_SPI_BUS) {
        safe_to_remove = true;
        return false;
    }

    safe_to_remove = false;

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

    // Any fs::File handles owned by application code must be flushed and
    // closed before this call. This driver cannot forcibly close copies of
    // File objects held elsewhere.
    SD.end();
    mounted = false;

    // On the real board the TF bus is expected to be shared with touch, so
    // Config.h defaults this to false. Never shut down a shared bus on eject.
    if (Config::SDCard::END_SPI_ON_UNMOUNT) {
        SPI.end();
    }

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
