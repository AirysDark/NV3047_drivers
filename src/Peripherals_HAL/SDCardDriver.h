#pragma once

#include "../Config.h"

#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDCardDriver {
public:
    SDCardDriver() = default;
    ~SDCardDriver();

    // TF support is intentionally disabled until the board's TF chip-select
    // is confirmed. The PCB-verified shared bus is CLK=12, MOSI=11, MISO=13.
    bool init();
    void end();

    // Runtime-safe eject command for the completed TF backend.
    // Close all application-owned fs::File handles before calling this.
    bool prepareForRemoval();

    bool isConfigured() const {
        return Config::SDCard::ENABLED && Config::PIN_SD_CS >= 0;
    }

    bool isMounted() const { return mounted; }
    bool isSafeToRemove() const { return safe_to_remove; }

    sdcard_type_t cardType() const;
    uint64_t cardSizeBytes() const;
    uint64_t totalBytes() const;
    uint64_t usedBytes() const;

    bool exists(const char* path) const;
    fs::File open(const char* path, const char* mode = FILE_READ);

    bool mkdir(const char* path);
    bool remove(const char* path);
    bool rename(const char* from, const char* to);
    bool rmdir(const char* path);

    SDCardDriver(const SDCardDriver&) = delete;
    SDCardDriver& operator=(const SDCardDriver&) = delete;

private:
    bool mounted = false;
    bool safe_to_remove = true;
};
