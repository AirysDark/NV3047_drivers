#include <Arduino.h>
#include "NV3047_Driver.h"

NV3047 panel;

// Asset properties
int16_t sync_bar_x = 40;
int16_t sync_bar_dir = 3; 
const int16_t BAR_WIDTH = 15;

int16_t box_x = 200;
int16_t box_y = 100;
int16_t box_dx = 2;
int16_t box_dy = 1;
const int16_t BOX_SIZE = 45;

void setup() {
    Serial.begin(115200);
    if (!panel.init()) {
        while (1) delay(1000);
    }
    panel.getDisplay().setBrightness(80);
}

void loop() {
    Framebuffer& canvas = panel.getCanvas();
    TouchDriver& touch = panel.getTouch();
    DisplayDriver& display = panel.getDisplay();
    
    uint16_t touch_x = 0, touch_y = 0;
    bool is_currently_touched = touch.getTouch(touch_x, touch_y);

    canvas.clear(Config::COLOR_BLACK);

    // Static Border Grid
    for (int16_t offset = 0; offset < 36; offset += 6) {
        canvas.drawRect(offset, offset, Config::SCREEN_WIDTH - (offset * 2), Config::SCREEN_HEIGHT - (offset * 2), Config::COLOR_LIGHT_GREY);
    }

    if (is_currently_touched) {
        if (touch.isNewPress()) display.setBrightness(100);
        canvas.drawRect(touch_x - 15, touch_y - 15, 30, 30, Config::COLOR_RED);
        canvas.drawHLine(touch_x - 25, touch_y, 50, Config::COLOR_WHITE);
        canvas.drawVLine(touch_x, touch_y - 25, 50, Config::COLOR_WHITE);
        canvas.fillRect(box_x, box_y, BOX_SIZE, BOX_SIZE, Config::COLOR_BLUE);
    } else {
        if (touch.isReleased()) display.setBrightness(80);

        // Render pacing bar
        canvas.fillRect(sync_bar_x, 38, BAR_WIDTH, Config::SCREEN_HEIGHT - 76, Config::COLOR_WHITE);
        sync_bar_x += sync_bar_dir;
        if (sync_bar_x <= 38 || sync_bar_x >= (Config::SCREEN_WIDTH - BAR_WIDTH - 38)) {
            sync_bar_dir = -sync_bar_dir; 
        }

        // Render bouncing blue box
        canvas.fillRect(box_x, box_y, BOX_SIZE, BOX_SIZE, Config::COLOR_BLUE);
        box_x += box_dx; box_y += box_dy;
        if (box_x <= 38 || box_x >= (Config::SCREEN_WIDTH - BOX_SIZE - 38))   box_dx = -box_dx;
        if (box_y <= 38 || box_y >= (Config::SCREEN_HEIGHT - BOX_SIZE - 38))  box_dy = -box_dy;
    }

    // The class handles the microsecond cadence timing loop automatically right here!
    canvas.swap();
}
