#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../Config.h"

class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager();

    bool init();
    void release();

    bool isReady() const;

    uint16_t* getFrontBuffer() const;
    uint16_t* getDrawBuffer() const;
    void swapBuffers();

    size_t getBufferCount() const;
    size_t getBufferSizeBytes() const;
    size_t getTotalAllocatedBytes() const;
    size_t getFreePsramBytes() const;
    size_t getLargestFreePsramBlockBytes() const;

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

private:
    uint16_t* buffers[Config::MemoryManager::BUFFER_COUNT];
    size_t front_index;
    size_t draw_index;
};
