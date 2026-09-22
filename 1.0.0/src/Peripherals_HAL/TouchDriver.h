#pragma once
#include "../Config.h"
#include <driver/spi_master.h>

class TouchDriver {
private:
    spi_device_handle_t spi_handle;
    uint16_t transfer16(uint8_t cmd);
    
    // Internal hardware-calibrated boundaries for the CrowPanel 4.3 resistive layer
    static constexpr uint16_t RAW_X_MIN = 300;
    static constexpr uint16_t RAW_X_MAX = 3850;
    static constexpr uint16_t RAW_Y_MIN = 250;
    static constexpr uint16_t RAW_Y_MAX = 3750;

    // State machine trackers to handle clean edge-debouncing
    bool was_pressed_last_frame;
    bool is_pressed_current_frame;

public:
    bool init(spi_device_handle_t handle);
    
    // Enhanced reader: Automatically handles noise filtering and axis calibration
    bool getTouch(uint16_t &x, uint16_t &y);
    
    // Quick hardware-level check to see if the panel is currently pressed
    bool isPressed();

    // --- EVENT TRIGGER EVALUATORS ---
    bool isNewPress();  // True only on the initial frame contact lands on the glass
    bool isReleased();  // True only on the exact frame the finger is lifted away
};
