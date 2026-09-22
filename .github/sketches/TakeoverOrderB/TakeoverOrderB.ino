#include <Arduino.h>
#include <NV3047_Driver.h>
#include <NV3047_Memory.h>

NV3047 hardware;
NV3047_Driver display;

void setup() {
    Serial.begin(115200);

    if (!display.begin(&hardware)) {
        return;
    }

    if (!display.isExternalMemoryManagerActive()) {
        return;
    }

    Framebuffer* canvas = display.getCanvas();
    if (!canvas) {
        return;
    }

    MemoryManager& memory = canvas->getMemoryManager();

    volatile size_t count = memory.getBufferCount();
    volatile size_t bytes = memory.getBufferSizeBytes();
    volatile size_t total = memory.getTotalAllocatedBytes();

    (void)count;
    (void)bytes;
    (void)total;
}

void loop() {
    delay(1000);
}
