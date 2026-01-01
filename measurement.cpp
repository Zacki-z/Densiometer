#include "measurement.h"
#include "util.h"

float read_sensor_voltage_avg(uint8_t analogPin) {
  uint32_t acc = 0;
  for (uint16_t i=0; i<SAMPLE_COUNT; i++) {
    acc += (uint16_t)analogRead(analogPin);
    delay(SAMPLE_DELAY_MS);
  }
  float raw = (float)acc / (float)SAMPLE_COUNT;
  return raw * (vref_volts() / 1023.0f);
}
