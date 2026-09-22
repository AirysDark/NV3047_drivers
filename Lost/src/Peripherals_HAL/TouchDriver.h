#pragma once
#include "../Config.h"
#include <driver/spi_master.h>

// --- REQUIRED OPERATING SYSTEM HEADERS ---
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// =========================================================================
//   ADDED: MAP.H LINKED DIRECTLY INTO THE TOUCH FILE STRUCTURE
// =========================================================================
#include "map.h" 
// =========================================================================

class TouchDriver {
private:
    spi_device_handle_t spi_handle;
    uint16_t transfer16(uint8_t cmd);
    
    // Low-level pointer linking this driver instance straight to our bare-metal heap memory buffer
    uint8_t* sharedTouchBuffer;

    // State machine trackers to handle clean edge-debouncing
    bool was_pressed_last_frame;
    bool is_pressed_current_frame;

public:
    // FIXED SIGNATURE: Accepts the secondary memory pointer with a safe default parameter
    // to cleanly satisfy the master library allocation handshake without breaking old links!
    bool init(spi_device_handle_t handle, uint8_t* touchBuffer = nullptr);
    
    // Enhanced reader: Automatically handles noise filtering and axis calibration
    bool getTouch(uint16_t &x, uint16_t &y);
    
    // Quick hardware-level check to see if the panel is currently pressed
    bool isPressed();

    // --- EVENT TRIGGER EVALUATORS ---
    bool isNewPress();  // True only on the initial frame contact lands on the glass
    bool isReleased();  // True only on the exact frame the finger is lifted away
};
