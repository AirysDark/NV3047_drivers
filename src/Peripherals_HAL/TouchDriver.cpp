#include "TouchDriver.h"

#include <Arduino.h>

namespace {

uint16_t medianSamples(uint16_t* values, size_t count) {
    for (size_t i = 1; i < count; ++i) {
        const uint16_t key = values[i];
        size_t j = i;

        while (j > 0 && values[j - 1] > key) {
            values[j] = values[j - 1];
            --j;
        }

        values[j] = key;
    }

    return values[count / 2U];
}

} // namespace

bool TouchDriver::init(spi_device_handle_t handle) {
    spi_handle = handle;
    was_pressed_last_frame = false;
    is_pressed_current_frame = false;

    pinMode(Config::PIN_TOUCH_IRQ, INPUT_PULLUP);

    return spi_handle != nullptr;
}

bool TouchDriver::transfer16(uint8_t cmd, uint16_t &value) {
    value = 0;

    if (!spi_handle) {
        return false;
    }

    uint8_t tx_data[3] = {cmd, 0x00, 0x00};
    uint8_t rx_data[3] = {0, 0, 0};

    spi_transaction_t transaction = {};
    transaction.length = 24;
    transaction.tx_buffer = tx_data;
    transaction.rx_buffer = rx_data;

    if (spi_device_polling_transmit(
            spi_handle,
            &transaction) != ESP_OK) {
        return false;
    }

    const uint16_t high_byte =
        static_cast<uint16_t>(rx_data[1]) << 8;
    const uint16_t low_byte =
        static_cast<uint16_t>(rx_data[2]);

    value =
        static_cast<uint16_t>(
            ((high_byte | low_byte) >> 3) & 0x0FFFU);

    return true;
}

bool TouchDriver::readRawPair(uint16_t &raw_x, uint16_t &raw_y) {
    if (!spi_handle || !isPressed()) return false;

    if (!transfer16(Config::Touch::X_COMMAND, raw_x)) {
        return false;
    }

    if (!transfer16(Config::Touch::Y_COMMAND, raw_y)) {
        return false;
    }

    return isPressed();
}

bool TouchDriver::isPressed() {
    return digitalRead(Config::PIN_TOUCH_IRQ) == LOW;
}

bool TouchDriver::isNewPress() {
    return is_pressed_current_frame && !was_pressed_last_frame;
}

bool TouchDriver::isReleased() {
    return !is_pressed_current_frame && was_pressed_last_frame;
}

bool TouchDriver::getRawTouch(uint16_t &raw_x, uint16_t &raw_y) {
    if (!isPressed()) return false;

    if (Config::Touch::SETTLE_DELAY_TICKS > 0) {
        vTaskDelay(
            static_cast<TickType_t>(
                Config::Touch::SETTLE_DELAY_TICKS));
    }

    return readRawPair(raw_x, raw_y);
}

bool TouchDriver::getTouch(uint16_t &x, uint16_t &y) {
    was_pressed_last_frame = is_pressed_current_frame;
    is_pressed_current_frame = isPressed();

    if (!is_pressed_current_frame) {
        return false;
    }

    if (Config::Touch::SETTLE_DELAY_TICKS > 0) {
        vTaskDelay(
            static_cast<TickType_t>(
                Config::Touch::SETTLE_DELAY_TICKS));
    }

    uint16_t sample_x[Config::Touch::SAMPLE_COUNT] = {};
    uint16_t sample_y[Config::Touch::SAMPLE_COUNT] = {};

    for (size_t i = 0; i < Config::Touch::SAMPLE_COUNT; ++i) {
        if (!readRawPair(sample_x[i], sample_y[i])) {
            is_pressed_current_frame = false;
            return false;
        }
    }

    uint16_t raw_x =
        medianSamples(sample_x, Config::Touch::SAMPLE_COUNT);
    uint16_t raw_y =
        medianSamples(sample_y, Config::Touch::SAMPLE_COUNT);

    if (raw_x < Config::Touch::RAW_X_MIN) {
        raw_x = Config::Touch::RAW_X_MIN;
    }
    if (raw_x > Config::Touch::RAW_X_MAX) {
        raw_x = Config::Touch::RAW_X_MAX;
    }
    if (raw_y < Config::Touch::RAW_Y_MIN) {
        raw_y = Config::Touch::RAW_Y_MIN;
    }
    if (raw_y > Config::Touch::RAW_Y_MAX) {
        raw_y = Config::Touch::RAW_Y_MAX;
    }

    // Physical CrowPanel 4.3 touch orientation:
    //
    // XPT2046 raw Y channel -> display X axis
    // XPT2046 raw X channel -> display Y axis
    //
    // Real-panel verification confirms the raw axes are swapped relative
    // to the display axes, but neither destination axis requires inversion.
    // Keep each calibration range attached to its original raw channel.
    uint32_t mapped_x =
        (static_cast<uint32_t>(
             raw_y - Config::Touch::RAW_Y_MIN) *
         (Config::SCREEN_WIDTH - 1U)) /
        (Config::Touch::RAW_Y_MAX -
         Config::Touch::RAW_Y_MIN);

    uint32_t mapped_y =
        (static_cast<uint32_t>(
             raw_x - Config::Touch::RAW_X_MIN) *
         (Config::SCREEN_HEIGHT - 1U)) /
        (Config::Touch::RAW_X_MAX -
         Config::Touch::RAW_X_MIN);

    if (mapped_x >= Config::SCREEN_WIDTH) {
        mapped_x = Config::SCREEN_WIDTH - 1U;
    }
    if (mapped_y >= Config::SCREEN_HEIGHT) {
        mapped_y = Config::SCREEN_HEIGHT - 1U;
    }

    x = static_cast<uint16_t>(mapped_x);
    y = static_cast<uint16_t>(mapped_y);

    return true;
}
