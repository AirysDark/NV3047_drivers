# NV3047 Display & HAL Driver Engine (v3.0.0 performance overhaul)

A low-level display, touch, framebuffer, and hardware abstraction driver for the **Elecrow CrowPanel 4.3" DIS06043H** on the **ESP32-S3**.

This branch is intentionally built around **Arduino-ESP32 core 2.0.17**. V3 is branched from the verified `driver_overhaul_v2` hardware baseline and focuses on rendering hot-path efficiency. The working RGB colour path, XPT2046 transport, touch orientation/calibration, and external-memory-provider ABI are intentionally preserved.

## What changed in v3

V3 keeps the verified V2 hardware behaviour and targets the load-dependent rendering overhead found by staged real-panel benchmarks.

- Added a frame-local cached draw-buffer pointer inside `Framebuffer`.
- Drawing primitives no longer call `MemoryManager::getDrawBuffer()` for every object/primitive.
- External-memory mode therefore avoids repeated `provider->is_ready()` and `provider->draw_buffer()` crossings during normal drawing.
- The draw pointer is resolved during initialization and refreshed once after each successful buffer-role swap.
- `Framebuffer::getDrawBuffer()` now exposes the already-cached current draw pointer.
- Added optimized `memset()` fast paths for full-screen black (`0x0000`) and white (`0xFFFF`) clears.
- Kept the existing 32-bit clear path for all other colours.
- Preserved the existing full-frame `esp_lcd_panel_draw_bitmap()` presentation path for controlled comparison against V2.
- Preserved external-provider `swap_buffers()`, `begin_frame()`, and `service()` semantics; V3 does not alter the memory-manager contract.
- Updated CI to compile `driver_overhaul_v3` in driver-only mode and both external-memory include orders.

### Why the draw-buffer cache matters

V2 resolves the draw buffer separately for each framebuffer drawing primitive. In local mode this is inexpensive, but with an external memory provider each lookup can cross the provider ABI through readiness and draw-buffer callbacks. Under heavy object counts that lookup count scales with the number of drawing calls.

V3 changes the hot path conceptually from:

```text
fillRect  -> getDrawBuffer -> provider checks
drawRect  -> getDrawBuffer -> provider checks
drawHLine -> getDrawBuffer -> provider checks
drawVLine -> getDrawBuffer -> provider checks
... repeated for every object
```

to:

```text
start/init frame -> resolve draw buffer once
all drawing      -> use cached pointer directly
present          -> swap buffer roles
next frame       -> resolve new draw buffer once
```

This change is deliberately isolated from touch, colour packing, RGB timing, and the external memory manager implementation.

## What changed in v2

- Added a dedicated `MemoryManager` class for the configurable framebuffer pool.
- Framebuffer allocation is now re-init safe and cannot leave dangling pointers after a partial allocation failure.
- Framebuffer and display memory-owning classes are non-copyable.
- Fixed potentially unaligned 32-bit writes in horizontal and rectangle blitters.
- Fixed touch mapping so the physically swapped XPT2046 axes are scaled directly into the correct display dimensions with verified forward directions, clamped inside X `0-479` and Y `0-271`.
- Replaced mean-of-three touch filtering with median-of-three filtering.
- Added raw touch diagnostic access and a `Touch-test` example.
- `NV3047_Driver::begin()` now returns `bool` and propagates initialization failures.
- Added direct drawing proxies and an explicit `present()` call to the high-level driver.
- Standardized application brightness usage to `0-100%`.
- Added framebuffer memory and frame timing diagnostics.
- Reused the DisplayDriver DMA fill buffer instead of allocating/freeing it on every fill.
- Repaired the XPT2046 transport using the physically verified hardware bus and corrected receive-byte alignment.
- Removed the selectable hardware-profile system so there is one unambiguous active pin map.
- Updated stale examples and package metadata.
- Documented PCB UART1, GPIO_D and I2S pin conflicts against the fixed active map.
- Kept the suspected V2.1 RGB/touch/TF mapping as documentation only for later hardware investigation.

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

The fixed Legacy Working baseline is **not treated as textbook RGB565**, because hardware testing with that GPIO map required green/blue compensation:

- red on the upper 5-bit bank,
- physical blue on the middle 6-bit bank,
- physical green on the lower 5-bit bank.

For the active fixed configuration:

```cpp
Config::COLOR_RED   = 0xF800;
Config::COLOR_GREEN = 0x001F;
Config::COLOR_BLUE  = 0x07E0;
```

This does **not** prove that the panel itself has a non-standard native colour format. The compensated colours may instead be masking an incorrect GPIO/data-lane assignment. The suspected V2.1 mapping documented below is retained specifically as a future hardware-check reference, but it is not an active code path.

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

`Framebuffer` is responsible for rendering and presentation state rather than raw allocation ownership. In V3 it caches the active draw-buffer pointer for the lifetime of each frame, so drawing primitives operate directly on that pointer. After a successful presentation and buffer-role swap, the cache is refreshed once for the next frame.

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

## Current fixed hardware configuration

The driver no longer contains selectable hardware profiles. The active code directly uses the established **Legacy Working baseline**.

RGB pins:

```text
PCLK  = 42
DE    = 40
VSYNC = 41
HSYNC = 39
BL    = 2

B: 15, 7, 6, 5, 4
G: 9, 46, 3, 8, 16, 1
R: 14, 21, 47, 48, 45
```

Touch pins:

```text
SCLK   = GPIO12
MOSI   = GPIO11
MISO   = GPIO13
TP_CS  = GPIO0
TP_IRQ = GPIO36
TF_CS  = GPIO10
```

Touch uses the ESP-IDF SPI device path on `SPI2_HOST` at **2.5 MHz**, mode 0, queue depth 7, with hardware-controlled touch chip select. GPIO10 is driven HIGH before the touch bus starts so the TF/SD device remains deselected.

The fixed colour constants remain:

```cpp
RED   = 0xF800
GREEN = 0x001F
BLUE  = 0x07E0
```

SD/TF is currently disabled in the fixed configuration.

## Suspected V2.1 mapping — documentation only

The following mapping was previously carried in the code as `V21_MATRIX_TEST`. It is now **documentation only**. There is no compile-time switch, runtime selector, or alternate initialization path for it.

It remains useful because it may represent the actual V2.1 board routing, but it has not been accepted as the production mapping for this driver.

Suspected RGB wiring:

```text
PCLK  = 9
DE    = 4
VSYNC = 3
HSYNC = 46
BL    = 2

B: 5, 6, 7, 15, 16
G: 1, 48, 47, 21, 14, 38
R: 45, 42, 41, 40, 39
```

Suspected touch/TF wiring:

```text
GPIO12 = TP_CLK / shared SCLK
GPIO11 = TP_DIN / shared MOSI
GPIO13 = TP_OUT / shared MISO
GPIO0  = TP_CS
GPIO36 = TP_IRQ
GPIO10 = TF/SD_CS
```

The suspected V2.1 colour interpretation is standard RGB565:

```text
RED   = 0xF800
GREEN = 0x07E0
BLUE  = 0x001F
```

During the earlier experiment this mapping deliberately retained the proven 6 MHz display timing:

```text
PCLK = 6 MHz

HBP = 43
HFP = 8
HPW = 2

VBP = 12
VFP = 8
VPW = 2
```

An earlier draft of the V2.1 reference placed LCD G0 on GPIO0, which conflicts with the suspected TP_CS connection on GPIO0. The later experimental code used LCD G0 on GPIO1 instead. That change is also unverified.

The suspected V2.1 RGB map consumes GPIO38 as LCD G5, which would conflict with the board's GPIO_D0 expansion pin. None of these V2.1 assignments are active in the current driver.

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

The active XPT2046 path has now been matched to direct hardware testing on this exact board:

```text
GPIO12 = TP_CLK / SCLK
GPIO11 = TP_DIN / MOSI
GPIO13 = TP_OUT / MISO
GPIO0  = TP_CS
GPIO36 = TP_IRQ
GPIO10 = TF/SD_CS
```

Transport settings:

```text
SPI host = SPI2_HOST
Clock    = 2.5 MHz
Mode     = 0
Queue    = 7
Flags    = 0
```

Before SPI initialization the driver drives GPIO10 HIGH to keep the TF/SD device deselected.

The baseline XPT2046 commands are:

```text
X = 0x90
Y = 0xD0
```

The 24-bit response is decoded from RX bytes 1 and 2:

```cpp
uint16_t raw =
    ((((uint16_t)rx_data[1] << 8) |
       ((uint16_t)rx_data[2])) >> 3) & 0x0FFF;
```

Direct hardware testing also confirmed that `0x94 / 0xD4` return live ADC data, but the driver uses `0x90 / 0xD0` as the clean baseline.

Calibration remains independent of the transport repair:

```text
RAW_X_MIN = 300
RAW_X_MAX = 3850
RAW_Y_MIN = 250
RAW_Y_MAX = 3750
```

### Touch orientation mapping

Real-panel testing confirmed that the XPT2046 raw channels are rotated relative to the 480 x 272 display axes. The driver therefore keeps each raw calibration range with its original ADC channel, but maps that channel into the opposite display dimension:

```text
raw Y + RAW_Y calibration -> display X (0..479)
raw X + RAW_X calibration -> display Y (0..271)
```

Real-panel testing confirmed that no destination-axis inversion is required after the raw-axis swap:

```text
screen X = mapped raw Y
screen Y = mapped raw X
```

The expected physical movement is:

```text
finger RIGHT -> detected X increases
finger LEFT  -> detected X decreases
finger DOWN  -> detected Y increases
finger UP    -> detected Y decreases
```

Expected corner mapping is approximately:

```text
top-left     -> X=0   Y=0
top-right    -> X=479 Y=0
bottom-left  -> X=0   Y=271
bottom-right -> X=479 Y=271
```

Do not swap the calibration constants themselves. `RAW_Y_MIN/MAX` remain attached to the raw Y channel even though that channel drives display X, and `RAW_X_MIN/MAX` remain attached to the raw X channel even though that channel drives display Y.

The axis swap and forward directions are now verified on the real panel. Future calibration work should adjust only the raw min/max ranges if edge accuracy needs tuning; do not alter the verified SPI transport or reintroduce destination-axis inversion.

GPIO36 IRQ is active LOW and is independent of the coordinate data path.

## PCB-verified external expansion ports

### UART1 connector

```text
GPIO18 = RX1
GPIO17 = TX1
3V3
GND
```

Configured as:

```cpp
Config::Expansion::UART1_RX = 18;
Config::Expansion::UART1_TX = 17;
```

### GPIO_D connector

```text
GPIO38
GPIO37
3V3
GND
```

Configured as:

```cpp
Config::Expansion::GPIO_D0 = 38;
Config::Expansion::GPIO_D1 = 37;
```

The repaired touch path does not consume GPIO18 or GPIO17, so both UART1 pins remain available to applications.

## PCB-verified I2S pins

The board silkscreen identifies:

```text
GPIO19 = I2S_LRCLK
GPIO35 = I2S_BCLK
GPIO20 = I2S_SDIN
```

Recorded in `Config.h` as:

```cpp
Config::I2S::LRCLK = 19;
Config::I2S::BCLK  = 35;
Config::I2S::SDIN  = 20;
```

The NV3047 driver does not yet initialize the audio path. The repaired touch path does not consume GPIO19, GPIO20, or GPIO35, so the documented I2S pins remain available to applications.

## SD/TF status

The TF interface is still disabled for mounting during touch recovery, but its physical shared-bus pins are recorded:

```cpp
Config::SDCard::ENABLED = false;

Config::PIN_SD_CS   = 10;
Config::PIN_SD_CLK  = 12;
Config::PIN_SD_MOSI = 11;
Config::PIN_SD_MISO = 13;
```

GPIO10 is forced HIGH before touch SPI initialization so the TF device stays deselected and cannot contend for GPIO13 MISO.

## Optional NV3047_memorymanager takeover

`NV3047_drivers` can now detect the separate `NV3047_memorymanager` library at runtime without making it a hard dependency.

Normal driver-only sketches are unchanged:

```cpp
#include <NV3047_Driver.h>
```

The driver's local `Core_Matrices/MemoryManager` owns the framebuffer pool.

To enable the external automatic manager, add its normal umbrella include to the sketch:

```cpp
#include <NV3047_Memory.h>
#include <NV3047_Driver.h>
```

No extra registration call is required. `NV3047_Memory.h` registers a versioned memory-provider ABI before Arduino `setup()`. During framebuffer initialization, the driver detects that provider and delegates ownership to `NV3047_memorymanager::AutoMemory`.

When takeover is active, the external library owns:

- the front/back RGB565 framebuffers,
- framebuffer role swapping,
- framebuffer memory diagnostics,
- per-frame scratch reset/service hooks,
- the display driver's persistent DMA fill buffer.

The local driver allocator is bypassed. If an external provider has registered but cannot initialize, driver initialization fails cleanly instead of silently creating a second allocator.

Runtime status is available through:

```cpp
display.isExternalMemoryManagerActive();
```

or through the canvas:

```cpp
display.getCanvas()->
    getMemoryManager().
    isExternalProviderActive();
```

The integration remains compatible with Arduino-ESP32 core **2.0.17** and the driver still works independently when the external library is absent.

## Notes for future optimization

The current default design intentionally keeps the ESP-IDF RGB driver's own framebuffer behaviour plus the configured application framebuffer pool because that is the stable Core 2.0.17 configuration.

Further V3 experiments can investigate:

1. profiling `esp_lcd_panel_draw_bitmap()` separately from rendering,
2. direct/zero-copy RGB framebuffer ownership,
3. hardware frame-event synchronization,
4. dirty-region presentation,
5. whether provider `service()` can safely be deferred or throttled without changing the external manager contract,
6. additional low-level geometry primitives.

Those changes should be benchmarked on the real panel rather than folded into the stable driver path blindly.

## Licence

Created by **AirysDark** for embedded display development.
