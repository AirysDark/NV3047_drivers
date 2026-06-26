#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// UI COLOR THEME (RGB565)
// ======================================================

#define UI_BG       0x0000  // Black background
#define UI_FG       0xFFFF  // White foreground (text/icons)
#define UI_ACCENT   0x07E0  // Green accent (highlight / selection)

#define UI_DARK     0x18E3  // Dark gray
#define UI_LIGHT    0xC618  // Light gray

#define UI_ERROR    0xF800  // Red (error / alert)
#define UI_WARN     0xFFE0  // Yellow (warning)
#define UI_OK       0x07E0  // Green (success)

#ifdef __cplusplus
}
#endif