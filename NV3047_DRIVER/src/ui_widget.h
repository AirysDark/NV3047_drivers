#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// WIDGET TYPES
// ======================================================

typedef enum {
    UI_WIDGET_BUTTON = 0,
    UI_WIDGET_LABEL,
    UI_WIDGET_CONTAINER
} ui_widget_type_t;

// ======================================================
// FORWARD DECL
// ======================================================

typedef struct ui_widget ui_widget_t;

// ======================================================
// FUNCTION POINTER TYPES (IMPORTANT FIX)
// ======================================================

typedef void (*ui_draw_fn_t)(ui_widget_t *self);

typedef void (*ui_event_fn_t)(
    ui_widget_t *self,
    int16_t x,
    int16_t y,
    bool pressed
);

// ======================================================
// BASE WIDGET STRUCTURE
// ======================================================

struct ui_widget {

    ui_widget_type_t type;

    // Geometry
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;

    // State (bitmask ready for expansion)
    uint8_t state;

    // Function pointers (polymorphism)
    ui_draw_fn_t draw;
    ui_event_fn_t event;

    // User data (button text, icon, etc.)
    void *data;
};

#ifdef __cplusplus
}
#endif