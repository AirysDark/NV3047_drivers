#pragma once

#include <stddef.h>
#include <stdint.h>

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

    size_t getBufferSizeBytes() const;
    size_t getTotalAllocatedBytes() const;
    size_t getFreePsramBytes() const;
    size_t getLargestFreePsramBlockBytes() const;

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

private:
    uint16_t* buffer_a;
    uint16_t* buffer_b;
    uint16_t* front_buffer;
    uint16_t* draw_buffer;
};
