#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ======================================================
// TOUCH DATA STRUCTURE
// ======================================================

typedef struct {
    int16_t x;
    int16_t y;
    bool pressed;
} touch_point_t;

// ======================================================
// TOUCH DRIVER API
// ======================================================

/**
 * @brief Initialise touch controller (GT911 or compatible)
 */
void touch_init(void);

/**
 * @brief Read latest touch state
 * @param tp Pointer to touch_point_t struct
 * @return true if touch data is valid, false otherwise
 */
bool touch_read(touch_point_t *tp);

/**
 * @brief Check if screen is currently being touched
 */
bool touch_is_pressed(void);

/**
 * @brief Get raw X coordinate
 */
int16_t touch_get_x(void);

/**
 * @brief Get raw Y coordinate
 */
int16_t touch_get_y(void);

#ifdef __cplusplus
}
#endif