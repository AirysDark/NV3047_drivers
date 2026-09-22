#include <NV3047_Driver.h>

NV3047 hardware;
NV3047_Driver myScreen;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Initializing NV3047 hardware...");

    if (!myScreen.begin(&hardware)) {
        Serial.println("NV3047 initialization FAILED.");
        while (true) {
            delay(1000);
        }
    }

    myScreen.setBrightness(80);
    myScreen.fillScreen(Config::COLOR_BLACK);

    Serial.println("Hardware init complete. Touch screen to cycle colors.");
}

void loop() {
    static int colorState = 0;
    static bool touchLatched = false;

    uint16_t touchX = 0;
    uint16_t touchY = 0;
    const bool touched = myScreen.getTouch(touchX, touchY);

    if (touched && !touchLatched) {
        touchLatched = true;

        Serial.print("Touch X: ");
        Serial.print(touchX);
        Serial.print(" Y: ");
        Serial.println(touchY);

        colorState = (colorState + 1) % 5;

        switch (colorState) {
            case 0:
                myScreen.fillScreen(Config::COLOR_BLACK);
                Serial.println("Displaying: BLACK");
                break;
            case 1:
                myScreen.fillScreen(Config::COLOR_RED);
                Serial.println("Displaying: RED");
                break;
            case 2:
                myScreen.fillScreen(Config::COLOR_GREEN);
                Serial.println("Displaying: GREEN");
                break;
            case 3:
                myScreen.fillScreen(Config::COLOR_BLUE);
                Serial.println("Displaying: BLUE");
                break;
            default:
                myScreen.fillScreen(Config::COLOR_WHITE);
                Serial.println("Displaying: WHITE");
                break;
        }
    }

    if (!touched) {
        touchLatched = false;
    }

    delay(5);
}
