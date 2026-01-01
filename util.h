#pragma once
#include <Arduino.h>
#include <math.h>

inline float compute_od(float v0, float v) {
  if (v0 <= 0.0f || v <= 0.0f) return NAN;
  return log10f(v0 / v);
}

inline float vref_volts() {
#if defined(ARDUINO_AVR_NANO_EVERY)
  return 1.1f; // INTERNAL1V1
#else
  return 1.1f; // INTERNAL (classic AVR ~1.1V)
#endif
}

inline void setup_analog_reference() {
#if defined(ARDUINO_AVR_NANO_EVERY)
  analogReference(INTERNAL1V1);
#else
  analogReference(INTERNAL);
#endif
  delay(20);
}
