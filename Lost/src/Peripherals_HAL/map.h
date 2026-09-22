#pragma once
#include "../Config.h"

namespace TouchMap {
    // Defines clear structural zones for the CrowPanel screen geometry
    enum ScreenZone {
        ZONE_NONE,
        ZONE_TOP_LEFT,
        ZONE_TOP_RIGHT,
        ZONE_BOTTOM_LEFT,
        ZONE_BOTTOM_RIGHT,
        ZONE_CENTER
    };

    // Tracks exactly where the user is touching and what layout segment it maps to
    struct MapData {
        uint16_t screenX;
        uint16_t screenY;
        ScreenZone activeZone;
        bool isPressed;
    };

    // Evaluates incoming coordinates and maps them cleanly to structural areas
    MapData processCoordinates(uint16_t x, uint16_t y, bool pressed);
}
