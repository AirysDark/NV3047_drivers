#include "ui_screen.h"
#include <stddef.h>

// ======================================
// ACTIVE SCREEN STATE
// ======================================

static ui_screen_t *current = NULL;

// ======================================
// SCREEN CONTROL
// ======================================

void ui_screen_set(ui_screen_t *screen)
{
    current = screen;
}

// ======================================
// SCREEN RENDER
// ======================================

void ui_screen_draw(void)
{
    if (current != NULL && current->draw != NULL)
    {
        current->draw();
    }
}

// ======================================
// SCREEN INPUT EVENT
// ======================================

void ui_screen_event(int16_t x, int16_t y, bool pressed)
{
    if (current != NULL && current->event != NULL)
    {
        current->event(x, y, pressed);
    }
}