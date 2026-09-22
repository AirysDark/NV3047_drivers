#include "MemoryManager.h"

#include <esp_heap_caps.h>
#include <string.h>

MemoryManager::MemoryManager()
    : buffers{},
      front_index(0),
      draw_index(1),
      ready(false) {}

MemoryManager::~MemoryManager() {
    release();
}

bool MemoryManager::init() {
    release();

    for (size_t i = 0; i < Config::MemoryManager::BUFFER_COUNT; ++i) {
        buffers[i] = static_cast<uint16_t*>(heap_caps_aligned_alloc(
            Config::MemoryManager::BUFFER_ALIGNMENT,
            Config::MemoryManager::BUFFER_SIZE_BYTES,
            Config::MemoryManager::ALLOCATION_CAPS));

        if (!buffers[i]) {
            release();
            return false;
        }

        if (Config::MemoryManager::ZERO_BUFFERS_ON_INIT) {
            memset(
                buffers[i],
                0,
                Config::MemoryManager::BUFFER_SIZE_BYTES);
        }
    }

    front_index = 0;
    draw_index = 1;
    ready = true;
    return true;
}

void MemoryManager::release() {
    ready = false;

    for (size_t i = 0; i < Config::MemoryManager::BUFFER_COUNT; ++i) {
        if (buffers[i]) {
            heap_caps_free(buffers[i]);
            buffers[i] = nullptr;
        }
    }

    front_index = 0;
    draw_index = 1;
}

bool MemoryManager::isReady() const {
    return ready;
}

uint16_t* MemoryManager::getFrontBuffer() const {
    return ready ? buffers[front_index] : nullptr;
}

uint16_t* MemoryManager::getDrawBuffer() const {
    return ready ? buffers[draw_index] : nullptr;
}

void MemoryManager::swapBuffers() {
    if (!ready) return;

    front_index = draw_index;
    draw_index = (draw_index + 1U) % Config::MemoryManager::BUFFER_COUNT;

    if (draw_index == front_index) {
        draw_index = (draw_index + 1U) % Config::MemoryManager::BUFFER_COUNT;
    }
}

size_t MemoryManager::getBufferCount() const {
    return Config::MemoryManager::BUFFER_COUNT;
}

size_t MemoryManager::getBufferSizeBytes() const {
    return Config::MemoryManager::BUFFER_SIZE_BYTES;
}

size_t MemoryManager::getTotalAllocatedBytes() const {
    if (!ready) return 0;

    return Config::MemoryManager::BUFFER_SIZE_BYTES *
           Config::MemoryManager::BUFFER_COUNT;
}

size_t MemoryManager::getFreeManagedMemoryBytes() const {
    return heap_caps_get_free_size(Config::MemoryManager::DIAGNOSTIC_CAPS);
}

size_t MemoryManager::getLargestFreeManagedMemoryBlockBytes() const {
    return heap_caps_get_largest_free_block(
        Config::MemoryManager::DIAGNOSTIC_CAPS);
}
