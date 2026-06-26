#include "ui_button.h"
#include <stdio.h>

// ======================================================
// DRAW (placeholder)
// ======================================================

static void draw(ui_widget_t *w)
{
    // placeholder draw logic
}

// ======================================================
// EVENT HANDLER (FIXED TYPES)
// ======================================================

static void event(ui_widget_t *w, int16_t x, int16_t y, bool pressed)
{
    if (pressed &&
        x >= w->x && x <= (w->x + w->w) &&
        y >= w->y && y <= (w->y + w->h))
    {
        w->state = 1;
    }
    else
    {
        w->state = 0;
    }
}

// ======================================================
// INIT
// ======================================================

void ui_button_init(ui_widget_t *w)
{
    w->type = UI_WIDGET_BUTTON;

    w->draw = draw;
    w->event = event;

    w->state = 0;
}