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

#include "touchSensor.h"

#if defined(DISPLAY_CYD)

// CYD Touch Screen SPI Pins (VSPI)
#define XPT2046_IRQ  36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK  25
#define XPT2046_CS   33

static SPIClass cydTouchSpi(VSPI);
static XPT2046_Touchscreen cydTs(XPT2046_CS, XPT2046_IRQ);
static bool cydTouchBegun = false;

touchSensor::touchSensor(uint8_t pin) {
  _pin = pin;
}

void touchSensor::begin() {
  if (cydTouchBegun) return;

  pinMode(0, INPUT_PULLUP); // BOOT button (GPIO 0, active LOW)

  cydTouchSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  cydTs.begin(cydTouchSpi);
  cydTs.setRotation(1); // Landscape
  cydTouchBegun = true;
}

void touchSensor::updateTouchState() {
  if (!cydTouchBegun) begin();

  bool pressed = false;

  // Check XPT2046 touch screen
  if (cydTs.tirqTouched() && cydTs.touched()) {
    pressed = true;
    TS_Point p = cydTs.getPoint();
    lastTouchX = p.x;
    lastTouchY = p.y;
  }
  // Check onboard BOOT button (GPIO 0, active LOW)
  else if (digitalRead(0) == LOW) {
    pressed = true;
  }
  // Optional external pin if configured
  else if (_pin != 0 && _pin != 34 && digitalRead(_pin) == HIGH) {
    pressed = true;
  }

  bool state = pressed ? HIGH : LOW;

  if (state != lastState) {
    lastDebounceMs = millis();
  }

  if ((millis() - lastDebounceMs) > debounceDelay) {
    if (state != currentState) {
      currentState = state;
      if (currentState == HIGH) {
        touchStartedMs = millis();
        shortTapDetected = false;
        longTapDetected = false;
      } else {
        unsigned long touchLengthMs = millis() - touchStartedMs;
        if (touchLengthMs < longTapMs) {
          shortTapDetected = true;
          lastTapTime = millis();
        } else {
          longTapDetected = true;
          lastTapTime = millis();
        }
      }
    }
  }
  lastState = state;
}

#else

touchSensor::touchSensor(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, INPUT);
}

void touchSensor::updateTouchState() {
  bool state = digitalRead(_pin);

  if (state != lastState) {
    lastDebounceMs = millis();
  }

  if ((millis()-lastDebounceMs) > debounceDelay) {
    if (state != currentState) {
      currentState = state;
      if (currentState == HIGH) {
        touchStartedMs = millis();
        shortTapDetected = false;
        longTapDetected = false;
      } else {
        unsigned long touchLengthMs = millis()-touchStartedMs;
        if (touchLengthMs < longTapMs) {
          shortTapDetected = true;
          lastTapTime = millis();
        } else {
          longTapDetected = true;
          lastTapTime = millis();
        }
      }
    }
  }
  lastState = state;
}

#endif

bool touchSensor::isTouched() {
  return currentState;
}

bool touchSensor::wasShortTapped() {
  if (shortTapDetected) {
    shortTapDetected = false;
    return true;
  } else return false;
}

bool touchSensor::wasLongTapped() {
  if (longTapDetected) {
    longTapDetected = false;
    return true;
  } else return false;
}

int touchSensor::secsSinceLastTap() {
  if (lastTapTime == 0) return 0;
  return ((millis()-lastTapTime)/1000);
}

void touchSensor::setLongTapTime(unsigned long ms) {
  longTapMs = ms;
}

void touchSensor::setDebounceTime(unsigned long ms) {
  debounceDelay = ms;
}
