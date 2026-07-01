#include "TouchDriver.h"
#include <driver/gpio.h>
#include <stdlib.h> // Required for abs()

bool TouchDriver::init(spi_device_handle_t handle) {
    spi_handle = handle;
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
    
    // FIXED FOR 24-BIT SPI BUS ALIGNMENT: 
    // Shifts the read offsets to index 0 and index 1 to capture the raw bits cleanly
    uint16_t high_byte = ((uint16_t)(rx_data[0] & 0xFF)) << 8;
    uint16_t low_byte  = (uint16_t)(rx_data[1] & 0xFF);
    
    return (high_byte | low_byte) >> 3;
}

bool TouchDriver::isPressed() {
    // Active LOW touch line: returns true if contact is active
    return gpio_get_level((gpio_num_t)Config::PIN_TOUCH_IRQ) == 0;
}

// --- STATE TRIGGER EVALUATORS ---

bool TouchDriver::isNewPress() {
    // True ONLY on the exact frame the contact goes from unpressed to pressed
    return is_pressed_current_frame && !was_pressed_last_frame;
}

bool TouchDriver::isReleased() {
    // True ONLY on the exact frame the contact lifts away from the glass surface
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
    vTaskDelay(1);

    // Multi-sample filtering loop to reject random ADC spikes (Touch Jitter)
    uint32_t total_x = 0;
    uint32_t total_y = 0;
    constexpr int samples = 3;

    for (int i = 0; i < samples; i++) {
        // FIXED FOR CROWPANEL 4.3: Injects the true, factory-verified control command bytes!
        // 0x94 selects the 12-bit X-channel, 0xD4 selects the 12-bit Y-channel.
        uint16_t sample_x = transfer16(0x94); // Read X-axis channel cleanly
        uint16_t sample_y = transfer16(0xD4); // Read Y-axis channel cleanly
        
        // Sanity Check: If touch is lifted mid-read, drop the frame state evaluations
        if (!isPressed()) {
            is_pressed_current_frame = false;
            return false;
        }

        total_x += sample_x;
        total_y += sample_y;
    }

    uint16_t avg_raw_x = total_x / samples;
    uint16_t avg_raw_y = total_y / samples;

    // --- CROWPANEL 4.3 MATRIX CALIBRATION & CLAMPING ---
    if (avg_raw_x < RAW_X_MIN) avg_raw_x = RAW_X_MIN;
    if (avg_raw_x > RAW_X_MAX) avg_raw_x = RAW_X_MAX;
    if (avg_raw_y < RAW_Y_MIN) avg_raw_y = RAW_Y_MIN;
    if (avg_raw_y > RAW_Y_MAX) avg_raw_y = RAW_Y_MAX;

    // --- NATIVE CROWPANEL ORIENTATION MATH MATRIX ---
    // 1. Map raw sensors over the raw screen height and width coordinates
    uint32_t mapped_x = ((uint32_t)(avg_raw_x - RAW_X_MIN) * Config::SCREEN_WIDTH) / (RAW_X_MAX - RAW_X_MIN);
    uint32_t mapped_y = ((uint32_t)(avg_raw_y - RAW_Y_MIN) * Config::SCREEN_HEIGHT) / (RAW_Y_MAX - RAW_Y_MIN);

    // 2. Swap the coordinate layout and invert the true vertical axis 
    // to correctly mirror the physical alignment of the display panel glass matrix.
    x = (uint16_t)mapped_x;
    y = (uint16_t)(Config::SCREEN_HEIGHT - 1 - mapped_y);
    
    return true;
}
