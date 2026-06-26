#include "display_manager.h"
#include "nv3047.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include <string.h>

static const char *TAG = "DISPLAY_MGR";

static uint16_t *framebuffer = NULL;

// FIXED: Adjusted from 320x240 to match your true 4.3" NV3047 480x272 widescreen resolution.
// This layout change eliminates memory address misalignment that causes image crawling.
#define LCD_WIDTH  480
#define LCD_HEIGHT 272

// ======================================================
// INIT DISPLAY + FRAMEBUFFER
// ======================================================
// FIXED: Renamed function to resolve linker conflict with display.cpp
esp_err_t display_manager_init(esp_lcd_panel_handle_t panel)
{
    ESP_LOGI(TAG, "Allocating DMA framebuffer for 480x272 display...");

    // Allocate frame container within high-speed internal DMA memory limits
    framebuffer = (uint16_t*)heap_caps_malloc(
        LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
        MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL
    );

    if (!framebuffer) {
        ESP_LOGE(TAG, "Widescreen Framebuffer allocation failed!");
        return ESP_ERR_NO_MEM;
    }

    // Clear buffer array to pure black
    memset(framebuffer, 0x00, LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t));

    ESP_LOGI(TAG, "Flushing initial frame to display surface...");

    esp_err_t err = esp_lcd_panel_draw_bitmap(
        panel,
        0, 0,
        LCD_WIDTH,
        LCD_HEIGHT,
        framebuffer
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Initial flush failed: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

// ======================================================
// CLEAR SCREEN
// ======================================================
void display_clear(uint16_t color, esp_lcd_panel_handle_t panel)
{
    if (!framebuffer) return;

    int size = LCD_WIDTH * LCD_HEIGHT;

    for (int i = 0; i < size; i++) {
        framebuffer[i] = color;
    }

    esp_lcd_panel_draw_bitmap(
        panel,
        0, 0,
        LCD_WIDTH,
        LCD_HEIGHT,
        framebuffer
    );
}

// ======================================================
// RGB TEST PATTERN
// ======================================================
void display_test_pattern(esp_lcd_panel_handle_t panel)
{
    if (!framebuffer) return;

    const int w = LCD_WIDTH;
    const int h = LCD_HEIGHT;

    // Split the 480-pixel width into three clean vertical bands
    const int band_width = w / 3; // 480 / 3 = 160 pixels per vertical column band

    for (int y = 0; y < h; y++) {
        uint16_t *row = &framebuffer[y * w];

        for (int x = 0; x < w; x++) {
            if (x < band_width) {
                row[x] = 0xF800; // Pure RED
            }
            else if (x < (band_width * 2)) {
                row[x] = 0x07E0; // Pure GREEN
            }
            else {
                row[x] = 0x001F; // Pure BLUE
            }
        }
    }

    esp_lcd_panel_draw_bitmap(
        panel,
        0, 0,
        LCD_WIDTH,
        LCD_HEIGHT,
        framebuffer
    );
}
