#include "TouchDriver.h"

#include <driver/gpio.h>

namespace {

uint16_t median3(uint16_t a, uint16_t b, uint16_t c) {
    if (a > b) {
        uint16_t t = a;
        a = b;
        b = t;
    }
    if (b > c) {
        uint16_t t = b;
        b = c;
        c = t;
    }
    if (a > b) {
        uint16_t t = a;
        a = b;
        b = t;
    }
    return b;
}

} // namespace

bool TouchDriver::init(spi_device_handle_t handle) {
    spi_handle = handle;
    was_pressed_last_frame = false;
    is_pressed_current_frame = false;

    if (!spi_handle) return false;

    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << Config::PIN_TOUCH_IRQ);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;

    return gpio_config(&io_conf) == ESP_OK;
}

uint16_t TouchDriver::transfer16(uint8_t cmd) {
    if (!spi_handle) return 0;

    uint8_t tx_data[3] = {cmd, 0x00, 0x00};
    uint8_t rx_data[3] = {0, 0, 0};

    spi_transaction_t transaction = {};
    transaction.length = 24;
    transaction.tx_buffer = tx_data;
    transaction.rx_buffer = rx_data;

    if (spi_device_polling_transmit(spi_handle, &transaction) != ESP_OK) {
        return 0;
    }

    // This byte alignment is intentionally retained because it is the verified
    // working layout for this panel under Arduino-ESP32 core 2.0.17.
    const uint16_t high_byte = static_cast<uint16_t>(rx_data[0]) << 8;
    const uint16_t low_byte = static_cast<uint16_t>(rx_data[1]);
    return static_cast<uint16_t>((high_byte | low_byte) >> 3);
}

bool TouchDriver::readRawPair(uint16_t &raw_x, uint16_t &raw_y) {
    if (!spi_handle || !isPressed()) return false;

    // These command bytes are intentionally retained from the verified hardware setup.
    raw_x = transfer16(0x94);
    raw_y = transfer16(0xD4);

    return isPressed();
}

bool TouchDriver::isPressed() {
    return gpio_get_level(static_cast<gpio_num_t>(Config::PIN_TOUCH_IRQ)) == 0;
}

bool TouchDriver::isNewPress() {
    return is_pressed_current_frame && !was_pressed_last_frame;
}

bool TouchDriver::isReleased() {
    return !is_pressed_current_frame && was_pressed_last_frame;
}

bool TouchDriver::getRawTouch(uint16_t &raw_x, uint16_t &raw_y) {
    if (!isPressed()) return false;
    vTaskDelay(1);
    return readRawPair(raw_x, raw_y);
}

bool TouchDriver::getTouch(uint16_t &x, uint16_t &y) {
    was_pressed_last_frame = is_pressed_current_frame;
    is_pressed_current_frame = isPressed();

    if (!is_pressed_current_frame) {
        return false;
    }

    vTaskDelay(1);

    uint16_t sample_x[3] = {0, 0, 0};
    uint16_t sample_y[3] = {0, 0, 0};

    for (int i = 0; i < 3; ++i) {
        if (!readRawPair(sample_x[i], sample_y[i])) {
            is_pressed_current_frame = false;
            return false;
        }
    }

    uint16_t raw_x = median3(sample_x[0], sample_x[1], sample_x[2]);
    uint16_t raw_y = median3(sample_y[0], sample_y[1], sample_y[2]);

    if (raw_x < RAW_X_MIN) raw_x = RAW_X_MIN;
    if (raw_x > RAW_X_MAX) raw_x = RAW_X_MAX;
    if (raw_y < RAW_Y_MIN) raw_y = RAW_Y_MIN;
    if (raw_y > RAW_Y_MAX) raw_y = RAW_Y_MAX;

    uint32_t mapped_x =
        (static_cast<uint32_t>(raw_x - RAW_X_MIN) * (Config::SCREEN_WIDTH - 1U)) /
        (RAW_X_MAX - RAW_X_MIN);

    uint32_t mapped_y =
        (static_cast<uint32_t>(raw_y - RAW_Y_MIN) * (Config::SCREEN_HEIGHT - 1U)) /
        (RAW_Y_MAX - RAW_Y_MIN);

    if (mapped_x >= Config::SCREEN_WIDTH) {
        mapped_x = Config::SCREEN_WIDTH - 1U;
    }
    if (mapped_y >= Config::SCREEN_HEIGHT) {
        mapped_y = Config::SCREEN_HEIGHT - 1U;
    }

    x = static_cast<uint16_t>(mapped_x);
    y = static_cast<uint16_t>((Config::SCREEN_HEIGHT - 1U) - mapped_y);
    return true;
}
