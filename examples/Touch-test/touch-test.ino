#include <Arduino.h>
#include "NV3047_Driver.h"

NV3047 panel;

void setup() {
    Serial.begin(115200);

    if (!panel.init()) {
        Serial.println("NV3047 initialization FAILED.");
        while (true) {
            delay(1000);
        }
    }

    panel.getDisplay().setBrightness(80);
    Serial.println("Touch diagnostic ready.");
}

void loop() {
    TouchDriver& touch = panel.getTouch();

    uint16_t x = 0;
    uint16_t y = 0;

    if (touch.getTouch(x, y)) {
        Serial.print("Mapped X=");
        Serial.print(x);
        Serial.print(" Y=");
        Serial.println(y);
    }

    // Raw readings are intentionally available separately for calibration work.
    uint16_t raw_x = 0;
    uint16_t raw_y = 0;

    if (touch.getRawTouch(raw_x, raw_y)) {
        Serial.print("Raw X=");
        Serial.print(raw_x);
        Serial.print(" Y=");
        Serial.println(raw_y);
    }

    delay(20);
}
