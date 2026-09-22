#include "DisplayDriver.h"

#include <driver/ledc.h>
#include <esp_heap_caps.h>

DisplayDriver::~DisplayDriver() {
    if (fill_buffer) {
        heap_caps_free(fill_buffer);
        fill_buffer = nullptr;
    }
}

bool DisplayDriver::init(esp_lcd_panel_handle_t rgb_handle) {
    if (!rgb_handle) return false;

    handle = rgb_handle;
    current_brightness = static_cast<uint8_t>(
        (static_cast<uint16_t>(Config::Display::DEFAULT_BRIGHTNESS_PERCENT) * 255U) / 100U);
    is_sleeping = false;

    if (esp_lcd_panel_reset(handle) != ESP_OK) {
        handle = nullptr;
        return false;
    }

    if (esp_lcd_panel_init(handle) != ESP_OK) {
        handle = nullptr;
        return false;
    }

    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num = LEDC_TIMER_0;
    ledc_timer.duty_resolution = LEDC_TIMER_8_BIT;
    ledc_timer.freq_hz = Config::Display::BACKLIGHT_PWM_HZ;

    if (ledc_timer_config(&ledc_timer) != ESP_OK) {
        handle = nullptr;
        return false;
    }

    ledc_channel_config_t ledc_channel = {};
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc_channel.timer_sel = LEDC_TIMER_0;
    ledc_channel.intr_type = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num = Config::PIN_BACKLIGHT;
    ledc_channel.duty = current_brightness;

    if (ledc_channel_config(&ledc_channel) != ESP_OK) {
        handle = nullptr;
        return false;
    }

    return true;
}

void DisplayDriver::setBacklight(uint8_t brightness) {
    if (!handle || is_sleeping) return;

    current_brightness = brightness;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void DisplayDriver::setBrightness(uint8_t percentage) {
    if (percentage > 100) percentage = 100;

    const uint16_t duty =
        (static_cast<uint16_t>(percentage) * 255U) / 100U;

    setBacklight(static_cast<uint8_t>(duty));
}

void DisplayDriver::sleep() {
    if (!handle || is_sleeping) return;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    is_sleeping = true;
}

void DisplayDriver::wake() {
    if (!handle || !is_sleeping) return;

    is_sleeping = false;
    setBacklight(current_brightness);
}

void DisplayDriver::drawBitmap(
    int x,
    int y,
    int w,
    int h,
    const uint16_t *bitmap) {
    if (!handle || !bitmap || w <= 0 || h <= 0) return;

    esp_lcd_panel_draw_bitmap(
        handle,
        x,
        y,
        x + w,
        y + h,
        const_cast<uint16_t*>(bitmap));
}

void DisplayDriver::fillScreen(uint16_t color) {
    if (!handle) return;

    const size_t pixel_count =
        static_cast<size_t>(Config::SCREEN_WIDTH) *
        Config::Display::FILL_BUFFER_LINES;

    if (!fill_buffer) {
        fill_buffer = static_cast<uint16_t*>(
            heap_caps_malloc(
                pixel_count * sizeof(uint16_t),
                MALLOC_CAP_DMA));

        if (!fill_buffer) return;
    }

    for (size_t i = 0; i < pixel_count; ++i) {
        fill_buffer[i] = color;
    }

    for (int y = 0;
         y < Config::SCREEN_HEIGHT;
         y += static_cast<int>(Config::Display::FILL_BUFFER_LINES)) {

        const int remaining = Config::SCREEN_HEIGHT - y;
        const int h =
            remaining < static_cast<int>(Config::Display::FILL_BUFFER_LINES)
                ? remaining
                : static_cast<int>(Config::Display::FILL_BUFFER_LINES);

        drawBitmap(0, y, Config::SCREEN_WIDTH, h, fill_buffer);
    }
}
