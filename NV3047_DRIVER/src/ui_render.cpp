#include "ui_render.h"

// external framebuffer from display driver
extern uint16_t *framebuffer;

#define W 320

void ui_fill(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            framebuffer[(y + j) * W + (x + i)] = color;
        }
    }
}
