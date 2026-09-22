# NV3047 Display & HAL Driver Engine (v2.0.0 overhaul)

A low-level display, touch, framebuffer, and hardware abstraction driver for the **Elecrow CrowPanel 4.3" DIS06043H** on the **ESP32-S3**.

This branch is intentionally built around **Arduino-ESP32 core 2.0.17**. The working electrical setup is treated as authoritative: the 6 MHz RGB pixel clock, the current RGB GPIO routing, the non-standard colour-bank mapping, and the verified XPT2046 command/byte handling are preserved.

## What changed in v2

- Added a dedicated `MemoryManager` class for the configurable framebuffer pool.
- Framebuffer allocation is now re-init safe and cannot leave dangling pointers after a partial allocation failure.
- Framebuffer and display memory-owning classes are non-copyable.
- Fixed potentially unaligned 32-bit writes in horizontal and rectangle blitters.
- Fixed touch mapping so coordinates remain inside X `0-479` and Y `0-271`.
- Replaced mean-of-three touch filtering with median-of-three filtering.
- Added raw touch diagnostic access and a `Touch-test` example.
- `NV3047_Driver::begin()` now returns `bool` and propagates initialization failures.
- Added direct drawing proxies and an explicit `present()` call to the high-level driver.
- Standardized application brightness usage to `0-100%`.
- Added framebuffer memory and frame timing diagnostics.
- Reused the DisplayDriver DMA fill buffer instead of allocating/freeing it on every fill.
- Reduced the touch SPI queue to the single slot required by polling transfers.
- Updated stale examples and package metadata.

## Single-source configuration

Normal hardware and performance tuning now happens in **`src/Config.h`**. The implementation files consume those values and should not need editing when changing normal driver settings.

The configuration file contains dedicated sections for:

- RGB GPIOs, timing, transfer alignment and signal polarity
- panel-specific colour-bank packing
- SPI clock, transfer sizing and queue depth
- XPT2046 calibration, command bytes, median sample count and settle delay
- display brightness, PWM frequency and scratch-buffer line count
- framebuffer memory count, size, alignment, allocation capabilities and zero-on-init
- presentation cadence and 32-bit clearing

Compile-time guards reject unsafe settings such as non-power-of-two framebuffer alignment, an undersized framebuffer, an invalid touch calibration range, an even median-filter sample count, or a non-16-bit RGB bus configuration.

The known-working defaults remain the same: Arduino-ESP32 **2.0.17**, 6 MHz PCLK, the verified RGB pin order and non-standard green/blue bank behaviour.

## Important colour mapping note

The working panel colour layout is **not treated as textbook RGB565**.

The physical 16-bit bus is still arranged as three 5/6/5 banks, but the verified display behaviour requires:

- red on the upper 5-bit bank,
- physical blue on the middle 6-bit bank,
- physical green on the lower 5-bit bank.

Therefore these values are intentional:

```cpp
Config::COLOR_RED   = 0xF800;
Config::COLOR_GREEN = 0x001F;
Config::COLOR_BLUE  = 0x07E0;
```

Do **not** swap GREEN and BLUE back to standard RGB565 values unless the RGB GPIO bank mapping is also reworked and tested on the actual panel.

For generated colours, use:

```cpp
uint16_t color = Config::packPanelColor(red, green, blue);
```

where `red`, `green`, and `blue` are normal 0-255 logical colour values. The helper packs them into this panel's verified physical bank order.

## Architecture

```text
NV3047_drivers/
├── library.properties
├── readme.md
├── examples/
│   ├── Color-test/
│   ├── Shape-test/
│   └── Touch-test/
└── src/
    ├── Config.h / .cpp
    ├── NV3047.h / .cpp
    ├── NV3047_Driver.h / .cpp
    │
    ├── Core_Matrices/
    │   ├── MemoryManager.h / .cpp
    │   ├── framebuffer.h / .cpp
    │   └── blitters.h / .cpp
    │
    ├── Bus_Layers/
    │   ├── RGB.h / .cpp
    │   └── SPI_Master.h / .cpp
    │
    └── Peripherals_HAL/
        ├── DisplayDriver.h / .cpp
        └── TouchDriver.h / .cpp
```

### MemoryManager

`MemoryManager` owns the framebuffer pool, but **all normal memory tuning is controlled from `Config.h`**. The implementation contains no fixed buffer count, alignment, allocation capability, or clear-on-init setting, and draw calls do not scan the buffer pool on every access.

The central configuration block is:

```cpp
namespace Config {
namespace MemoryManager {
    constexpr size_t BUFFER_COUNT = 2;
    constexpr size_t BUFFER_SIZE_BYTES =
        static_cast<size_t>(SCREEN_WIDTH) * SCREEN_HEIGHT * sizeof(uint16_t);
    constexpr size_t BUFFER_ALIGNMENT = 64;
    constexpr uint32_t ALLOCATION_CAPS =
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
    constexpr uint32_t DIAGNOSTIC_CAPS = MALLOC_CAP_SPIRAM;
    constexpr bool ZERO_BUFFERS_ON_INIT = true;
}
}
```

Change those values in `Config.h`; `MemoryManager.h/.cpp` should not need editing for normal memory-layout changes. `BUFFER_COUNT` supports 2 or more buffers and the manager rotates through the configured pool automatically.

At the default 480 x 272 RGB565 configuration:

```text
One framebuffer:  261,120 bytes
Buffer count:     2
Total allocation: 522,240 bytes
```

It exposes diagnostic information for configured buffer count, buffer size, total framebuffer allocation, free managed-memory capability space, and the largest free block.

### Framebuffer

`Framebuffer` is now responsible for rendering and presentation state rather than raw allocation ownership. It draws into the MemoryManager back buffer and swaps buffer identities after a successful panel presentation.

It also tracks:

- frame count,
- last frame interval in microseconds,
- approximate presentation FPS.

## Arduino IDE configuration

Use **Arduino-ESP32 core 2.0.17**.

Recommended board settings:

- **Board:** ESP32-S3 Dev Module
- **Flash Size:** 4MB (32Mb)
- **Partition Scheme:** Huge APP (3MB No OTA / 1MB SPIFFS)
- **Flash Mode:** QIO
- **PSRAM:** QSPI PSRAM

With the default configuration, PSRAM is required for the full-size application framebuffer pool.

## RGB timing

The known-stable hardware configuration remains:

```cpp
PCLK_FREQ_HZ = 6000000;
```

The existing tuned presentation interval of `16546 us` also remains the default because it is part of the known-working Core 2.0.17 setup.

The configuration now additionally calculates the theoretical scan period from the configured resolution and porch values:

```cpp
Config::Framebuffer::CALCULATED_SCAN_PERIOD_US
```

To experimentally pace presentation from that calculated value, set:

```cpp
Config::Framebuffer::USE_CALCULATED_SCAN_CADENCE = true;
```

The default remains `false` so the overhaul does not silently change the known-working display behaviour.

Hardware VSYNC callbacks and direct zero-copy RGB framebuffer ownership are intentionally **not** enabled in this branch. They are worthwhile experiments later, but they are higher-risk changes on Arduino-ESP32 2.0.17.

## Quick start

```cpp
#include <Arduino.h>
#include <NV3047_Driver.h>

NV3047 hardware;
NV3047_Driver display;

void setup() {
    Serial.begin(115200);

    if (!display.begin(&hardware)) {
        Serial.println("NV3047 init failed");
        while (true) delay(1000);
    }

    display.setBrightness(80);
}

void loop() {
    display.clear(Config::COLOR_BLACK);

    display.fillRect(
        40,
        40,
        100,
        60,
        Config::COLOR_RED);

    if (!display.present()) {
        Serial.println("Present failed");
    }
}
```

`fillScreen(color)` is retained for simple sketches and immediately presents the filled frame. For multi-draw frames, use `clear()`, drawing calls, then `present()`.

## Memory diagnostics

```cpp
Framebuffer& canvas = hardware.getCanvas();
MemoryManager& memory = canvas.getMemoryManager();

Serial.println(memory.getBufferCount());
Serial.println(memory.getBufferSizeBytes());
Serial.println(memory.getTotalAllocatedBytes());
Serial.println(memory.getFreeManagedMemoryBytes());
Serial.println(memory.getLargestFreeManagedMemoryBlockBytes());

Serial.println(canvas.getFrameCount());
Serial.println(canvas.getLastFrameTimeUs());
Serial.println(canvas.getApproxFPS());
```

## Touch diagnostics

The normal touch path uses the odd sample count configured in `Config::Touch::SAMPLE_COUNT` (default 3) and chooses the median sample to reject ADC spikes. Supported configured values are odd counts from 3 through 9.

The public mapped coordinates are clamped to:

```text
X: 0-479
Y: 0-271
```

For calibration work, `TouchDriver::getRawTouch()` exposes the verified raw XPT2046 values without changing the working command-byte layout. SPI transaction failures are propagated as failed reads instead of being converted into fake edge coordinates.

See `examples/Touch-test/touch-test.ino`.

## Notes for future optimization

The current default design intentionally keeps the ESP-IDF RGB driver's own framebuffer behaviour plus the configured application framebuffer pool because that is the stable Core 2.0.17 configuration.

A future experimental branch can investigate:

1. direct/zero-copy RGB framebuffer ownership,
2. hardware frame-event synchronization,
3. dirty-region presentation,
4. additional low-level geometry primitives.

Those changes should be benchmarked on the real panel rather than folded into the stable driver path blindly.

## Licence

Created by **AirysDark** for embedded display development.
