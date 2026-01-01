#pragma once
#include <Arduino.h>
#include "config.h"

struct Button {
  uint8_t pin;
  bool stableLevel;
  bool lastRead;
  uint32_t lastChangeMs;
  uint32_t pressedAtMs;
  bool wasPressed;
};

void button_init(Button &b, uint8_t pin);
bool button_update(Button &b, bool &shortPress, bool &longPress);
