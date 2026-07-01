#pragma once
#include <stdint.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

namespace Config {
    // True Display Resolution (Completely un-padded raw frame layout)
    constexpr uint16_t SCREEN_WIDTH = 480;
    constexpr uint16_t SCREEN_HEIGHT = 272;

    // RGB Bus Pins (16-bit)
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

    // RGB Control Pins
    constexpr int PIN_RGB_DE    = 40;
    constexpr int PIN_RGB_VSYNC = 41;
    constexpr int PIN_RGB_HSYNC = 39;
    constexpr int PIN_RGB_PCLK  = 42;

    // Backlight
    constexpr int PIN_BACKLIGHT = 2;

    // SPI Touch Pins (XPT2046 Dedicated Pinouts)
    constexpr spi_host_device_t SPI_HOST_ID = SPI2_HOST;
    constexpr int PIN_SPI_SCLK  = 20; 
    constexpr int PIN_SPI_MOSI  = 19; 
    constexpr int PIN_SPI_MISO  = -1; 
    constexpr int PIN_TOUCH_CS  = 18; 
    constexpr int PIN_TOUCH_IRQ = 36; 
    
    // --- LATEST STABLE WORKING TIMINGS ---
    // --- OFFICIAL ELECROW FACTORY TIMINGS FOR DIS06043H ---
    // Slower, rock-solid baseline clock paired with precise horizontal and vertical blanking gates
    // --- LOCKED CROWPANEL 4.3 PRODUCTION TIMINGS ---
    // Cleans pixel jitter completely and provides the ultimate sharp baseline.
    // --- STABILISED CLOCK AND HORIZONTAL GATE PORCHES ---
    // Dropping the PCLK to 6MHz gives the unshielded PCB copper traces
    // plenty of time to clear, completely straightening the jagged middle artifacts.
    constexpr int PCLK_FREQ_HZ = 6000000; // Lowered to 6MHz for trace stability
    
    constexpr uint16_t HSYNC_BACK_PORCH  = 43;
    constexpr uint16_t HSYNC_FRONT_PORCH = 8;
    constexpr uint16_t HSYNC_PULSE_WIDTH = 2;
    constexpr uint16_t VSYNC_BACK_PORCH  = 12;
    constexpr uint16_t VSYNC_FRONT_PORCH = 8;
    constexpr uint16_t VSYNC_PULSE_WIDTH = 2;


    // --- COLOR PALETTE DEFINITIONS ---
    constexpr uint16_t COLOR_RED        = 0xF800; 
    constexpr uint16_t COLOR_GREEN      = 0x001F; 
    constexpr uint16_t COLOR_BLUE       = 0x07E0; 
    constexpr uint16_t COLOR_WHITE      = 0xFFFF; 
    constexpr uint16_t COLOR_BLACK      = 0x0000;
    constexpr uint16_t COLOR_LIGHT_GREY = 0xF7DE; 
    constexpr uint16_t COLOR_SIDE_WHITE  = 0xFA3F; 

    namespace Framebuffer {
        constexpr size_t NUM_BUFFERS = 2;
        constexpr size_t BUFFER_SIZE_BYTES = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t);
        constexpr bool USE_32BIT_CLEAR = true;
    }
}
