# NV3047 Display & HAL Driver Engine (v1.0.0)

A high-performance, low-level Hardware Abstraction Layer (HAL) display driver custom-built for the **Elecrow CrowPanel 4.3" HMI Display (DIS06043H)** running on the **ESP32-S3** under Arduino Core 2.0.17. 

This library completely detaches low-level hardware registers, timings, and bit-blitting math from your application layer, providing a rock-solid, super-smooth foundation for high-level UI builders.

---

## 🚀 Key Hardware Features
* **Linker-Safe Harmonic Cadence Lock:** Bypasses broken core VSYNC callback bugs using internal high-resolution microsecond tracking to synchronize buffer swaps perfectly behind the physical 60Hz panel refresh cycle. Eliminates screen tearing and "lightning-bolt" fractures.
* **Stabilised 6MHz Pixel Clock:** Tuned specifically to eliminate high-frequency PCB cross-talk and VCR-style line tracking distortion on unshielded parallel copper traces.
* **High-Speed 32-Bit Clearing:** Combines dual 16-bit RGB565 pixels into dense 32-bit words to cut memory bus clear cycles cleanly in half.
* **Boundary-Clipped Blitting Primitives:** High-efficiency geometric blitters equipped with hard boundary clipping checking loops to protect system memory allocations.
* **Calibrated Touch Matrix:** Remaps raw XPT2046 SPI sensor voltages into guaranteed `0-479` X and `0-271` Y coordinates, with automatic axis swapping and hardware de-bouncing.

---

## 📁 Signature Directory Architecture
The library is organised into clean, technical subfolders to maintain modular separation:
```text
NV3047/
├── library.properties        # Arduino IDE library registration metadata
├── README.md                 # System overview documentation
└── src/
    ├── Config.h              # Central hardware pins, porches, and clock profiles
    ├── NV3047.h / .cpp       # Global hardware orchestrator lifecycle manager
    ├── NV3047_Driver.h/.cpp  # Main application proxy proxy API wrapper
    │
    ├── Core_Matrices/        # Custom double-buffered canvases & blitters
    │   ├── framebuffer.h / .cpp
    │   └── blitters.h / .cpp
    │
    ├── Bus_Layers/           # Parallel bus signal configurations & SPI masters
    │   ├── RGB.h / .cpp
    │   └── SPI_Master.h / .cpp
    │
    └── Peripherals_HAL/      # Screen power controls & inputs remapping matrices
        ├── DisplayDriver.h / .cpp
        └── TouchDriver.h / .cpp
```

---

## 🛠️ Ideal Arduino IDE Board Configurations
To guarantee successful compilation and optimal PSRAM allocation, use these exact settings in the **Tools** menu:
* **Board:** `ESP32-S3 Dev Module`
* **Flash Size:** `4MB (32Mb)`
* **Partition Scheme:** `Huge APP (3MB No OTA/1MB SPIFFS)`
* **Flash Mode:** `QIO`
* **PSRAM:** `QSPI PSRAM` (Mandatory for dual-framebuffer streaming)

---

## 💻 Clean Baseline Quick Start
This simple example initializes the entire hardware HAL stack, draws an un-fractured grey boundary frame box, tracks touch coordinate reticles, and animates a benchmark element smoothly at a locked 60Hz pace:

```cpp
#include <Arduino.h>
#include "NV3047_Driver.h"

// Instantiate the underlying hardware layer and application proxy wrapper
NV3047        hw_instance;
NV3047_Driver panel;

// Coordinate state tracking variables
int16_t sync_bar_x = 40;
int16_t sync_bar_dir = 3;
const int16_t BAR_WIDTH = 15;

void setup() {
    Serial.begin(115200);
    
    // Bind and ignite the low-level hardware abstraction layer
    panel.begin(&hw_instance);
    
    // Set baseline brightness to 80% to preserve the glass panel life
    panel.setBrightness(80);
}

void loop() {
    // Reference the rendering canvas and hardware driver primitives
    Framebuffer& canvas = hw_instance.getCanvas();
    
    uint16_t touch_x = 0;
    uint16_t touch_y = 0;
    bool is_currently_touched = panel.getTouch(touch_x, touch_y);

    // 1. Clear the hidden background drawing canvas using 32-bit block shifts
    canvas.clear(Config::COLOR_BLACK);

    // 2. Render static outer diagnostic grid
    for (int16_t offset = 0; offset < 36; offset += 6) {
        canvas.drawRect(
            offset, 
            offset, 
            Config::SCREEN_WIDTH - (offset * 2), 
            Config::SCREEN_HEIGHT - (offset * 2), 
            Config::COLOR_LIGHT_GREY
        );
    }

    // 3. Process application states
    if (is_currently_touched) {
        // Draw crosshair tracking reticle directly under touch point coordinates
        canvas.drawRect(touch_x - 15, touch_y - 15, 30, 30, Config::COLOR_RED);
        canvas.drawHLine(touch_x - 25, touch_y, 50, Config::COLOR_WHITE);
        canvas.drawVLine(touch_x, touch_y - 25, 50, Config::COLOR_WHITE);
    } else {
        // Render ultra-smooth benchmark pacing bar inside bounds box
        canvas.fillRect(sync_bar_x, 38, BAR_WIDTH, Config::SCREEN_HEIGHT - 76, Config::COLOR_WHITE);
        
        sync_bar_x += sync_bar_dir;
        if (sync_bar_x <= 38 || sync_bar_x >= (Config::SCREEN_WIDTH - BAR_WIDTH - 38)) {
            sync_bar_dir = -sync_bar_dir;
        }
    }

    // 4. Swap buffers (Natively regulates frame cadence down to the microsecond)
    canvas.swap();
}
```

---

## 📜 Licence
Created by **AirysDark**. Built strictly for high-performance embedded systems development. Feel free to use this as a modular low-level hardware engine package for any custom graphical framework libraries.
