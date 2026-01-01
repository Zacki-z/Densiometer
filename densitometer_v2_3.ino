#include <Arduino.h>
#include "pins.h"
#include "config.h"
#include "version.h"
#include "util.h"
#include "controls.h"
#include "measurement.h"
#include "display.h"

static bool powerEnabled = false;
static Level currentLevel = LVL_OFF;

static float V0 = 0.0f;
static bool V0_valid = false;

static Button btnPower;
static Button btnMeasure;

static void set_level_outputs(Level l) {
  digitalWrite(PIN_MOSFET_1, (l == LVL_LOW || l == LVL_HIGH) ? HIGH : LOW);
  digitalWrite(PIN_MOSFET_2, (l == LVL_MID || l == LVL_HIGH) ? HIGH : LOW);
}

static Level read_rotary_level() {
  if (digitalRead(PIN_ROT_0) == LOW) return LVL_OFF;
  if (digitalRead(PIN_ROT_1) == LOW) return LVL_LOW;
  if (digitalRead(PIN_ROT_2) == LOW) return LVL_MID;
  if (digitalRead(PIN_ROT_3) == LOW) return LVL_HIGH;
  return currentLevel;
}

static void blink_status(uint8_t times) {
  for (uint8_t i=0;i<times;i++){
    digitalWrite(PIN_LED_STATUS, HIGH);
    delay(80);
    digitalWrite(PIN_LED_STATUS, LOW);
    delay(80);
  }
}

void setup() {
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, LOW);

  pinMode(PIN_MOSFET_1, OUTPUT);
  pinMode(PIN_MOSFET_2, OUTPUT);
  set_level_outputs(LVL_OFF);

  pinMode(PIN_ROT_0, INPUT_PULLUP);
  pinMode(PIN_ROT_1, INPUT_PULLUP);
  pinMode(PIN_ROT_2, INPUT_PULLUP);
  pinMode(PIN_ROT_3, INPUT_PULLUP);

  button_init(btnPower, PIN_BTN_POWER);
  button_init(btnMeasure, PIN_BTN_MEASURE);

  Serial.begin(SERIAL_BAUD);
  Wire.begin();

  setup_analog_reference();
  lcd_init();

  blink_status(2);
}

void loop() {
  currentLevel = read_rotary_level();

  bool sp=false, lp=false;
  if (button_update(btnPower, sp, lp)) {
    if (sp || lp) {
      powerEnabled = !powerEnabled;
      blink_status(powerEnabled ? 1 : 2);
    }
  }

  bool spm=false, lpm=false;
  if (button_update(btnMeasure, spm, lpm)) {
    if (lpm) {
      if (powerEnabled && currentLevel != LVL_OFF) {
        set_level_outputs(currentLevel);
        delay(80);
        V0 = read_sensor_voltage_avg(PIN_SENSOR_AIN);
        V0_valid = (V0 > 0.01f);
        blink_status(V0_valid ? 3 : 5);
      } else {
        blink_status(5);
      }
    }
  }

  if (powerEnabled && currentLevel != LVL_OFF) {
    set_level_outputs(currentLevel);
    digitalWrite(PIN_LED_STATUS, HIGH);
  } else {
    set_level_outputs(LVL_OFF);
    digitalWrite(PIN_LED_STATUS, LOW);
  }

  float v = read_sensor_voltage_avg(PIN_SENSOR_AIN);
  float od = (V0_valid) ? compute_od(V0, v) : NAN;

  lcd_show(currentLevel, v, V0_valid, od);
  serial_show(currentLevel, v, V0_valid, V0, od);

  delay(250);
}
