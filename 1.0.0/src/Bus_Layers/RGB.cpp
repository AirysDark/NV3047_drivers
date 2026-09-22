#include "RGB.h"
#include <string.h>
#include <esp_lcd_panel_rgb.h> // Fixed: Restores the missing parallel display types to the compiler

bool RGB::init() {
    esp_lcd_rgb_panel_config_t panel_config = {};
    
    panel_config.data_width = 16;
    panel_config.psram_trans_align = 64;
    panel_config.sram_trans_align = 8;
    panel_config.clk_src = LCD_CLK_SRC_PLL160M;
    panel_config.disp_gpio_num = -1;
    panel_config.pclk_gpio_num = Config::PIN_RGB_PCLK;
    panel_config.vsync_gpio_num = Config::PIN_RGB_VSYNC;
    panel_config.hsync_gpio_num = Config::PIN_RGB_HSYNC;
    panel_config.de_gpio_num = Config::PIN_RGB_DE;
    
    // --- CROWPANEL FIXED PCB BUS ROUTING ---
    // Physical mapping matching the sequential parallel bus layout.
    
    // Blue Channel (Bits 0-4)
    panel_config.data_gpio_nums[0] = Config::PIN_RGB_B0;
    panel_config.data_gpio_nums[1] = Config::PIN_RGB_B1;
    panel_config.data_gpio_nums[2] = Config::PIN_RGB_B2;
    panel_config.data_gpio_nums[3] = Config::PIN_RGB_B3;
    panel_config.data_gpio_nums[4] = Config::PIN_RGB_B4;
    
    // Green Channel (Bits 5-10)
    panel_config.data_gpio_nums[5] = Config::PIN_RGB_G0;
    panel_config.data_gpio_nums[6] = Config::PIN_RGB_G1;
    panel_config.data_gpio_nums[7] = Config::PIN_RGB_G2;
    panel_config.data_gpio_nums[8] = Config::PIN_RGB_G3;
    panel_config.data_gpio_nums[9] = Config::PIN_RGB_G4;
    panel_config.data_gpio_nums[10] = Config::PIN_RGB_G5;
    
    // Red Channel (Bits 11-15)
    panel_config.data_gpio_nums[11] = Config::PIN_RGB_R0;
    panel_config.data_gpio_nums[12] = Config::PIN_RGB_R1;
    panel_config.data_gpio_nums[13] = Config::PIN_RGB_R2;
    panel_config.data_gpio_nums[14] = Config::PIN_RGB_R3;
    panel_config.data_gpio_nums[15] = Config::PIN_RGB_R4;

    panel_config.timings.pclk_hz = Config::PCLK_FREQ_HZ;
    panel_config.timings.h_res = Config::SCREEN_WIDTH;
    panel_config.timings.v_res = Config::SCREEN_HEIGHT;
    
    panel_config.timings.hsync_back_porch  = Config::HSYNC_BACK_PORCH;
    panel_config.timings.hsync_front_porch = Config::HSYNC_FRONT_PORCH;
    panel_config.timings.hsync_pulse_width = Config::HSYNC_PULSE_WIDTH;
    panel_config.timings.vsync_back_porch  = Config::VSYNC_BACK_PORCH;
    panel_config.timings.vsync_front_porch = Config::VSYNC_FRONT_PORCH;
    panel_config.timings.vsync_pulse_width = Config::VSYNC_PULSE_WIDTH;
    
    // --- VERIFIED CRYSTAL CLEAR HARDWARE PROPERTIES ---
    // Enforces falling edge latching and public active-high sync windows.
    panel_config.timings.flags.pclk_active_neg = 1; // 1 = Latch data on stable falling edge
    panel_config.timings.flags.hsync_idle_low  = 0; // 0 = HSYNC is Active-High
    panel_config.timings.flags.vsync_idle_low  = 0; // 0 = VSYNC is Active-High
    panel_config.timings.flags.de_idle_high    = 0; 
    panel_config.timings.flags.pclk_idle_high  = 0; 





    
    panel_config.flags.fb_in_psram = 1; 

    esp_err_t ret = esp_lcd_new_rgb_panel(&panel_config, &panel_handle);
    return (ret == ESP_OK);
}

esp_lcd_panel_handle_t RGB::getHandle() { return panel_handle; }

uint16_t RGB::getColor(const char* colorName) {
    if (strcmp(colorName, "RED") == 0)             return Config::COLOR_RED;
    if (strcmp(colorName, "GREEN") == 0)           return Config::COLOR_GREEN;
    if (strcmp(colorName, "BLUE") == 0)            return Config::COLOR_BLUE;
    if (strcmp(colorName, "WHITE") == 0)           return Config::COLOR_WHITE;
    if (strcmp(colorName, "LIGHT_GREY") == 0)      return Config::COLOR_LIGHT_GREY;
    if (strcmp(colorName, "SIDE_WHITE") == 0)      return Config::COLOR_SIDE_WHITE;
    return Config::COLOR_BLACK;
}
