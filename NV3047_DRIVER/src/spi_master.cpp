#include "spi_master.h"
#include "config.h"
#include <Arduino.h>

// Instantiate an isolated pointer to the hardware SPI engine.
SPIClass* hspi_bus = NULL;

// FIXED: Dropped down from 40MHz to a stable 2MHz. 
// The NV3047 config engine will ignore anything above 4-6MHz during initialization.
static const SPISettings display_spi_settings(20000000, MSBFIRST, SPI_MODE0); 
static const SPISettings display_init_settings(2000000, MSBFIRST, SPI_MODE0); 

void spi_master_init_bus(void) {
    // 1. Initialize the pointer to the default FSPI hardware slot
    hspi_bus = new SPIClass(FSPI);
    
    // 2. Map the physical pin layout directly to the hardware peripheral matrix
    hspi_bus->begin(PIN_SPI_SCLK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SCREEN_CS);
    
    // 3. Ensure the critical DC line is established as a clean output state
    pinMode(PIN_SCREEN_DC, OUTPUT);
    digitalWrite(PIN_SCREEN_DC, HIGH);
    
    pinMode(PIN_SCREEN_CS, OUTPUT);
    digitalWrite(PIN_SCREEN_CS, HIGH);
}

void spi_master_send_command(uint8_t cmd) {
    // Force transmission on the slow, safe initialization clock profile
    hspi_bus->beginTransaction(display_init_settings);
    
    // Signal to the screen driver chip that the incoming byte is a command
    digitalWrite(PIN_SCREEN_DC, LOW);
    digitalWrite(PIN_SCREEN_CS, LOW); // Select display
    
    hspi_bus->transfer(cmd);          // Push raw byte out
    
    // Tiny delay constraint to allow NV3047 state machine execution time
    delayMicroseconds(10);
    
    digitalWrite(PIN_SCREEN_CS, HIGH); // Deselect display
    hspi_bus->endTransaction();
}

void spi_master_send_data(const uint8_t* data, size_t length) {
    if (length == 0) return;
    
    hspi_bus->beginTransaction(display_init_settings);
    
    // Signal to the screen driver chip that the incoming bytes are data/parameters
    digitalWrite(PIN_SCREEN_DC, HIGH);
    digitalWrite(PIN_SCREEN_CS, LOW);
    
    // Stream data buffer out sequentially
    for (size_t i = 0; i < length; i++) {
        hspi_bus->transfer(data[i]);
        delayMicroseconds(2); // Prevent bus overrun on sensitive controller boards
    }
    
    digitalWrite(PIN_SCREEN_CS, HIGH);
    hspi_bus->endTransaction();
}

void spi_master_stream_pixels(const uint16_t* colors, size_t pixel_count) {
    if (pixel_count == 0) return;
    
    // High-speed pixel operations can resume running at faster speeds if desired
    hspi_bus->beginTransaction(display_init_settings);
    digitalWrite(PIN_SCREEN_DC, HIGH);
    digitalWrite(PIN_SCREEN_CS, LOW);
    
    // Stream 16-bit blocks
    for (size_t i = 0; i < pixel_count; i++) {
        uint16_t pixel = colors[i];
        // Swap bytes if colors appear inverted/incorrect on NV3047 interface layers
        uint16_t swapped_pixel = (pixel >> 8) | (pixel << 8);
        hspi_bus->transfer16(swapped_pixel);
    }
    
    digitalWrite(PIN_SCREEN_CS, HIGH);
    hspi_bus->endTransaction();
}
