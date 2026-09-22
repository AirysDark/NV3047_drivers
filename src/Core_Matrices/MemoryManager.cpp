#include "MemoryManager.h"

#include <esp_heap_caps.h>
#include <string.h>

MemoryManager::MemoryManager()
    : buffers{},
      front_index(0),
      draw_index(1),
      ready(false),
      external_provider(nullptr) {}

MemoryManager::~MemoryManager() {
    release();
}

bool MemoryManager::init() {
    release();

    const NV3047MemoryProviderV1* provider =
        nv3047_driver_get_memory_provider();

    if (provider) {
        const size_t dma_block_bytes =
            static_cast<size_t>(Config::SCREEN_WIDTH) *
            Config::Display::FILL_BUFFER_LINES *
            sizeof(uint16_t);

        const bool started = provider->begin(
            Config::SCREEN_WIDTH,
            Config::SCREEN_HEIGHT,
            Config::MemoryManager::BUFFER_COUNT,
            Config::MemoryManager::BUFFER_SIZE_BYTES,
            Config::MemoryManager::BUFFER_ALIGNMENT,
            Config::MemoryManager::ZERO_BUFFERS_ON_INIT,
            dma_block_bytes,
            4);

        // Once an external provider has registered, it owns the policy.
        // Never silently fall back to the local allocator after a takeover
        // failure because that would create competing memory owners.
        if (!started || !provider->is_ready()) {
            return false;
        }

        if (!provider->front_buffer() ||
            !provider->draw_buffer()) {
            provider->end();
            return false;
        }

        external_provider = provider;
        ready = true;

        provider->begin_frame();
        return true;
    }

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

    if (external_provider) {
        external_provider->end();
        external_provider = nullptr;
    }

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
    if (!ready) {
        return false;
    }

    if (external_provider) {
        return external_provider->is_ready();
    }

    return true;
}

uint16_t* MemoryManager::getFrontBuffer() const {
    if (!isReady()) {
        return nullptr;
    }

    if (external_provider) {
        return external_provider->front_buffer();
    }

    return buffers[front_index];
}

uint16_t* MemoryManager::getDrawBuffer() const {
    if (!isReady()) {
        return nullptr;
    }

    if (external_provider) {
        return external_provider->draw_buffer();
    }

    return buffers[draw_index];
}

void MemoryManager::swapBuffers() {
    if (!isReady()) return;

    if (external_provider) {
        external_provider->swap_buffers();

        // One completed display swap starts the next managed frame and gives
        // the adaptive broker a chance to rebalance idle/active workloads.
        external_provider->begin_frame();
        external_provider->service();
        return;
    }

    front_index = draw_index;
    draw_index = (draw_index + 1U) % Config::MemoryManager::BUFFER_COUNT;

    if (draw_index == front_index) {
        draw_index = (draw_index + 1U) % Config::MemoryManager::BUFFER_COUNT;
    }
}

size_t MemoryManager::getBufferCount() const {
    if (external_provider && isReady()) {
        return external_provider->buffer_count();
    }

    return Config::MemoryManager::BUFFER_COUNT;
}

size_t MemoryManager::getBufferSizeBytes() const {
    if (external_provider && isReady()) {
        return external_provider->buffer_size_bytes();
    }

    return Config::MemoryManager::BUFFER_SIZE_BYTES;
}

size_t MemoryManager::getTotalAllocatedBytes() const {
    if (!isReady()) return 0;

    if (external_provider) {
        return external_provider->total_allocated_bytes();
    }

    return Config::MemoryManager::BUFFER_SIZE_BYTES *
           Config::MemoryManager::BUFFER_COUNT;
}

size_t MemoryManager::getFreeManagedMemoryBytes() const {
    if (external_provider && isReady()) {
        return external_provider->free_managed_bytes();
    }

    return heap_caps_get_free_size(
        Config::MemoryManager::DIAGNOSTIC_CAPS);
}

size_t MemoryManager::getLargestFreeManagedMemoryBlockBytes() const {
    if (external_provider && isReady()) {
        return external_provider->
            largest_free_managed_block_bytes();
    }

    return heap_caps_get_largest_free_block(
        Config::MemoryManager::DIAGNOSTIC_CAPS);
}
