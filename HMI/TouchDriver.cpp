#include "TouchDriver.h"

// ============================================
// TOUCH PINS
// ============================================

#define TOUCH_SCK   12
#define TOUCH_MISO  13
#define TOUCH_MOSI  11
#define TOUCH_CS    0
#define TOUCH_INT   36

// ============================================
// GLOBAL INSTANCE
// ============================================

TouchDriver Touch;

// ============================================
// CONSTRUCTOR
// ============================================

TouchDriver::TouchDriver()
{
    ts = nullptr;

    touchX = 0;
    touchY = 0;

    rawX = 0;
    rawY = 0;

    touched = false;
}

// ============================================
// INIT
// ============================================

void TouchDriver::begin()
{
    SPI.begin(
        TOUCH_SCK,
        TOUCH_MISO,
        TOUCH_MOSI,
        TOUCH_CS
    );

    ts = new XPT2046_Touchscreen(
        TOUCH_CS,
        TOUCH_INT
    );

    ts->begin();

    // Keep rotation at 1
    // because your X/Y orientation
    // already matches correctly
    ts->setRotation(1);
}

// ============================================
// UPDATE
// ============================================

bool TouchDriver::update()
{
    if (!ts)
    {
        touched = false;
        return false;
    }

    touched = ts->touched();

    if (!touched)
    {
        return false;
    }

    TS_Point p = ts->getPoint();

    rawX = p.x;
    rawY = p.y;

    // ========================================
    // MAP TOUCH
    // ========================================

    // LEFT / RIGHT CORRECT
    touchX = map(
        rawY,
        Y_MIN,
        Y_MAX,
        0,
        479
    );

// NORMAL Y AXIS
    touchY = map(
        rawX,
        X_MIN,
        X_MAX,
        0,
        271
    );

    // ========================================
    // CLAMP
    // ========================================

    if (touchX > 479)
    {
        touchX = 479;
    }

    if (touchY > 271)
    {
        touchY = 271;
    }

    return true;
}

// ============================================
// GETTERS
// ============================================

uint16_t TouchDriver::getX()
{
    return touchX;
}

uint16_t TouchDriver::getY()
{
    return touchY;
}

bool TouchDriver::isTouched()
{
    return touched;
}