#pragma once

#include <stddef.h>
#include <stdint.h>

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_heap_caps.h>

namespace Config {
    constexpr uint16_t SCREEN_WIDTH = 480;
    constexpr uint16_t SCREEN_HEIGHT = 272;

    // ============================================================
    // RGB PANEL PINS
    // ============================================================
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

    // ============================================================
    // TOUCH PINS
    // ============================================================
    constexpr spi_host_device_t SPI_HOST_ID = SPI3_HOST;
    constexpr int PIN_SPI_SCLK  = 20;
    constexpr int PIN_SPI_MOSI  = 19;
    constexpr int PIN_SPI_MISO  = -1;
    constexpr int PIN_TOUCH_CS  = 18;
    constexpr int PIN_TOUCH_IRQ = 36;

    // ============================================================
    // RGB PANEL TIMING
    // ============================================================
    // Known-stable electrical timing. Keep 6 MHz unless re-validating the panel.
    constexpr int PCLK_FREQ_HZ = 6000000;
    constexpr uint16_t HSYNC_BACK_PORCH  = 43;
    constexpr uint16_t HSYNC_FRONT_PORCH = 8;
    constexpr uint16_t HSYNC_PULSE_WIDTH = 2;
    constexpr uint16_t VSYNC_BACK_PORCH  = 12;
    constexpr uint16_t VSYNC_FRONT_PORCH = 8;
    constexpr uint16_t VSYNC_PULSE_WIDTH = 2;

    namespace RGB {
        constexpr size_t DATA_WIDTH = 16;
        constexpr size_t PSRAM_TRANSFER_ALIGNMENT = 64;
        constexpr size_t SRAM_TRANSFER_ALIGNMENT = 8;

        constexpr bool FRAMEBUFFER_IN_PSRAM = true;

        // Verified stable electrical properties for this panel.
        constexpr bool PCLK_ACTIVE_NEG = true;
        constexpr bool HSYNC_IDLE_LOW = false;
        constexpr bool VSYNC_IDLE_LOW = false;
        constexpr bool DE_IDLE_HIGH = false;
        constexpr bool PCLK_IDLE_HIGH = false;
    }

    // ============================================================
    // PANEL COLOUR MAPPING
    // ============================================================
    // IMPORTANT: The panel's verified working colour-bank behaviour is non-standard.
    // Red uses the upper 5-bit bank, physical blue uses the middle 6-bit bank,
    // and physical green uses the lower 5-bit bank.
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
    // SPI CONFIGURATION
    // ============================================================
    namespace SPI {
        constexpr int TOUCH_CLOCK_HZ = 1000000;
        constexpr int MAX_TRANSFER_SIZE_BYTES = 32;
        constexpr int DEVICE_QUEUE_SIZE = 1;
    }

    // ============================================================
    // TOUCH CONFIGURATION
    // ============================================================
    namespace Touch {
        constexpr uint16_t RAW_X_MIN = 300;
        constexpr uint16_t RAW_X_MAX = 3850;
        constexpr uint16_t RAW_Y_MIN = 250;
        constexpr uint16_t RAW_Y_MAX = 3750;

        // Verified working command bytes for this panel/Core 2.0.17 setup.
        constexpr uint8_t X_COMMAND = 0x94;
        constexpr uint8_t Y_COMMAND = 0xD4;

        // Odd sample counts allow a true median filter.
        constexpr size_t SAMPLE_COUNT = 3;
        constexpr uint32_t SETTLE_DELAY_TICKS = 1;

        static_assert(RAW_X_MAX > RAW_X_MIN, "Touch X calibration range is invalid");
        static_assert(RAW_Y_MAX > RAW_Y_MIN, "Touch Y calibration range is invalid");
        static_assert(SAMPLE_COUNT >= 3, "Touch median filter requires at least 3 samples");
        static_assert((SAMPLE_COUNT & 1U) == 1U, "Touch sample count must be odd");
        static_assert(SAMPLE_COUNT <= 9, "Touch sample count is intentionally capped for stack safety");
    }

    // ============================================================
    // DISPLAY / BACKLIGHT CONFIGURATION
    // ============================================================
    namespace Display {
        constexpr uint8_t DEFAULT_BRIGHTNESS_PERCENT = 80;
        constexpr uint32_t BACKLIGHT_PWM_HZ = 5000;
        constexpr size_t FILL_BUFFER_LINES = 10;

        static_assert(DEFAULT_BRIGHTNESS_PERCENT <= 100, "Default brightness must be 0-100");
        static_assert(FILL_BUFFER_LINES > 0, "Display fill buffer must contain at least one line");
    }

    // ============================================================
    // MEMORY MANAGER CONFIGURATION
    // ============================================================
    // Change framebuffer memory behaviour here only. MemoryManager.h/.cpp
    // consumes these settings and should not need editing for normal tuning.
    namespace MemoryManager {
        constexpr size_t BUFFER_COUNT = 2;

        constexpr size_t DISPLAY_FRAME_SIZE_BYTES =
            static_cast<size_t>(SCREEN_WIDTH) *
            static_cast<size_t>(SCREEN_HEIGHT) *
            sizeof(uint16_t);

        constexpr size_t BUFFER_SIZE_BYTES = DISPLAY_FRAME_SIZE_BYTES;
        constexpr size_t BUFFER_ALIGNMENT = 64;

        constexpr uint32_t ALLOCATION_CAPS =
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;

        constexpr uint32_t DIAGNOSTIC_CAPS = MALLOC_CAP_SPIRAM;
        constexpr bool ZERO_BUFFERS_ON_INIT = true;

        static_assert(BUFFER_COUNT >= 2, "NV3047 MemoryManager requires at least 2 framebuffers");
        static_assert(BUFFER_ALIGNMENT >= 4, "Framebuffer alignment must be at least 4 bytes");
        static_assert(
            (BUFFER_ALIGNMENT & (BUFFER_ALIGNMENT - 1U)) == 0U,
            "Framebuffer alignment must be a power of two");
        static_assert(
            BUFFER_SIZE_BYTES >= DISPLAY_FRAME_SIZE_BYTES,
            "Framebuffer allocation is smaller than the physical display frame");
        static_assert(
            (BUFFER_SIZE_BYTES % sizeof(uint16_t)) == 0,
            "Framebuffer size must contain whole RGB565 pixels");
    }

    // ============================================================
    // FRAME PRESENTATION CONFIGURATION
    // ============================================================
    namespace Framebuffer {
        constexpr bool USE_32BIT_CLEAR = true;

        constexpr uint32_t PANEL_TOTAL_WIDTH =
            SCREEN_WIDTH + HSYNC_BACK_PORCH + HSYNC_FRONT_PORCH + HSYNC_PULSE_WIDTH;
        constexpr uint32_t PANEL_TOTAL_HEIGHT =
            SCREEN_HEIGHT + VSYNC_BACK_PORCH + VSYNC_FRONT_PORCH + VSYNC_PULSE_WIDTH;
        constexpr uint32_t CALCULATED_SCAN_PERIOD_US =
            static_cast<uint32_t>(
                (static_cast<uint64_t>(PANEL_TOTAL_WIDTH) * PANEL_TOTAL_HEIGHT * 1000000ULL) /
                PCLK_FREQ_HZ);

        // Keep the known-working tuned cadence as the default.
        constexpr uint32_t TUNED_PRESENT_INTERVAL_US = 16546;
        constexpr bool USE_CALCULATED_SCAN_CADENCE = false;
        constexpr uint32_t FRAME_CADENCE_US =
            USE_CALCULATED_SCAN_CADENCE
                ? CALCULATED_SCAN_PERIOD_US
                : TUNED_PRESENT_INTERVAL_US;
    }
}
