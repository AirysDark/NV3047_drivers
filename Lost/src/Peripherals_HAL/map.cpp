#include "map.h"

namespace TouchMap {

    MapData processCoordinates(uint16_t x, uint16_t y, bool pressed) {
        MapData data = {};
        data.screenX = x;
        data.screenY = y;
        data.isPressed = pressed;
        data.activeZone = ZONE_NONE;

        // If finger isn't touching the display glass surface, drop out instantly
        if (!pressed) {
            return data;
        }

        // Pull thresholds cleanly from the new TouchMapSettings constants in Config.h
        uint16_t midX   = Config::TouchMapSettings::MID_X;
        uint16_t midY   = Config::TouchMapSettings::MID_Y;
        uint16_t halfW  = Config::TouchMapSettings::CENTER_DEADZONE_HALF_W;
        uint16_t halfH  = Config::TouchMapSettings::CENTER_DEADZONE_HALF_H;

        // FIXED: Dynamically matches your center dead-zone box using your new Config settings
        if (x >= (midX - halfW) && x <= (midX + halfW) && y >= (midY - halfH) && y <= (midY + halfH)) {
            data.activeZone = ZONE_CENTER;
        }
        // Map remaining outer edge zones cleanly across the quadrant crosshair lines
        else if (x < midX && y < midY) {
            data.activeZone = ZONE_TOP_LEFT;
        }
        else if (x >= midX && y < midY) {
            data.activeZone = ZONE_TOP_RIGHT;
        }
        else if (x < midX && y >= midY) {
            data.activeZone = ZONE_BOTTOM_LEFT;
        }
        else if (x >= midX && y >= midY) {
            data.activeZone = ZONE_BOTTOM_RIGHT;
        }

        return data;
    }
}
