#include "ui.h"
#include "ui_screen.h"

void ui_init(void) {}

void ui_tick(ui_event_t *event)
{
    if (event)
        ui_screen_event(event->x, event->y, event->pressed);
}

void ui_draw(void)
{
    ui_screen_draw();
}
