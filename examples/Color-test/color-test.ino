#include <NV3047_Driver.h>


NV3047_Driver myScreen;

void setup() {
    Serial.begin(115200);
    // Give the hardware a moment to stabilize after power-on
    delay(1000);
    Serial.println("Initializing Bare-Metal Hardware...");
    
    // Boot the hardware
    myScreen.begin();
    
    // Initial state: BLACK (using Config namespace)
    myScreen.fillScreen(Config::COLOR_BLACK);
    myScreen.setBrightness(128); 
    
    Serial.println("Hardware Init Complete. Touch screen to cycle colors.");
}

void loop() {
    static int colorState = 0; // 0:Black, 1:Red, 2:Green, 3:Blue, 4:White
    uint16_t touchX, touchY;
    
    if (myScreen.getTouch(touchX, touchY)) {
        Serial.print("Touch detected! X: ");
        Serial.print(touchX);
        Serial.print(" Y: ");
        Serial.println(touchY);
        
        colorState = (colorState + 1) % 5; 
        
        switch(colorState) {
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
            case 4: 
                myScreen.fillScreen(Config::COLOR_WHITE);
                Serial.println("Displaying: WHITE");
                break;
        }
        
        // Debounce to prevent rapid cycling
        delay(500); 
    }
    
    delay(20);
}
