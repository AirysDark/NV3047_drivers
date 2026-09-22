#pragma once

#include "../Config.h"

#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDCardDriver {
public:
    SDCardDriver() = default;
    ~SDCardDriver();

    bool init();
    void end();

    bool isMounted() const { return mounted; }

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
};
