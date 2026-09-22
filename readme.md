# NV3047 Display & HAL Driver Engine (v2.0.0 overhaul)

A low-level display, touch, framebuffer, and hardware abstraction driver for the **Elecrow CrowPanel 4.3" DIS06043H** on the **ESP32-S3**.

This branch is intentionally built around **Arduino-ESP32 core 2.0.17**. The 6 MHz RGB configuration and non-standard colour-bank mapping remain the known-working display setup. Touch, expansion, and I2S pin assignments are now based directly on the silkscreen photographed on the actual PCB.

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
- Corrected the active touch SPI pins from the actual PCB silkscreen.
- Documented PCB-verified UART1, GPIO_D and I2S pins.
- Added shared-bus SD/TF support with independent TF chip-select, runtime safe-removal, and one SPI mutex shared with XPT2046 touch.

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

The default `LEGACY_WORKING` profile is **not treated as textbook RGB565**, because hardware testing with that GPIO map required green/blue compensation:

- red on the upper 5-bit bank,
- physical blue on the middle 6-bit bank,
- physical green on the lower 5-bit bank.

For `LEGACY_WORKING`:

```cpp
Config::COLOR_RED   = 0xF800;
Config::COLOR_GREEN = 0x001F;
Config::COLOR_BLUE  = 0x07E0;
```

This does **not** prove that the panel itself has a non-standard native colour format. The compensated colours may instead be masking an incorrect GPIO/data-lane assignment. That is exactly what the opt-in `V21_MATRIX_TEST` profile is designed to test; it switches to standard RGB565 together with the proposed V2.1 lane map.

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

## Selectable RGB profiles

The driver now contains two compile-time RGB GPIO-matrix profiles in `Config.h`.

The default remains the known-working profile:

```cpp
constexpr RGBProfile ACTIVE_RGB_PROFILE =
    RGBProfile::LEGACY_WORKING;
```

Available values:

```cpp
RGBProfile::LEGACY_WORKING
RGBProfile::V21_MATRIX_TEST
```

### LEGACY_WORKING

This preserves the hardware-tested map:

```text
PCLK  = 42
DE    = 40
VSYNC = 41
HSYNC = 39

B: 15, 7, 6, 5, 4
G: 9, 46, 3, 8, 16, 1
R: 14, 21, 47, 48, 45
```

and preserves the compensated colour constants:

```cpp
RED   = 0xF800
GREEN = 0x001F
BLUE  = 0x07E0
```

### V21_MATRIX_TEST

This profile tests the proposed V2.1 GPIO-matrix assignment:

```text
PCLK  = 9
DE    = 4
VSYNC = 3
HSYNC = 46

B: 5, 6, 7, 15, 16
G: 1, 48, 47, 21, 14, 38
R: 45, 42, 41, 40, 39
```

For this profile, colour constants switch back to standard RGB565:

```cpp
RED   = 0xF800
GREEN = 0x07E0
BLUE  = 0x001F
```

The test profile deliberately keeps the proven display timing:

```text
PCLK = 6 MHz

HBP = 43
HFP = 8
HPW = 2

VBP = 12
VFP = 8
VPW = 2
```

This isolates the experiment to **GPIO-matrix routing and colour-lane order**. The unverified 9 MHz / 8-8-4 timing proposal is not enabled.

To test the V2.1 map, change only:

```cpp
constexpr RGBProfile ACTIVE_RGB_PROFILE =
    RGBProfile::V21_MATRIX_TEST;
```

Selecting `V21_MATRIX_TEST` also enables the complete V2.1 shared-peripheral startup path:

```text
GPIO0  = TP_CS
GPIO10 = SD_CS
GPIO12 = shared SCLK
GPIO11 = shared MOSI
GPIO13 = shared MISO
GPIO36 = TP_IRQ
```

The TF card is automatically mounted during `NV3047::init()`. If a card is present and readable, `isSDMounted()` is true when initialization returns. If no card is inserted or mounting fails, LCD and touch initialization still succeeds; the application can insert a card later and call `getSDCard()->init()`.

To revert immediately:

```cpp
constexpr RGBProfile ACTIVE_RGB_PROFILE =
    RGBProfile::LEGACY_WORKING;
```

`examples/Color-test/color-test.ino` prints the selected profile and the active PCLK/DE/VSYNC/HSYNC pins at startup.

### Expansion conflict under V21_MATRIX_TEST

The proposed V2.1 RGB map uses:

```text
GPIO38 = LCD G5
```

but this physical board also exposes GPIO38 on the `GPIO_D` expansion connector.

Therefore `Config::Expansion::GPIO_D0_AVAILABLE` becomes `false` while `V21_MATRIX_TEST` is selected. Do not drive an external device on GPIO38 during that RGB test.

The V2.1 RGB map remains a **test hypothesis**, not a verified production mapping. A correct picture with standard RGB565 colours would be strong evidence for it; a blank, unstable, or mis-coloured image would not justify changing the known-working default.

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

For hardware revision **V2.1**, the photographed PCB silkscreen and Elecrow's published V2.1 definition agree on the XPT2046 interface:

```text
GPIO0  = TP_CS
GPIO12 = TP_CLK
GPIO11 = TP_DIN / MOSI
GPIO13 = TP_OUT / MISO
GPIO36 = TP_IRQ
```

The active configuration is therefore:

```cpp
constexpr int PIN_SHARED_SPI_SCLK = 12;
constexpr int PIN_SHARED_SPI_MOSI = 11;
constexpr int PIN_SHARED_SPI_MISO = 13;

constexpr int PIN_TOUCH_CS  = 0;
constexpr int PIN_TOUCH_IRQ = 36;
```

GPIO0 is an ESP32-S3 boot-strapping pin, so the driver keeps TP_CS inactive/high whenever touch is not being addressed. The board hardware is designed around this connection.

The normal touch path uses the odd sample count configured in `Config::Touch::SAMPLE_COUNT` (default 3) and chooses the median sample to reject ADC spikes.

The public mapped coordinates remain:

```text
X: 0-479
Y: 0-271
```

The existing XPT2046 command bytes and byte decoding are retained. Touch now performs its transactions through the same Arduino `SPIClass` instance used by the TF card, using `beginTransaction()` / `endTransaction()` so shared-bus access is serialized.

See `examples/Touch-test/touch-test.ino`.

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

These pins no longer conflict with the corrected active touch mapping.

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

The NV3047 driver does not yet initialize the audio path; these are reserved for future speaker/I2S support.

## Shared XPT2046 + TF SPI bus

Touch and the microSD/TF slot share:

```text
SCLK = GPIO12
MOSI = GPIO11
MISO = GPIO13
```

but have independent chip selects:

```text
TP_CS = GPIO0
SD_CS = GPIO10
```

The active SD configuration is:

```cpp
constexpr int PIN_SD_CS   = 10;
constexpr int PIN_SD_CLK  = PIN_SHARED_SPI_SCLK;
constexpr int PIN_SD_MOSI = PIN_SHARED_SPI_MOSI;
constexpr int PIN_SD_MISO = PIN_SHARED_SPI_MISO;

namespace Config {
namespace SDCard {
    constexpr bool ENABLED = true;
    constexpr bool SHARES_TOUCH_SPI_BUS = true;
    constexpr uint32_t CLOCK_HZ = 4000000;
    constexpr bool END_SPI_ON_UNMOUNT = false;
}
}
```

### Shared-bus implementation

`SPI_Master` starts one Arduino `SPI` object on GPIO12/11/13. Both `TouchDriver` and `SDCardDriver` use that exact same object.

This is deliberate. Arduino-ESP32 core 2.0.17's SD implementation performs its transfers using `SPIClass::beginTransaction()` and `endTransaction()`. The touch driver now does the same, so the two devices use one bus mutex rather than two independent SPI controllers fighting over the same physical wires.

Both chip-select pins are held high when inactive.

### Runtime SD insertion/removal

The display and touch do not require an SD card to be installed.

With `V21_MATRIX_TEST`, the driver automatically attempts the initial TF mount. The mount result is available through:

```cpp
display.isSDMounted();
display.getSDCard();
```

A missing or unreadable card is deliberately non-fatal so an SD fault cannot prevent the display/touch system from starting.

For a manual mount or remount:

```cpp
SDCardDriver* sd = display.getSDCard();

if (sd && sd->init()) {
    // SD mounted.
}
```

Before physical removal, close application-owned files and issue:

```cpp
logFile.flush();
logFile.close();

if (sd && sd->prepareForRemoval()) {
    // SAFE TO REMOVE SD CARD
}
```

Then:

```cpp
sd->isMounted();       // false
sd->isSafeToRemove();  // true
```

`SD.end()` unmounts the filesystem, but the shared SPI controller remains running because touch still needs it.

After inserting a card again, call `sd->init()`.

## Origin of the V2.1 RGB test profile

A suggested V2.1 pin block was collected with this LCD mapping:

```cpp
#define LCD_PCLK      9
#define LCD_DE        4
#define LCD_VSYNC     3
#define LCD_HSYNC     46

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

This mapping is now retained as the opt-in `V21_MATRIX_TEST` profile. It is still **unverified** and is not the default.

The earlier version of this reference assigned `LCD_G0 = GPIO0`, which conflicted with the PCB-verified `TP_CS = GPIO0`. The revised test profile therefore uses `LCD_G0 = GPIO1`, matching the newer V2.1 proposal.

The known-working `LEGACY_WORKING` profile continues to use:

```text
DE    = 40
VSYNC = 41
HSYNC = 39
PCLK  = 42
BL    = 2
```

The working RGB data assignments in this project deliberately use a non-standard software bank/bit order because that is how the physical panel was made to produce the correct colours during hardware testing. Do not replace them from the Google reference merely to make the labels look textbook-correct.

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
