/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * touchSensor Library - supports TTP223, XPT2046 Touchscreen, and momentary switches
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 */

#pragma once
#include <Arduino.h>

#if defined(DISPLAY_CYD)
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#endif

class touchSensor {
  private:
    uint8_t _pin;
    bool lastState = LOW;
    bool currentState = LOW;
    unsigned long lastDebounceMs = 0;
    unsigned long debounceDelay = 60;
    unsigned long touchStartedMs = 0;
    bool shortTapDetected = false;
    bool longTapDetected = false;
    unsigned long longTapMs = 800;
    unsigned long lastTapTime = 0;
#if defined(DISPLAY_CYD)
    int lastTouchX = -1;
    int lastTouchY = -1;
#endif

  public:
    touchSensor(uint8_t pin = 0);
#if defined(DISPLAY_CYD)
    void begin();
    int getTouchX() const { return lastTouchX; }
    int getTouchY() const { return lastTouchY; }
#endif
    void updateTouchState();
    bool isTouched();
    bool wasShortTapped();
    bool wasLongTapped();
    int secsSinceLastTap();
    void setLongTapTime(unsigned long ms);
    void setDebounceTime(unsigned long ms);
};
