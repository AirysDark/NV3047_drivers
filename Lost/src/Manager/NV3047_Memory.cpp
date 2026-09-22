#include "NV3047_Memory.h"
#include <esp_heap_caps.h>

// --- BARE-METAL MEMORY REGISTRATION MECHANICS ---
// We remove the low-level 'esp_psram_init' calls entirely from this runtime function.
// Forcing physical initialization after the bootloader has already started execution 
// violently corrupts the hardware page tables, freezing the module into a permanent black screen loop.
// Instead, we safely utilize the dynamic capability heap manager to intercept memory.
extern "C" {
    uint32_t esp_get_free_internal_heap_size(void);
}

NV3047_Memory::NV3047_Memory() {
    displayBuffer1 = nullptr;
    displayBuffer2 = nullptr;
    touchMemoryBuffer = nullptr;
    isAllocated = false;
}

NV3047_Memory::~NV3047_Memory() {
    if (displayBuffer1) heap_caps_free(displayBuffer1);
    if (displayBuffer2) heap_caps_free(displayBuffer2);
    if (touchMemoryBuffer) heap_caps_free(touchMemoryBuffer);
}

bool NV3047_Memory::allocate() {
    if (isAllocated) return true;

    // Dynamic footprint bytes calculation pulling straight from our updated Config namespaces
    size_t fbSize = Config::MemoryAlloc::DISPLAY_BUFFER_SIZE;

    // --- AUTOMATIC RECOVERY FALLBACK ROUTINE ---
    // Instead of forcing hardware chip boots manually which bricks the chip, we query 
    // the system total. If PSRAM is not initialized via your IDE menus, we automatically
    // drop back to reclaiming high-speed internal DRAM regions using safe 8-bit constraints.
    if (heap_caps_get_total_size(MALLOC_CAP_SPIRAM) == 0) {
        displayBuffer1 = (uint16_t*) heap_caps_malloc(fbSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        displayBuffer2 = (uint16_t*) heap_caps_malloc(fbSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    } else {
        // PSRAM is safely online! Allocate the heavy video frames straight out to the 4MB space.
        displayBuffer1 = (uint16_t*) heap_caps_malloc(fbSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        displayBuffer2 = (uint16_t*) heap_caps_malloc(fbSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }

    // Safety fallback trap: If memory pools are entirely saturated, return false
    if (displayBuffer1 == nullptr || displayBuffer2 == nullptr) {
        return false;
    }

    // Pins the lightweight touch data strictly into shielded internal RAM using our config bounds
    touchMemoryBuffer = (uint8_t*) heap_caps_malloc(Config::MemoryAlloc::TOUCH_BUFFER_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

    if (touchMemoryBuffer == nullptr) {
        return false;
    }

    isAllocated = true;
    return true;
}
