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
        ├── SDCardDriver.h / .cpp
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

## External expansion ports (future reference)

The CrowPanel exposes two external **HY2.0-4P** expansion connectors. These are documented here for future peripherals and are **not currently initialized or claimed by the NV3047 driver**.

### UART1 port

```text
Pin 1 : GND
Pin 2 : 3V3
Pin 3 : GPIO18  (RX)
Pin 4 : GPIO17  (TX)
```

Suggested future use:

```cpp
constexpr int UART1_RX = 18;
constexpr int UART1_TX = 17;
```

### GPIO_D port

```text
Pin 1 : GND
Pin 2 : 3V3
Pin 3 : GPIO19
Pin 4 : GPIO20
```

Suggested future use:

```cpp
constexpr int EXPANSION_GPIO_D1 = 19;
constexpr int EXPANSION_GPIO_D2 = 20;
```

### Important pin-conflict note

The current working driver configuration already uses some of these GPIO numbers internally:

- GPIO18 is currently the working touch chip-select pin.
- GPIO19 and GPIO20 are currently used by the working touch SPI bus.

Because of that, these expansion pins should be treated as **board-reference information only** until the hardware revision and internal routing are verified. Do not enable external devices on these pins at the same time as the current touch configuration without first resolving the GPIO conflict.

## Alternative / reference DIS06043H pin map

The following pinout has been collected as an **alternative/reference DIS06043H mapping**. It is retained for hardware comparison and future board-revision investigation.

**Important:** this is **not** the active pin map used by `driver_overhaul_v2`. The current working RGB/touch configuration in `Config.h` remains authoritative and should not be replaced with this reference map without testing the actual panel.

### Reference microSD / TF slot

```cpp
#define SD_CS   10
#define SD_CLK  12
#define SD_MOSI 11
#define SD_MISO 13
#define SD_IRQ  36
```

The SPI SD pins `CS=10`, `CLK=12`, `MOSI=11`, and `MISO=13` are the values currently used by `SDCardDriver`. The reported `SD_IRQ=36` value is **not used**, because GPIO36 is retained by the working driver as the touch interrupt input.

### Reference LCD mapping

```cpp
#define LCD_PCLK      9
#define LCD_DE        4
#define LCD_VSYNC     3
#define LCD_HSYNC     46
#define LCD_BACKLIGHT 2

#define LCD_R0 45
#define LCD_R1 42
#define LCD_R2 41
#define LCD_R3 40
#define LCD_R4 39

#define LCD_G0 0
#define LCD_G1 48
#define LCD_G2 47
#define LCD_G3 21
#define LCD_G4 14
#define LCD_G5 38

#define LCD_B0 5
#define LCD_B1 6
#define LCD_B2 7
#define LCD_B3 15
#define LCD_B4 16
```

### Reference resistive-touch mapping

```cpp
#define TOUCH_CLK  1
#define TOUCH_DIN  11
#define TOUCH_DOUT 13
#define TOUCH_CS   38
```

### Why this reference is interesting

There is a strong structural overlap between this reference map and the current working map.

```text
REFERENCE BLUE:   5, 6, 7, 15, 16
WORKING BLUE:    15, 7, 6, 5, 4
```

Four of the five blue GPIOs are shared.

The reference red bank is:

```text
45, 42, 41, 40, 39
```

Those correspond in the working map to:

```text
45 = R4
42 = PCLK
41 = VSYNC
40 = DE
39 = HSYNC
```

The reference green bank also reuses much of the working red bank:

```text
REFERENCE GREEN: 0, 48, 47, 21, 14, 38
WORKING RED:     14, 21, 47, 48, 45
```

And the reference timing pins:

```text
PCLK  = 9
HSYNC = 46
VSYNC = 3
DE    = 4
```

are GPIOs that currently sit inside the working green/blue data banks.

This overlap is documented because it may indicate a different PCB revision, a differently-labelled vendor pin map, or a systematic signal-group remapping. It may also help explain why this project requires its unusual panel colour-bank compensation.

Until the alternative mapping is verified on the actual hardware, **do not replace the working RGB/touch map in `Config.h` with it**.

## MicroSD support

The DIS06043H microSD slot is supported through the dedicated `SDCardDriver` HAL using the Arduino-ESP32 **2.0.17** SPI SD library.

The confirmed SPI wiring is configured only in `Config.h`:

```cpp
constexpr int PIN_SD_CS   = 10;
constexpr int PIN_SD_CLK  = 12;
constexpr int PIN_SD_MOSI = 11;
constexpr int PIN_SD_MISO = 13;

namespace SDCard {
    constexpr uint32_t CLOCK_HZ = 4000000;
    constexpr const char* MOUNT_POINT = "/sd";
    constexpr uint8_t MAX_OPEN_FILES = 5;
    constexpr bool FORMAT_IF_MOUNT_FAILED = false;
    constexpr bool END_SPI_ON_UNMOUNT = true;
}
```

The SD slot uses Arduino's global `SPI` object, which maps to FSPI on ESP32-S3 under core 2.0.17. The existing XPT2046 driver remains on its separate `SPI3_HOST` bus, so adding SD support does not replace the working touch wiring.

The reported `SD_IRQ = 36` value is not used by this driver. GPIO36 remains assigned to the currently working touch IRQ path.

Basic use:

```cpp
#include <SDCardDriver.h>

SDCardDriver sd;

void setup() {
    Serial.begin(115200);

    if (!sd.init()) {
        Serial.println("SD mount failed");
        return;
    }

    Serial.println(sd.cardSizeBytes());

    fs::File file = sd.open("/test.txt", FILE_WRITE);
    if (file) {
        file.println("NV3047 SD test");
        file.close();
    }
}
```

The SD card is **not automatically mounted by `NV3047::init()`**. This is intentional so the display/touch driver can still start normally with no card inserted. Mount it explicitly with `SDCardDriver::init()` when the application needs storage.

### Runtime safe-removal command

For a future UI/menu command such as **Eject SD Card** or **Prepare SD for removal**, call:

```cpp
if (sd.prepareForRemoval()) {
    // UI can now show: SAFE TO REMOVE SD CARD
}
```

The driver then reports:

```cpp
sd.isMounted();       // false
sd.isSafeToRemove();  // true
```

Before calling `prepareForRemoval()`, close every application-owned `fs::File`:

```cpp
fs::File logFile = sd.open("/log.txt", FILE_APPEND);

// ...write data...

logFile.flush();
logFile.close();

sd.prepareForRemoval();
```

`prepareForRemoval()` unmounts the SD filesystem and, with the default configuration, shuts down the SD SPI bus. The display, touch, framebuffer and backlight continue running normally.

After physically inserting a card again, call:

```cpp
sd.init();
```

to remount it during runtime.

**Important:** the driver cannot forcibly close `fs::File` objects that the application has copied into another scope. Those file handles must be flushed/closed before the eject command for removal to be genuinely safe.

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
