#include "DisplayDriver.h"
#include <driver/ledc.h>
#include <esp_heap_caps.h>

bool DisplayDriver::init(esp_lcd_panel_handle_t rgb_handle) {
    if (!rgb_handle) return false;
    handle = rgb_handle;
    
    // Establish initial tracking parameters
    current_brightness = 204; // FIXED: Sets starting duty directly to 80% (204 out of 255)
    is_sleeping = false;
    
    // --- CRITICAL FACTORY HARDWARE INITIALIZATION ---
    // Forces the display panel's low-level registers to align before parallel bus data streams begin
    esp_lcd_panel_reset(handle);
    esp_lcd_panel_init(handle);
    
    // 1. Configure the High-Speed LEDC PWM Hardware Timer Engine
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num = LEDC_TIMER_0;
    ledc_timer.duty_resolution = LEDC_TIMER_8_BIT;
    ledc_timer.freq_hz = 5000;
    ledc_timer_config(&ledc_timer);

    // 2. Bind the Configured Channel directly to the Backlight physical GPIO Pin
    ledc_channel_config_t ledc_channel = {};
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc_channel.timer_sel = LEDC_TIMER_0;
    ledc_channel.intr_type = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num = Config::PIN_BACKLIGHT;
    ledc_channel.duty = current_brightness; // FIXED: Set to 204 so the screen powers up bright instantly
    ledc_channel_config(&ledc_channel);

    return true;
}

void DisplayDriver::setBacklight(uint8_t brightness) {
    if (is_sleeping) return; // Prevent breaking power profiles if screen is asleep
    
    current_brightness = brightness;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

// --- HARDWARE LIFECYCLE MANAGEMENT ---

void DisplayDriver::setBrightness(uint8_t percentage) {
    if (percentage > 100) percentage = 100;
    
    // Scale 0-100% smoothly into an 8-bit duty resolution value (0-255)
    uint16_t duty = ((uint16_t)percentage * 255) / 100;
    setBacklight((uint8_t)duty);
}

void DisplayDriver::sleep() {
    if (is_sleeping) return;
    
    // Force the physical PWM line to zero to turn off the backlight panel completely
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    is_sleeping = true;
}

void DisplayDriver::wake() {
    if (!is_sleeping) return;
    
    is_sleeping = false;
    // Restore the hardware bus immediately back to its previous working state
    setBacklight(current_brightness);
}

// --- SCREEN FILL PRIMITIVES ---

void DisplayDriver::drawBitmap(int x, int y, int w, int h, const uint16_t *bitmap) {
    esp_lcd_panel_draw_bitmap(handle, x, y, x + w, y + h, (void*)bitmap);
}

void DisplayDriver::fillScreen(uint16_t color) {
    const int lines = 10;
    size_t buf_size = Config::SCREEN_WIDTH * lines;
    uint16_t* color_buf = (uint16_t*)heap_caps_malloc(buf_size * sizeof(uint16_t), MALLOC_CAP_DMA);
    
    if (!color_buf) return;

    for (size_t i = 0; i < buf_size; i++) {
        color_buf[i] = color;
    }

    for (int y = 0; y < Config::SCREEN_HEIGHT; y += lines) {
        int h = (y + lines > Config::SCREEN_HEIGHT) ? (Config::SCREEN_HEIGHT - y) : lines;
        drawBitmap(0, y, Config::SCREEN_WIDTH, h, color_buf);
    }
    
    heap_caps_free(color_buf);
}
