#include "TouchDriver.h"
#include <driver/gpio.h>
#include <stdlib.h> // Required for abs()

// FIXED IMPLEMENTATION SIGNATURE: Matches your updated TouchDriver.h header perfectly
// and accepts your shielded internal RAM buffer memory slot cleanly!
bool TouchDriver::init(spi_device_handle_t handle, uint8_t* touchBuffer) {
    spi_handle = handle;
    sharedTouchBuffer = touchBuffer; // Injects your dynamic bare-metal heap memory array
    
    was_pressed_last_frame = false;
    is_pressed_current_frame = false;
    
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << Config::PIN_TOUCH_IRQ);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    
    return (spi_handle != nullptr);
}

uint16_t TouchDriver::transfer16(uint8_t cmd) {
    if (!spi_handle) return 0;

    uint8_t tx_data[3] = {cmd, 0x00, 0x00};
    uint8_t rx_data[3] = {0, 0, 0};
    
    spi_transaction_t t = {};
    t.length = 24;
    t.tx_buffer = tx_data;
    t.rx_buffer = rx_data;
    
    spi_device_polling_transmit(spi_handle, &t);
    
    // FOR 24-BIT BIT-ALIGNMENT ORDER MATRICES: 
    // Combining the raw bits directly out of index 1 and index 2 via an explicit shift-right (>> 3)
    // strips the trailing clock delays and resolves the 480 / 65535 bitwise register overflows!
    uint16_t raw_value = ((uint16_t)(rx_data[1] & 0x7F) << 5) | (rx_data[2] >> 3);
    
    return raw_value & 0x0FFF;
}

bool TouchDriver::isPressed() {
    // Active LOW touch line: returns true if contact is active
    return gpio_get_level((gpio_num_t)Config::PIN_TOUCH_IRQ) == 0;
}

// --- STATE TRIGGER EVALUATORS ---

bool TouchDriver::isNewPress() {
    return is_pressed_current_frame && !was_pressed_last_frame;
}

bool TouchDriver::isReleased() {
    return !is_pressed_current_frame && was_pressed_last_frame;
}

bool TouchDriver::getTouch(uint16_t &x, uint16_t &y) {
    // Save previous frame state before scanning current metrics
    was_pressed_last_frame = is_pressed_current_frame;
    is_pressed_current_frame = isPressed();

    if (!is_pressed_current_frame) {
        return false; // Hardware pin reports no contact
    }
    
    // --- SELF-THROTTLING INTERNAL HARDWARE GATE ---
    vTaskDelay(2);

    // Multi-sample filtering loop to reject random ADC spikes (Touch Jitter)
    uint32_t total_x = 0;
    uint32_t total_y = 0;
    constexpr int samples = 3;

    for (int i = 0; i < samples; i++) {
        uint16_t sample_x = transfer16(0x94); // Read X-axis channel cleanly
        uint16_t sample_y = transfer16(0xD4); // Read Y-axis channel cleanly
        
        if (!isPressed()) {
            is_pressed_current_frame = false;
            return false;
        }

        total_x += sample_x;
        total_y += sample_y;
    }

    uint16_t avg_raw_x = total_x / samples;
    uint16_t avg_raw_y = total_y / samples;

    // --- CROWPANEL 4.3 CLAMPING: Pull thresholds dynamically from Config.h ---
    uint16_t xMin = Config::TouchCalibration::RAW_X_MIN;
    uint16_t xMax = Config::TouchCalibration::RAW_X_MAX;
    uint16_t yMin = Config::TouchCalibration::RAW_Y_MIN;
    uint16_t yMax = Config::TouchCalibration::RAW_Y_MAX;

    if (avg_raw_x < xMin) avg_raw_x = xMin;
    if (avg_raw_x > xMax) avg_raw_x = xMax;
    if (avg_raw_y < yMin) avg_raw_y = yMin;
    if (avg_raw_y > yMax) avg_raw_y = yMax;

    // --- NATIVE CROWPANEL ORIENTATION MATH MATRIX ---
    uint32_t mapped_x = ((uint32_t)(avg_raw_x - xMin) * Config::SCREEN_WIDTH) / (xMax - xMin);
    uint32_t mapped_y = ((uint32_t)(avg_raw_y - yMin) * Config::SCREEN_HEIGHT) / (yMax - yMin);

    x = (uint16_t)mapped_x;
    y = (uint16_t)(Config::SCREEN_HEIGHT - 1 - mapped_y);
    
    return true;
}
