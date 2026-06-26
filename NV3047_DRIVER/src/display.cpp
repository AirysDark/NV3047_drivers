#include "display.h"
#include "nv3047_cmds.h"
#include "nv3047_driver.h" 
#include "display_manager.h" // Links directly to your widescreen engine
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h" 
#include "esp_log.h"
#include <string.h>

esp_lcd_panel_handle_t panel_handle = NULL;

esp_lcd_panel_io_handle_t lcd_spi_init(void)
{
    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_config;
    memset(&io_config, 0, sizeof(io_config));

    io_config.dc_gpio_num = (int)PIN_SCREEN_DC;
    io_config.cs_gpio_num = (int)PIN_SCREEN_CS;
    io_config.pclk_hz = 2 * 1000 * 1000;  
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    io_config.spi_mode = 0;               
    io_config.trans_queue_depth = 10;
    
    esp_err_t ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io); 
    if (ret != ESP_OK) {
        return NULL;
    }
    return io;
}

void display_init(void)
{
    esp_lcd_panel_io_handle_t io_handle = lcd_spi_init();
    if (io_handle == NULL) return;

    esp_err_t ret = nv3047_new_panel(io_handle, &panel_handle);
    if (ret != ESP_OK) return;

    // 1. Awaken and configure the internal registers of the chip
    nv3047_init();

    // 2. Fire up your custom bare-metal widescreen framebuffer matrix
    display_manager_init(panel_handle);

    // 3. Immediately draw the raw Red, Green, and Blue testing lines
    display_test_pattern(panel_handle);
}
