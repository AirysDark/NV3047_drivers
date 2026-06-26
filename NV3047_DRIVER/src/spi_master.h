#ifndef CUSTOM_SPI_MASTER_H
#define CUSTOM_SPI_MASTER_H

#include <Arduino.h>
#include <SPI.h>
#include "config.h" // Gives the compiler access to your pin mapping variables

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Bootstraps the hardware SPI bus and maps the pins inside Core 2.0.17.
 */
void spi_master_init_bus(void);

/**
 * @brief Sends a single 8-bit command byte to the display controller glass.
 * Automatically handles pulling the Data/Command (DC) pin LOW.
 */
void spi_master_send_command(uint8_t cmd);

/**
 * @brief Sends an arbitrary array of data bytes (configuration parameters).
 * Automatically handles pulling the Data/Command (DC) pin HIGH.
 */
void spi_master_send_data(const uint8_t* data, size_t length);

/**
 * @brief Blasts a large array of raw 16-bit RGB565 pixel frames to the screen.
 * Uses optimized 32-bit FIFO block streaming for ultra-high framerates.
 */
void spi_master_stream_pixels(const uint16_t* colors, size_t pixel_count);

#ifdef __cplusplus
}
#endif

#endif // CUSTOM_SPI_MASTER_H
