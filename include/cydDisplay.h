/*
 * Departures Board (c) 2025-2026 Gadec Software & Contributors
 *
 * cydDisplay.h - Display Driver for ESP32 Cheap Yellow Display (ESP32-2432S028R)
 * Provides high-speed rendering of U8g2 graphics to the onboard 320x240 ILI9341/ST7789 TFT.
 *
 * Supports:
 *  - Native 320x240 CYD canvas
 *  - Configurable UK station color palettes (Amber, White, Yellow, Green, Orange, Cyan)
 *  - Smooth LEDC Backlight PWM brightness control on GPIO 21
 */

#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <U8g2lib.h>
#include <esp_idf_version.h>

// Color schemes for departures board
enum CydColorScheme {
  CYD_COLOR_AMBER = 0,   // Classic UK National Rail Amber (255, 176, 0)
  CYD_COLOR_WHITE = 1,   // Modern UK station Crisp White (245, 245, 255)
  CYD_COLOR_YELLOW = 2,  // High-contrast Dot-Matrix Yellow (255, 220, 0)
  CYD_COLOR_GREEN = 3,   // London Underground Emerald (0, 230, 100)
  CYD_COLOR_ORANGE = 4,  // Deep Rail Orange (255, 110, 0)
  CYD_COLOR_CYAN = 5     // Ice Cyan (0, 230, 255)
};

// Legacy scale values retained for configuration compatibility. CYD now uses
// the native 320x240 layout for either value.
enum CydScaleMode {
  CYD_SCALE_FULLSCREEN = 0,
  CYD_SCALE_CENTERED = 1
};

// Backlight GPIO pin on CYD
#define CYD_TFT_BL 21
#define CYD_PWM_CHANNEL 0
#define CYD_PWM_FREQ 5000
#define CYD_PWM_RES 8

// Display state
extern TFT_eSPI cyd_tft;
extern CydColorScheme cyd_current_color_scheme;
extern CydScaleMode cyd_current_scale_mode;
extern uint16_t cyd_fg_color;
extern uint16_t cyd_bg_color;
extern uint8_t cyd_current_brightness;

#define CYD_NATIVE_WIDTH 320
#define CYD_NATIVE_HEIGHT 240
#define CYD_NATIVE_X_OFFSET ((320 - CYD_NATIVE_WIDTH) / 2)
#define CYD_NATIVE_Y_OFFSET ((240 - CYD_NATIVE_HEIGHT) / 2)
#define CYD_NATIVE_TILE_WIDTH (CYD_NATIVE_WIDTH / 8)
#define CYD_NATIVE_TILE_HEIGHT (CYD_NATIVE_HEIGHT / 8)

// Helper to calculate 16-bit RGB565 color for a scheme
inline uint16_t cyd_get_palette_color(CydColorScheme scheme) {
  switch (scheme) {
    case CYD_COLOR_AMBER:  return cyd_tft.color565(255, 176, 0);   // Classic Rail Amber
    case CYD_COLOR_WHITE:  return cyd_tft.color565(245, 245, 255); // Crisp White
    case CYD_COLOR_YELLOW: return cyd_tft.color565(255, 225, 0);   // Dot-matrix Yellow
    case CYD_COLOR_GREEN:  return cyd_tft.color565(0, 230, 100);   // Tube Emerald
    case CYD_COLOR_ORANGE: return cyd_tft.color565(255, 100, 0);   // Deep Orange
    case CYD_COLOR_CYAN:   return cyd_tft.color565(0, 230, 255);   // Cyan
    default:               return cyd_tft.color565(255, 176, 0);
  }
}

// Backlight control functions
inline void cyd_init_backlight() {
  pinMode(CYD_TFT_BL, OUTPUT);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  ledcAttach(CYD_TFT_BL, CYD_PWM_FREQ, CYD_PWM_RES);
  ledcWrite(CYD_TFT_BL, 255);
#else
  ledcSetup(CYD_PWM_CHANNEL, CYD_PWM_FREQ, CYD_PWM_RES);
  ledcAttachPin(CYD_TFT_BL, CYD_PWM_CHANNEL);
  ledcWrite(CYD_PWM_CHANNEL, 255);
#endif
  cyd_current_brightness = 255;
}

inline void cyd_set_backlight(uint8_t brightness) {
  cyd_current_brightness = brightness;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  ledcWrite(CYD_TFT_BL, brightness);
#else
  ledcWrite(CYD_PWM_CHANNEL, brightness);
#endif
}

// Low-level tile drawing function called by U8x8 callback
inline void cyd_draw_tiles(uint8_t tile_x, uint8_t tile_y, uint8_t cnt, const uint8_t *tile_ptr) {
  if (cnt == 0 || tile_ptr == nullptr) return;

  uint16_t line_buffer[CYD_NATIVE_WIDTH];
  int width = cnt * 8;

  cyd_tft.startWrite();
  cyd_tft.setAddrWindow(CYD_NATIVE_X_OFFSET + tile_x * 8, CYD_NATIVE_Y_OFFSET + tile_y * 8, width, 8);

  for (uint8_t vy = 0; vy < 8; vy++) {
    uint8_t bit_mask = (1 << vy);
    for (uint8_t c = 0; c < cnt; c++) {
      const uint8_t *tptr = tile_ptr + (c * 8);
      uint16_t *dest = line_buffer + (c * 8);
      for (uint8_t sx = 0; sx < 8; sx++) {
        dest[sx] = (tptr[sx] & bit_mask) ? cyd_fg_color : cyd_bg_color;
      }
    }
    cyd_tft.pushColors(line_buffer, width, true);
  }
  cyd_tft.endWrite();
}

// U8x8 display info structure for the native 320x240 CYD buffer
static const u8x8_display_info_t u8x8_cyd_display_info = {
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  /* post_chip_enable_wait_ns = */ 0,
  /* pre_chip_disable_wait_ns = */ 0,
  /* reset_pulse_width_ms = */ 0,
  /* post_reset_wait_ms = */ 0,
  /* sda_setup_time_ns = */ 0,
  /* sck_pulse_width_ns = */ 0,
  /* sck_clock_hz = */ 40000000UL,
  /* spi_mode = */ 0,
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 0,
  /* write_pulse_width_ns = */ 0,
  /* tile_width = */ CYD_NATIVE_TILE_WIDTH,
  /* tile_height = */ CYD_NATIVE_TILE_HEIGHT,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ CYD_NATIVE_WIDTH,
  /* pixel_height = */ CYD_NATIVE_HEIGHT
};

// Custom U8x8 callback connecting U8g2 buffer to CYD TFT
inline uint8_t u8x8_d_cyd_tft(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
  switch(msg) {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_cyd_display_info);
      break;

    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      cyd_tft.init();
      cyd_tft.setRotation(1); // Landscape 320x240
      cyd_tft.fillScreen(TFT_BLACK);
      cyd_init_backlight();
      break;

    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if (arg_int == 0) {
        cyd_set_backlight(cyd_current_brightness ? cyd_current_brightness : 255);
      } else {
        cyd_set_backlight(0);
      }
      break;

    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      cyd_set_backlight(arg_int);
      break;

    case U8X8_MSG_DISPLAY_DRAW_TILE: {
      u8x8_tile_t *t = (u8x8_tile_t *)arg_ptr;
      cyd_draw_tiles(t->x_pos, t->y_pos, t->cnt, t->tile_ptr);
      break;
    }

    default:
      return 0;
  }
  return 1;
}

// Setup routine linking U8g2 to our custom CYD driver
inline void u8g2_Setup_cyd_native(u8g2_t *u8g2, const u8g2_cb_t *rotation) {
  static uint8_t buf[CYD_NATIVE_TILE_WIDTH * CYD_NATIVE_TILE_HEIGHT * 8];
  u8g2_SetupDisplay(u8g2, u8x8_d_cyd_tft, u8x8_cad_empty, u8x8_byte_empty, u8x8_dummy_cb);
  u8g2_SetupBuffer(u8g2, buf, CYD_NATIVE_TILE_HEIGHT, u8g2_ll_hvline_vertical_top_lsb, rotation);
}

// C++ Display Class derived from U8G2
class U8G2_CYD_TFT : public U8G2 {
public:
  U8G2_CYD_TFT(const u8g2_cb_t *rotation = U8G2_R0) : U8G2() {
    u8g2_Setup_cyd_native(&u8g2, rotation);
  }

  // Preserve the original custom bitmap fonts while rendering them as crisp 2x pixels.
  u8g2_uint_t drawStr(u8g2_uint_t x, u8g2_uint_t y, const char *text) {
    return text_scale == 1 ? U8G2::drawStr(x, y, text) : u8g2_DrawStrX2(&u8g2, x, y, text);
  }

  u8g2_uint_t getStrWidth(const char *text) {
    return text_scale == 1 ? U8G2::getStrWidth(text) : u8g2_GetStrWidth(&u8g2, text) * 2;
  }

  u8g2_uint_t drawStrUnscaled(u8g2_uint_t x, u8g2_uint_t y, const char *text) {
    return U8G2::drawStr(x, y, text);
  }

  u8g2_uint_t getStrWidthUnscaled(const char *text) {
    return U8G2::getStrWidth(text);
  }

  void setTextScale(uint8_t scale) {
    text_scale = scale == 1 ? 1 : 2;
  }

  uint8_t getTextScale() const {
    return text_scale;
  }

  TFT_eSPI &getTft() { return cyd_tft; }

  void setColorScheme(CydColorScheme scheme) {
    cyd_current_color_scheme = scheme;
    cyd_fg_color = cyd_get_palette_color(scheme);
  }

  CydColorScheme getColorScheme() const {
    return cyd_current_color_scheme;
  }

  void setScaleMode(CydScaleMode mode) {
    cyd_current_scale_mode = mode;
  }

  CydScaleMode getScaleMode() const {
    return cyd_current_scale_mode;
  }

  void setBacklight(uint8_t brightness) {
    cyd_set_backlight(brightness);
  }

  void clearScreen() {
    cyd_tft.fillScreen(TFT_BLACK);
    clearBuffer();
  }

  void updateDisplayArea(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th) {
    if (tw <= 32 && (ty + th) <= 8) {
      sendBuffer();
    } else {
      U8G2::updateDisplayArea(tx, ty, tw, th);
    }
  }

private:
  uint8_t text_scale = 2;
};
