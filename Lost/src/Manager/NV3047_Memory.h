#pragma once
#include <stdint.h>
#include <stddef.h>
#include "../Config.h"

// Clear out global macro definitions from this tracking block header.
// Leaving manual macro assignments here can trick secondary sub-modules 
// into compiling with wrong memory references before your configuration
// handles the true hardware capabilities check natively!

class NV3047_Memory {
private:
    uint16_t* displayBuffer1;
    uint16_t* displayBuffer2;
    uint8_t*  touchMemoryBuffer;
    bool isAllocated;

public:
    NV3047_Memory();
    ~NV3047_Memory();

    // Core allocator: Sets up memory blocks across internal and external memory domains
    bool allocate();

    // Getters allowing your library subsystems to grab the exact allocated pointers
    uint16_t* getDisplayBuffer1() const { return displayBuffer1; }
    uint16_t* getDisplayBuffer2() const { return displayBuffer2; }
    uint8_t*  getTouchBuffer() const { return touchMemoryBuffer; }
};
