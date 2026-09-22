#pragma once

#include <stddef.h>
#include <stdint.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_heap_caps.h>

namespace Config {
    constexpr uint16_t SCREEN_WIDTH = 480;
    constexpr uint16_t SCREEN_HEIGHT = 272;

    // RGB bus pins. This is the verified working physical routing for this panel.
    constexpr int PIN_RGB_B0 = 15;
    constexpr int PIN_RGB_B1 = 7;
    constexpr int PIN_RGB_B2 = 6;
    constexpr int PIN_RGB_B3 = 5;
    constexpr int PIN_RGB_B4 = 4;

    constexpr int PIN_RGB_G0 = 9;
    constexpr int PIN_RGB_G1 = 46;
    constexpr int PIN_RGB_G2 = 3;
    constexpr int PIN_RGB_G3 = 8;
    constexpr int PIN_RGB_G4 = 16;
    constexpr int PIN_RGB_G5 = 1;

    constexpr int PIN_RGB_R0 = 14;
    constexpr int PIN_RGB_R1 = 21;
    constexpr int PIN_RGB_R2 = 47;
    constexpr int PIN_RGB_R3 = 48;
    constexpr int PIN_RGB_R4 = 45;

    constexpr int PIN_RGB_DE    = 40;
    constexpr int PIN_RGB_VSYNC = 41;
    constexpr int PIN_RGB_HSYNC = 39;
    constexpr int PIN_RGB_PCLK  = 42;

    constexpr int PIN_BACKLIGHT = 2;

    // XPT2046 touch wiring verified for this project.
    constexpr spi_host_device_t SPI_HOST_ID = SPI3_HOST;
    constexpr int PIN_SPI_SCLK  = 20;
    constexpr int PIN_SPI_MOSI  = 19;
    constexpr int PIN_SPI_MISO  = -1;
    constexpr int PIN_TOUCH_CS  = 18;
    constexpr int PIN_TOUCH_IRQ = 36;

    // Known-stable electrical timing. Keep 6 MHz unless re-validating the panel.
    constexpr int PCLK_FREQ_HZ = 6000000;
    constexpr uint16_t HSYNC_BACK_PORCH  = 43;
    constexpr uint16_t HSYNC_FRONT_PORCH = 8;
    constexpr uint16_t HSYNC_PULSE_WIDTH = 2;
    constexpr uint16_t VSYNC_BACK_PORCH  = 12;
    constexpr uint16_t VSYNC_FRONT_PORCH = 8;
    constexpr uint16_t VSYNC_PULSE_WIDTH = 2;

    // IMPORTANT: The panel's verified working colour-bank behaviour is non-standard.
    // Red uses the upper 5-bit bank, physical blue uses the middle 6-bit bank,
    // and physical green uses the lower 5-bit bank. Do not "correct" GREEN/BLUE
    // to textbook RGB565 values without re-mapping and re-testing the RGB bus.
    constexpr uint16_t packPanelColor(uint8_t red, uint8_t green, uint8_t blue) {
        return static_cast<uint16_t>(
            (static_cast<uint16_t>(red & 0xF8U) << 8) |
            (static_cast<uint16_t>(blue & 0xFCU) << 3) |
            (static_cast<uint16_t>(green) >> 3));
    }

    constexpr uint16_t COLOR_RED         = 0xF800;
    constexpr uint16_t COLOR_GREEN       = 0x001F;
    constexpr uint16_t COLOR_BLUE        = 0x07E0;
    constexpr uint16_t COLOR_WHITE       = 0xFFFF;
    constexpr uint16_t COLOR_BLACK       = 0x0000;
    constexpr uint16_t COLOR_LIGHT_GREY  = 0xF7DE;
    constexpr uint16_t COLOR_SIDE_WHITE  = 0xFA3F;

    // ============================================================
    // MEMORY MANAGER CONFIGURATION
    // ============================================================
    // Change framebuffer memory behaviour here only. MemoryManager.h/.cpp
    // consume these settings and should not need editing for normal tuning.
    namespace MemoryManager {
        // Minimum supported value is 2. Values above 2 create a rotating
        // framebuffer pool without requiring changes to MemoryManager.cpp.
        constexpr size_t BUFFER_COUNT = 2;

        // One full 480x272 RGB565 frame by default.
        constexpr size_t BUFFER_SIZE_BYTES =
            static_cast<size_t>(SCREEN_WIDTH) *
            static_cast<size_t>(SCREEN_HEIGHT) *
            sizeof(uint16_t);

        // Keep buffers aligned for efficient 32-bit writes and RGB/PSRAM access.
        constexpr size_t BUFFER_ALIGNMENT = 64;

        // Allocation target. Default is external PSRAM with byte access.
        // This can be changed here if a future memory layout requires it.
        constexpr uint32_t ALLOCATION_CAPS =
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;

        // Capability bucket used by the diagnostic free/largest-block queries.
        constexpr uint32_t DIAGNOSTIC_CAPS = MALLOC_CAP_SPIRAM;

        // Clear every allocated framebuffer to black during init.
        constexpr bool ZERO_BUFFERS_ON_INIT = true;

        // Keep compile-time protection close to the setting being changed.
        static_assert(BUFFER_COUNT >= 2, "NV3047 MemoryManager requires at least 2 framebuffers");
        static_assert(BUFFER_ALIGNMENT >= 4, "Framebuffer alignment must be at least 4 bytes");
    }

    namespace Framebuffer {
        constexpr bool USE_32BIT_CLEAR = true;

        // The theoretical scan period is useful for diagnostics. The default presentation
        // cadence remains the hardware-tested value used by the working 2.0.17 build.
        constexpr uint32_t PANEL_TOTAL_WIDTH =
            SCREEN_WIDTH + HSYNC_BACK_PORCH + HSYNC_FRONT_PORCH + HSYNC_PULSE_WIDTH;
        constexpr uint32_t PANEL_TOTAL_HEIGHT =
            SCREEN_HEIGHT + VSYNC_BACK_PORCH + VSYNC_FRONT_PORCH + VSYNC_PULSE_WIDTH;
        constexpr uint32_t CALCULATED_SCAN_PERIOD_US =
            static_cast<uint32_t>(
                (static_cast<uint64_t>(PANEL_TOTAL_WIDTH) * PANEL_TOTAL_HEIGHT * 1000000ULL) /
                PCLK_FREQ_HZ);

        constexpr uint32_t TUNED_PRESENT_INTERVAL_US = 16546;
        constexpr bool USE_CALCULATED_SCAN_CADENCE = false;
        constexpr uint32_t FRAME_CADENCE_US =
            USE_CALCULATED_SCAN_CADENCE ? CALCULATED_SCAN_PERIOD_US : TUNED_PRESENT_INTERVAL_US;
    }
}
