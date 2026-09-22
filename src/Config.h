#pragma once

#include <stddef.h>
#include <stdint.h>

#include <driver/gpio.h>
#include <esp_heap_caps.h>

namespace Config {
    constexpr uint16_t SCREEN_WIDTH = 480;
    constexpr uint16_t SCREEN_HEIGHT = 272;

    // ============================================================
    // RGB PANEL PROFILE
    // ============================================================
    // LEGACY_WORKING:
    //   Hardware-tested mapping already known to produce a stable image.
    //
    // V21_MATRIX_TEST:
    //   Proposed V2.1 GPIO-matrix assignment. Use only for controlled testing.
    //   It intentionally keeps the proven legacy timing values so pin routing
    //   and colour order can be evaluated without changing two variables at once.
    enum class RGBProfile : uint8_t {
        LEGACY_WORKING = 0,
        V21_MATRIX_TEST = 1
    };

    constexpr RGBProfile ACTIVE_RGB_PROFILE =
        RGBProfile::LEGACY_WORKING;

    constexpr bool RGB_V21_MATRIX_ACTIVE =
        ACTIVE_RGB_PROFILE == RGBProfile::V21_MATRIX_TEST;

    constexpr const char* RGB_PROFILE_NAME =
        RGB_V21_MATRIX_ACTIVE
            ? "V21_MATRIX_TEST"
            : "LEGACY_WORKING";

    // ============================================================
    // RGB PANEL PINS
    // ============================================================
    constexpr int PIN_RGB_B0 = RGB_V21_MATRIX_ACTIVE ? 5  : 15;
    constexpr int PIN_RGB_B1 = RGB_V21_MATRIX_ACTIVE ? 6  : 7;
    constexpr int PIN_RGB_B2 = RGB_V21_MATRIX_ACTIVE ? 7  : 6;
    constexpr int PIN_RGB_B3 = RGB_V21_MATRIX_ACTIVE ? 15 : 5;
    constexpr int PIN_RGB_B4 = RGB_V21_MATRIX_ACTIVE ? 16 : 4;

    constexpr int PIN_RGB_G0 = RGB_V21_MATRIX_ACTIVE ? 1  : 9;
    constexpr int PIN_RGB_G1 = RGB_V21_MATRIX_ACTIVE ? 48 : 46;
    constexpr int PIN_RGB_G2 = RGB_V21_MATRIX_ACTIVE ? 47 : 3;
    constexpr int PIN_RGB_G3 = RGB_V21_MATRIX_ACTIVE ? 21 : 8;
    constexpr int PIN_RGB_G4 = RGB_V21_MATRIX_ACTIVE ? 14 : 16;
    constexpr int PIN_RGB_G5 = RGB_V21_MATRIX_ACTIVE ? 38 : 1;

    constexpr int PIN_RGB_R0 = RGB_V21_MATRIX_ACTIVE ? 45 : 14;
    constexpr int PIN_RGB_R1 = RGB_V21_MATRIX_ACTIVE ? 42 : 21;
    constexpr int PIN_RGB_R2 = RGB_V21_MATRIX_ACTIVE ? 41 : 47;
    constexpr int PIN_RGB_R3 = RGB_V21_MATRIX_ACTIVE ? 40 : 48;
    constexpr int PIN_RGB_R4 = RGB_V21_MATRIX_ACTIVE ? 39 : 45;

    constexpr int PIN_RGB_DE =
        RGB_V21_MATRIX_ACTIVE ? 4 : 40;
    constexpr int PIN_RGB_VSYNC =
        RGB_V21_MATRIX_ACTIVE ? 3 : 41;
    constexpr int PIN_RGB_HSYNC =
        RGB_V21_MATRIX_ACTIVE ? 46 : 39;
    constexpr int PIN_RGB_PCLK =
        RGB_V21_MATRIX_ACTIVE ? 9 : 42;

    constexpr int PIN_BACKLIGHT = 2;

    // ============================================================
    // SHARED PERIPHERAL SPI BUS + TOUCH PINS
    // ============================================================
    // PCB silkscreen + Elecrow V2.1 definition:
    // IO12 = TP_CLK, IO11 = TP_DIN, IO13 = TP_OUT,
    // IO0  = TP_CS,  IO36 = TP_IRQ.
    //
    // TF/microSD shares the same clock/data lines with its own CS on IO10.
    constexpr int PIN_SHARED_SPI_SCLK = 12;
    constexpr int PIN_SHARED_SPI_MOSI = 11;
    constexpr int PIN_SHARED_SPI_MISO = 13;

    constexpr int PIN_TOUCH_CS  = 0;
    constexpr int PIN_TOUCH_IRQ = 36;

    // Backward-compatible aliases used by existing bus-layer code.
    constexpr int PIN_SPI_SCLK = PIN_SHARED_SPI_SCLK;
    constexpr int PIN_SPI_MOSI = PIN_SHARED_SPI_MOSI;
    constexpr int PIN_SPI_MISO = PIN_SHARED_SPI_MISO;

    // ============================================================
    // MICRO-SD (TF) CARD PINS
    // ============================================================
    // TF shares the SPI data/clock bus with touch and has an independent CS.
    constexpr int PIN_SD_CS   = 10;
    constexpr int PIN_SD_CLK  = PIN_SHARED_SPI_SCLK;
    constexpr int PIN_SD_MOSI = PIN_SHARED_SPI_MOSI;
    constexpr int PIN_SD_MISO = PIN_SHARED_SPI_MISO;

    // ============================================================
    // EXTERNAL EXPANSION / BOARD SILKSCREEN PINS
    // ============================================================
    namespace Expansion {
        constexpr int UART1_RX = 18;
        constexpr int UART1_TX = 17;

        constexpr int GPIO_D0 = 38;
        constexpr int GPIO_D1 = 37;

        // Proposed V2.1 RGB profile consumes GPIO38 as LCD G5.
        constexpr bool GPIO_D0_AVAILABLE =
            !RGB_V21_MATRIX_ACTIVE;
        constexpr bool GPIO_D1_AVAILABLE = true;
    }

    // ============================================================
    // I2S BOARD SILKSCREEN PINS
    // ============================================================
    namespace I2S {
        constexpr int LRCLK = 19;
        constexpr int BCLK  = 35;
        constexpr int SDIN  = 20;
    }

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

        static_assert(DATA_WIDTH == 16, "This NV3047 driver build is fixed to a 16-bit RGB bus");
        static_assert(
            PSRAM_TRANSFER_ALIGNMENT > 0 &&
            (PSRAM_TRANSFER_ALIGNMENT & (PSRAM_TRANSFER_ALIGNMENT - 1U)) == 0U,
            "RGB PSRAM transfer alignment must be a nonzero power of two");
        static_assert(
            SRAM_TRANSFER_ALIGNMENT > 0 &&
            (SRAM_TRANSFER_ALIGNMENT & (SRAM_TRANSFER_ALIGNMENT - 1U)) == 0U,
            "RGB SRAM transfer alignment must be a nonzero power of two");
    }

    // ============================================================
    // PANEL COLOUR MAPPING
    // ============================================================
    // Legacy hardware testing required GREEN/BLUE compensation.
    // The V2.1 matrix test profile intentionally uses textbook RGB565 so the
    // test can reveal whether the legacy colour swap was caused by GPIO/data
    // lane ordering rather than the panel's native colour format.
    constexpr uint16_t packPanelColor(
        uint8_t red,
        uint8_t green,
        uint8_t blue) {

        return RGB_V21_MATRIX_ACTIVE
            ? static_cast<uint16_t>(
                (static_cast<uint16_t>(red & 0xF8U) << 8) |
                (static_cast<uint16_t>(green & 0xFCU) << 3) |
                (static_cast<uint16_t>(blue) >> 3))
            : static_cast<uint16_t>(
                (static_cast<uint16_t>(red & 0xF8U) << 8) |
                (static_cast<uint16_t>(blue & 0xFCU) << 3) |
                (static_cast<uint16_t>(green) >> 3));
    }

    constexpr uint16_t COLOR_RED = 0xF800;
    constexpr uint16_t COLOR_GREEN =
        RGB_V21_MATRIX_ACTIVE ? 0x07E0 : 0x001F;
    constexpr uint16_t COLOR_BLUE =
        RGB_V21_MATRIX_ACTIVE ? 0x001F : 0x07E0;
    constexpr uint16_t COLOR_WHITE      = 0xFFFF;
    constexpr uint16_t COLOR_BLACK      = 0x0000;
    constexpr uint16_t COLOR_LIGHT_GREY = 0xF7DE;
    constexpr uint16_t COLOR_SIDE_WHITE = 0xFA3F;

    // ============================================================
    // SPI CONFIGURATION
    // ============================================================
    namespace SPI {
        constexpr int TOUCH_CLOCK_HZ = 1000000;

        static_assert(TOUCH_CLOCK_HZ > 0, "Touch SPI clock must be greater than zero");
    }


    // ============================================================
    // SD CARD CONFIGURATION
    // ============================================================
    namespace SDCard {
        constexpr bool ENABLED = true;
        constexpr bool SHARES_TOUCH_SPI_BUS = true;

        // Conservative default for broad card compatibility.
        constexpr uint32_t CLOCK_HZ = 4000000;

        constexpr const char* MOUNT_POINT = "/sd";
        constexpr uint8_t MAX_OPEN_FILES = 5;
        constexpr bool FORMAT_IF_MOUNT_FAILED = false;

        // Never shut down the shared SPI bus during SD eject; touch still needs it.
        constexpr bool END_SPI_ON_UNMOUNT = false;

        static_assert(CLOCK_HZ > 0, "SD card SPI clock must be greater than zero");
        static_assert(MAX_OPEN_FILES > 0, "SD card must allow at least one open file");
        static_assert(PIN_SD_CS >= 0, "TF chip-select GPIO must be configured");
        static_assert(
            PIN_SD_CS != PIN_TOUCH_CS,
            "TF and touch require independent chip-select GPIOs");
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
        static_assert(BACKLIGHT_PWM_HZ > 0, "Backlight PWM frequency must be greater than zero");
        static_assert(FILL_BUFFER_LINES > 0, "Display fill buffer must contain at least one line");
        static_assert(
            FILL_BUFFER_LINES <= SCREEN_HEIGHT,
            "Display fill buffer cannot exceed screen height");
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
        static_assert(
            (BUFFER_SIZE_BYTES % BUFFER_ALIGNMENT) == 0,
            "Framebuffer size must be a multiple of its configured alignment");
    }

    // ============================================================
    // FRAME PRESENTATION CONFIGURATION
    // ============================================================
    namespace Framebuffer {
        constexpr bool USE_32BIT_CLEAR = true;

        static_assert(PCLK_FREQ_HZ > 0, "Pixel clock must be greater than zero");

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
