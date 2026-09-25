/*
 * Departures Board (c) 2025-2026 Gadec Software & Contributors
 *
 * cydDisplay.cpp - Global display state definitions for CYD
 */

#if defined(DISPLAY_CYD)
#include "cydDisplay.h"

TFT_eSPI cyd_tft = TFT_eSPI();
CydColorScheme cyd_current_color_scheme = CYD_COLOR_AMBER;
CydScaleMode cyd_current_scale_mode = CYD_SCALE_FULLSCREEN;
uint16_t cyd_fg_color = 0xFD80; // Rail amber (255, 176, 0)
uint16_t cyd_bg_color = 0x0000; // Deep Black
uint8_t cyd_current_brightness = 255;
#endif
