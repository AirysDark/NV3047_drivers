#include "MemoryManager.h"
#include "../Config.h"

#include <esp_heap_caps.h>
#include <string.h>

MemoryManager::MemoryManager()
    : buffer_a(nullptr),
      buffer_b(nullptr),
      front_buffer(nullptr),
      draw_buffer(nullptr) {}

MemoryManager::~MemoryManager() {
    release();
}

bool MemoryManager::init() {
    // Re-initialisation is safe: release any previous ownership first.
    release();

    buffer_a = static_cast<uint16_t*>(heap_caps_aligned_alloc(
        64,
        Config::Framebuffer::BUFFER_SIZE_BYTES,
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));

    buffer_b = static_cast<uint16_t*>(heap_caps_aligned_alloc(
        64,
        Config::Framebuffer::BUFFER_SIZE_BYTES,
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));

    if (!buffer_a || !buffer_b) {
        release();
        return false;
    }

    memset(buffer_a, 0, Config::Framebuffer::BUFFER_SIZE_BYTES);
    memset(buffer_b, 0, Config::Framebuffer::BUFFER_SIZE_BYTES);

    front_buffer = buffer_a;
    draw_buffer = buffer_b;
    return true;
}

void MemoryManager::release() {
    if (buffer_a) {
        heap_caps_free(buffer_a);
        buffer_a = nullptr;
    }

    if (buffer_b) {
        heap_caps_free(buffer_b);
        buffer_b = nullptr;
    }

    front_buffer = nullptr;
    draw_buffer = nullptr;
}

bool MemoryManager::isReady() const {
    return buffer_a && buffer_b && front_buffer && draw_buffer;
}

uint16_t* MemoryManager::getFrontBuffer() const {
    return front_buffer;
}

uint16_t* MemoryManager::getDrawBuffer() const {
    return draw_buffer;
}

void MemoryManager::swapBuffers() {
    if (!isReady()) return;

    uint16_t* old_front = front_buffer;
    front_buffer = draw_buffer;
    draw_buffer = old_front;
}

size_t MemoryManager::getBufferSizeBytes() const {
    return Config::Framebuffer::BUFFER_SIZE_BYTES;
}

size_t MemoryManager::getTotalAllocatedBytes() const {
    return isReady() ? (Config::Framebuffer::BUFFER_SIZE_BYTES * Config::Framebuffer::NUM_BUFFERS) : 0;
}

size_t MemoryManager::getFreePsramBytes() const {
    return heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
}

size_t MemoryManager::getLargestFreePsramBlockBytes() const {
    return heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
}
