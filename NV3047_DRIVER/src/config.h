#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// ESP32-S3-WROOM-1-N4R2 ARDUINO CORE 2.0.17 PIN SPECIFICATION
// ============================================================================

// 1. Hardware SPI Bus (Display & Shared SD Card Data)
extern const uint8_t PIN_SPI_MOSI;
extern const uint8_t PIN_SPI_SCLK;
extern const uint8_t PIN_SPI_MISO;

// Display Control Lines
extern const uint8_t PIN_SCREEN_CS;
extern const uint8_t PIN_SCREEN_DC;
extern const uint8_t PIN_SCREEN_RST;
extern const uint8_t PIN_SCREEN_BL;

// SPI Touch Controller
extern const uint8_t PIN_TOUCH_SPI_CS;
extern const uint8_t PIN_TOUCH_SPI_IRQ;

// SPI SD Card CS
extern const uint8_t PIN_SD_SPI_CS;

// 2. Hardware I2C Bus (Capacitive Touch Screens & Sensors)
extern const uint8_t PIN_I2C_SDA;
extern const uint8_t PIN_I2C_SCL;
extern const uint8_t PIN_TOUCH_I2C_RST;
extern const uint8_t PIN_TOUCH_I2C_INT;

// 3. High-Speed Parallel SDMMC Bus
extern const uint8_t PIN_SDMMC_CLK;
extern const uint8_t PIN_SDMMC_CMD;
extern const uint8_t PIN_SDMMC_D0;
extern const uint8_t PIN_SDMMC_D1;
extern const uint8_t PIN_SDMMC_D2;
extern const uint8_t PIN_SDMMC_D3;

// 4. Hardware UART / Serial & Audio Buses
extern const uint8_t PIN_UART0_TX;
extern const uint8_t PIN_UART0_RX;
extern const uint8_t PIN_UART2_TX;
extern const uint8_t PIN_UART2_RX;
extern const uint8_t PIN_I2S_BCK;
extern const uint8_t PIN_I2S_WS;
extern const uint8_t PIN_I2S_DO;

// 5. Native USB Interface
extern const uint8_t PIN_USB_DM;
extern const uint8_t PIN_USB_DP;

// 6. Safe General Purpose IO
extern const uint8_t PIN_GPIO_4;
extern const uint8_t PIN_GPIO_5;
extern const uint8_t PIN_GPIO_6;
extern const uint8_t PIN_GPIO_7;
extern const uint8_t PIN_GPIO_8;
extern const uint8_t PIN_GPIO_15;
extern const uint8_t PIN_GPIO_16;
extern const uint8_t PIN_GPIO_35;
extern const uint8_t PIN_GPIO_36;
extern const uint8_t PIN_GPIO_37;
extern const uint8_t PIN_BOARD_LED;

// ============================================================================
// CORE INITIALISATION PROTOTYPES
// ============================================================================
/**
 * @brief Sets up baseline I/O safety states via Core 2.0.17 handlers.
 * Disables bus conflicts and drops screens out of active reset layout loops.
 */
void app_config_init_hardware(void);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_H
