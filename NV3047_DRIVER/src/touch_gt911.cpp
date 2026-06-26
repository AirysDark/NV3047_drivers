#include "touch.h"
#include <Wire.h>

#define GT911_ADDR 0x5D

static bool touch_ready = false;

void touch_init(void)
{
    Wire.begin(21, 22);
    Wire.setClock(400000);

    delay(100);

    Wire.beginTransmission(GT911_ADDR);
    if (Wire.endTransmission() == 0) {
        touch_ready = true;
    }
}

static void gt911_read(int *x, int *y, bool *pressed)
{
    *pressed = false;
    *x = 0;
    *y = 0;

    if (!touch_ready) return;

    uint8_t reg[2] = {0x81, 0x4E};

    Wire.beginTransmission(GT911_ADDR);
    Wire.write(reg, 2);
    Wire.endTransmission(false);

    Wire.requestFrom(GT911_ADDR, 8);

    if (Wire.available() < 8) return;

    uint8_t data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = Wire.read();
    }

    uint8_t points = data[0] & 0x0F;
    if (points == 0) return;

    *x = data[1] | (data[2] << 8);
    *y = data[3] | (data[4] << 8);

    *pressed = true;

    uint8_t clear[3] = {0x81, 0x4E, 0x00};
    Wire.beginTransmission(GT911_ADDR);
    Wire.write(clear, 3);
    Wire.endTransmission();
}

bool touch_read(touch_point_t *tp)
{
    int x, y;
    bool pressed;

    gt911_read(&x, &y, &pressed);

    if (!pressed) {
        tp->pressed = false;
        return false;
    }

    // SCALE TO YOUR 320x240 DISPLAY
    tp->x = x * 320 / 4095;
    tp->y = y * 240 / 4095;
    tp->pressed = true;

    return true;
}