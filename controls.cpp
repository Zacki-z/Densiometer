#include "controls.h"

void button_init(Button &b, uint8_t pin) {
  b.pin = pin;
  pinMode(b.pin, INPUT_PULLUP);
  b.stableLevel = digitalRead(b.pin);
  b.lastRead = b.stableLevel;
  b.lastChangeMs = millis();
  b.pressedAtMs = 0;
  b.wasPressed = false;
}

bool button_update(Button &b, bool &shortPress, bool &longPress) {
  shortPress = false;
  longPress  = false;

  bool r = digitalRead(b.pin);

  if (r != b.lastRead) {
    b.lastRead = r;
    b.lastChangeMs = millis();
  }

  if ((millis() - b.lastChangeMs) > DEBOUNCE_MS && r != b.stableLevel) {
    b.stableLevel = r;

    if (b.stableLevel == LOW) {
      b.pressedAtMs = millis();
      b.wasPressed = true;
    } else {
      if (b.wasPressed) {
        uint32_t dt = millis() - b.pressedAtMs;
        if (dt >= LONGPRESS_MS) longPress = true;
        else shortPress = true;
      }
      b.wasPressed = false;
    }
    return true;
  }
  return false;
}
