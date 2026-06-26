#include "config.h"
#include <Arduino.h>

// 1. Hardware SPI Bus Definition
const uint8_t PIN_SPI_MOSI      = 11;
const uint8_t PIN_SPI_SCLK      = 12;
const uint8_t PIN_SPI_MISO      = 13;

const uint8_t PIN_SCREEN_CS     = 10;
const uint8_t PIN_SCREEN_DC      = 9;
const uint8_t PIN_SCREEN_RST     = 14;
const uint8_t PIN_SCREEN_BL      = 21;

const uint8_t PIN_TOUCH_SPI_CS  = 4;
const uint8_t PIN_TOUCH_SPI_IRQ = 5;
const uint8_t PIN_SD_SPI_CS     = 6;

// 2. Hardware I2C Bus Definition (FIXED: Relocated to safe pins 7 & 8 to resolve SDMMC crash)
const uint8_t PIN_I2C_SDA       = 7;
const uint8_t PIN_I2C_SCL       = 8;
const uint8_t PIN_TOUCH_I2C_RST = 3;
const uint8_t PIN_TOUCH_I2C_INT = 4;

// 3. High-Speed Parallel SDMMC Bus Definition
const uint8_t PIN_SDMMC_CLK     = 39;
const uint8_t PIN_SDMMC_CMD     = 40;
const uint8_t PIN_SDMMC_D0      = 41;
const uint8_t PIN_SDMMC_D1      = 42;
const uint8_t PIN_SDMMC_D2      = 2;
const uint8_t PIN_SDMMC_D3      = 1;

// 4. Hardware UART / Serial & Audio Buses Definition
const uint8_t PIN_UART0_TX      = 43;
const uint8_t PIN_UART0_RX      = 44;
const uint8_t PIN_UART2_TX      = 17;
const uint8_t PIN_UART2_RX      = 18;
const uint8_t PIN_I2S_BCK       = 15;
const uint8_t PIN_I2S_WS        = 16;
const uint8_t PIN_I2S_DO        = 17;

// 5. Native USB Interface Definition
const uint8_t PIN_USB_DM        = 19;
const uint8_t PIN_USB_DP        = 20;

// 6. Safe General Purpose IO Definition
const uint8_t PIN_GPIO_4        = 4;
const uint8_t PIN_GPIO_5        = 5;
const uint8_t PIN_GPIO_6        = 6;
const uint8_t PIN_GPIO_7        = 7;
const uint8_t PIN_GPIO_8        = 8;
const uint8_t PIN_GPIO_15       = 15;
const uint8_t PIN_GPIO_16       = 16;
const uint8_t PIN_GPIO_35       = 35;
const uint8_t PIN_GPIO_36       = 36;
const uint8_t PIN_GPIO_37       = 37;
const uint8_t PIN_BOARD_LED     = 38; 

// ============================================================================
// REGISTRY AND HARDWARE INITIALISATION ROUTINE
// ============================================================================
void app_config_init_hardware(void) {
    // 1. Initialise main display control lines as pure digital outputs
    pinMode(PIN_SCREEN_CS, OUTPUT);
    digitalWrite(PIN_SCREEN_CS, HIGH); // Pull high to deselect screen at bootup

    pinMode(PIN_SCREEN_DC, OUTPUT);
    digitalWrite(PIN_SCREEN_DC, LOW);

    // 2. FIXED: Critical Hard Reset Sequence for the NV3047 Controller
    pinMode(PIN_SCREEN_RST, OUTPUT);
    digitalWrite(PIN_SCREEN_RST, HIGH); 
    delay(20);
    digitalWrite(PIN_SCREEN_RST, LOW);  // Force hardware line low to purge internal states
    delay(50);                          // Hold line to ensure complete discharge
    digitalWrite(PIN_SCREEN_RST, HIGH); // Pull back high to release reset latch
    delay(120);                         // Wait for internal chip oscillator stability

    // 3. Drive backlight power high
    pinMode(PIN_SCREEN_BL, OUTPUT);
    digitalWrite(PIN_SCREEN_BL, HIGH); // Turn backlight full brightness

    // 4. Clear secondary shared SPI device select lines to avoid bus conflicts
    pinMode(PIN_TOUCH_SPI_CS, OUTPUT);
    digitalWrite(PIN_TOUCH_SPI_CS, HIGH);

    pinMode(PIN_SD_SPI_CS, OUTPUT);
    digitalWrite(PIN_SD_SPI_CS, HIGH);

    // 5. Set up the local diagnostic on-board LED pin
    pinMode(PIN_BOARD_LED, OUTPUT);
    digitalWrite(PIN_BOARD_LED, LOW);
}
