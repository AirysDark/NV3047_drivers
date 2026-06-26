#include "nv3047_driver.h"
#include "DDriver.h"
#include "TouchDriver.h"
#include <Arduino.h>

// ======================================================
// CORE INIT (Grafts your true 1-month-old setup logic)
// ======================================================

void nv3047_driver_init(void)
{
    Serial.begin(115200);
    delay(200);
    Serial.println("[NV3047 Library] Spinning up native display engine...");

    // 1. Fire up the true 480x272 parallel RGB tracking configuration timings
    Display.begin();

    // 2. Flash the red, green, and blue blocks to break the panel out of the black state
    Display.testPattern();

    // 3. Register the display to the LVGL canvas engine
    Display.beginLVGL();

    // 4. Spin up the shared SPI touchscreen bus lines
    Touch.begin();

    Serial.println("[NV3047 Library] Subsystems initialized and locked.");
}

// ======================================================
// TICK LOOP (Ensures touch polls cleanly in the background)
// ======================================================

void nv3047_driver_tick(void)
{
    // Asynchronously update finger touch coordinate inputs
    Touch.update();

    // Let the background LVGL memory buffers refresh the screen visuals
    lv_timer_handler();
}

// ======================================================
// DRAW PIPELINE
// ======================================================

void nv3047_driver_draw(void)
{
    // Keeping this hook active if you call high level canvas refreshes elsewhere
}

// ======================================================
// GRAPHICS BACKEND FALLBACKS
// ======================================================

void nv3047_driver_fill(uint16_t color)
{
    if (Display.gfx) {
        Display.gfx->fillScreen(color);
    }
}

void nv3047_driver_test_pattern(void)
{
    Display.testPattern();
}
