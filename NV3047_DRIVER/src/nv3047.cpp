#include "nv3047.h"
#include "nv3047_cmds.h"
#include "spi_master.h"
#include <stdint.h>
#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// PRIVATE DRIVER HELPERS
// ======================================================

// Helper function to bundle command and parameter sequence logic
static void write_reg(uint8_t cmd, const uint8_t *params, size_t length) {
    spi_master_send_command(cmd);
    if (length > 0 && params != NULL) {
        spi_master_send_data(params, length);
    }
}

// Helper to bundle single parameter commands
static void write_reg_byte(uint8_t cmd, uint8_t param) {
    write_reg(cmd, &param, 1);
}

// ======================================================
// LOW LEVEL LIFECYCLE DRIVER API IMPLEMENTATION
// ======================================================

esp_err_t nv3047_init(void)
{
    // 1. Initial Wake Up Sequence
    nv3047_sleep_out();

    // 2. NV3047 Specific Manufacturer Extension Commands Latch (Unlock Protected Registers)
    write_reg_byte(NV3047_CMD_EXT_UNLOCK, 0x30); 
    write_reg_byte(NV3047_CMD_EXT_UNLOCK, 0x47); 

    // 3. Frame Rate Control & Display Timing Config
    uint8_t frame_rate_data[] = {0x00, 0x22}; // Standard 60Hz porch calculations for 480x272
    write_reg(NV3047_CMD_FRAME_RATE, frame_rate_data, 2);

    // 4. Panel Timing & Interface Control Settings
    uint8_t interface_cfg[] = {0x02, 0x02, 0x14}; // RGB interface settings, HSYNC/VSYNC polarity
    write_reg(NV3047_CMD_INTERFACE_CTRL, interface_cfg, 3);

    // 5. Set Resolution Addressing Bounds (Default 480 x 272 Window Layout)
    nv3047_set_window(0, 0, 479, 271);

    // 6. Memory Access Control (Orientation alignment)
    write_reg_byte(NV3047_CMD_MADCTL, 0x70); // BGR/RGB mapping configuration filter 

    // 7. Interface Pixel Format
    write_reg_byte(NV3047_CMD_PIXEL_FORMAT, 0x05); // 16-bit/pixel format (RGB565 Mode)

    // 8. Relock Engine Protection Layer
    write_reg_byte(NV3047_CMD_EXT_UNLOCK, 0x00);

    // 9. Fire Display Ignition Array
    nv3047_display_on();

    return ESP_OK;
}

esp_err_t nv3047_sleep_out(void)
{
    spi_master_send_command(NV3047_CMD_SLEEP_OUT);
    delay(120); // Required delay for internal charge pump stability
    return ESP_OK;
}

esp_err_t nv3047_display_on(void)
{
    spi_master_send_command(NV3047_CMD_DISPLAY_ON);
    delay(20);
    return ESP_OK;
}

esp_err_t nv3047_display_off(void)
{
    spi_master_send_command(NV3047_CMD_DISPLAY_OFF);
    delay(20);
    return ESP_OK;
}

// ======================================================
// CONFIGURATION MATRIX IMPLEMENTATION
// ======================================================

esp_err_t nv3047_set_rotation(uint8_t rotation)
{
    uint8_t madctl_val = 0x00;
    
    // Standard MADCTL rotations mapping for NV3047
    switch (rotation % 4) {
        case 0: madctl_val = 0x70; break; // Landscape Normal
        case 1: madctl_val = 0x20; break; // Portrait
        case 2: madctl_val = 0xB0; break; // Landscape Inverted
        case 3: madctl_val = 0xE0; break; // Portrait Inverted
    }
    
    write_reg_byte(NV3047_CMD_MADCTL, madctl_val);
    return ESP_OK;
}

esp_err_t nv3047_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t col_address[] = { (uint8_t)(x0 >> 8), (uint8_t)(x0 & 0xFF), (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF) };
    uint8_t row_address[] = { (uint8_t)(y0 >> 8), (uint8_t)(y0 & 0xFF), (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF) };
    
    write_reg(NV3047_CMD_CASET, col_address, 4);
    write_reg(NV3047_CMD_RASET, row_address, 4);
    return ESP_OK;
}

// ======================================================
// PIXEL DATA STREAMING ENGINE
// ======================================================

esp_err_t nv3047_write_pixels(const uint16_t *data, size_t len)
{
    if (len == 0 || data == NULL) return ESP_ERR_INVALID_ARG;
    
    spi_master_send_command(NV3047_CMD_RAMWR);
    spi_master_stream_pixels(data, len);
    return ESP_OK;
}

void nv3047_fill(uint16_t color)
{
    spi_master_send_command(NV3047_CMD_RAMWR);
    
    size_t total_pixels = 480 * 272;
    uint16_t chunk_buffer[256];
    
    for (int i = 0; i < 256; i++) {
        chunk_buffer[i] = color;
    }
    
    size_t remaining = total_pixels;
    while (remaining > 0) {
        size_t current_chunk = (remaining > 256) ? 256 : remaining;
        spi_master_stream_pixels(chunk_buffer, current_chunk);
        remaining -= current_chunk;
    }
}

void nv3047_test_pattern(void)
{
    spi_master_send_command(NV3047_CMD_RAMWR); 
    
    // Allocate a safe chunk size instead of a 480-word array to protect stack bounds
    uint16_t line_buffer[240]; 
    
    for (int y = 0; y < 272; y++) {
        uint16_t color = 0x0000;
        if (y < 90)       color = 0xF800; // Red block band
        else if (y < 180) color = 0x07E0; // Green block band
        else              color = 0x001F; // Blue block band
        
        for (int x = 0; x < 240; x++) {
            line_buffer[x] = color;
        }
        
        // Output two chunks to make up a single full 480 pixel horizontal line scan
        spi_master_stream_pixels(line_buffer, 240);
        spi_master_stream_pixels(line_buffer, 240);
    }
}

#ifdef __cplusplus
}
#endif
