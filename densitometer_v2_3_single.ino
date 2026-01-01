#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

#define FW_VERSION "v2.3-single"

// -------- Pins (Nano Every mapping) --------
static const uint8_t PIN_LED_STATUS   = 2;
static const uint8_t PIN_BTN_POWER    = 3;  // ON/OFF toggle
static const uint8_t PIN_BTN_MEASURE  = 4;  // short=measure, long=cal
static const uint8_t PIN_ROT_0        = 5;  // OFF
static const uint8_t PIN_ROT_1        = 6;  // LOW
static const uint8_t PIN_ROT_2        = 7;  // MID
static const uint8_t PIN_ROT_3        = 8;  // HIGH
static const uint8_t PIN_MOSFET_1     = 9;  // level switch A
static const uint8_t PIN_MOSFET_2     = 10; // level switch B
static const uint8_t PIN_SENSOR_AIN   = A0; // TIA output

// -------- LCD --------
static const uint8_t LCD_ADDR = 0x27;
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// -------- Config --------
static const uint16_t SAMPLE_COUNT = 50;     // averaging
static const uint16_t SAMPLE_DELAY_MS = 2;   // small delay between samples
static const uint16_t DEBOUNCE_MS = 35;
static const uint16_t LONGPRESS_MS = 900;

// Choose reference per board
static float vref_volts() {
#if defined(ARDUINO_AVR_NANO_EVERY)
  // Nano Every supports INTERNAL1V1
  return 1.1f;
#else
  // Classic AVR INTERNAL ~1.1V; you can adjust if needed
  return 1.1f;
#endif
}

static void setup_analog_reference() {
#if defined(ARDUINO_AVR_NANO_EVERY)
  analogReference(INTERNAL1V1);
#else
  analogReference(INTERNAL);
#endif
  delay(20);
}

// -------- State --------
enum Level { LVL_OFF=0, LVL_LOW=1, LVL_MID=2, LVL_HIGH=3 };

static bool powerEnabled = false;
static Level currentLevel = LVL_OFF;

static float V0 = 0.0f;         // reference voltage in "clear" condition
static bool V0_valid = false;

// -------- Utils --------
static const char* level_name(Level l) {
  switch (l) {
    case LVL_OFF: return "OFF ";
    case LVL_LOW: return "LOW ";
    case LVL_MID: return "MID ";
    case LVL_HIGH:return "HIGH";
    default: return "UNK ";
  }
}

static void set_level_outputs(Level l) {
  // Truth table for two MOSFET gates:
  // OFF: 0,0
  // LOW: 1,0
  // MID: 0,1
  // HIGH:1,1
  digitalWrite(PIN_MOSFET_1, (l == LVL_LOW || l == LVL_HIGH) ? HIGH : LOW);
  digitalWrite(PIN_MOSFET_2, (l == LVL_MID || l == LVL_HIGH) ? HIGH : LOW);
}

static Level read_rotary_level() {
  // Expect one-hot rotary: a selected position pulls input LOW (using INPUT_PULLUP)
  if (digitalRead(PIN_ROT_0) == LOW) return LVL_OFF;
  if (digitalRead(PIN_ROT_1) == LOW) return LVL_LOW;
  if (digitalRead(PIN_ROT_2) == LOW) return LVL_MID;
  if (digitalRead(PIN_ROT_3) == LOW) return LVL_HIGH;
  // If none active, keep last (or OFF)
  return currentLevel;
}

static float read_sensor_voltage_avg() {
  uint32_t acc = 0;
  for (uint16_t i=0; i<SAMPLE_COUNT; i++) {
    acc += (uint16_t)analogRead(PIN_SENSOR_AIN);
    delay(SAMPLE_DELAY_MS);
  }
  float raw = (float)acc / (float)SAMPLE_COUNT;
  float v = raw * (vref_volts() / 1023.0f);
  return v;
}

static float compute_od(float v0, float v) {
  if (v0 <= 0.0f || v <= 0.0f) return NAN;
  return log10f(v0 / v);
}

// -------- Button handling (debounce + longpress) --------
struct ButtonState {
  uint8_t pin;
  bool stableLevel;
  bool lastRead;
  uint32_t lastChangeMs;
  uint32_t pressedAtMs;
  bool wasPressed;
};

static ButtonState btnPower { PIN_BTN_POWER, HIGH, HIGH, 0, 0, false };
static ButtonState btnMeasure{ PIN_BTN_MEASURE,HIGH, HIGH, 0, 0, false };

static void init_button(ButtonState &b) {
  pinMode(b.pin, INPUT_PULLUP);
  b.stableLevel = digitalRead(b.pin);
  b.lastRead = b.stableLevel;
  b.lastChangeMs = millis();
  b.pressedAtMs = 0;
  b.wasPressed = false;
}

static bool update_button(ButtonState &b, bool &shortPress, bool &longPress) {
  shortPress = false;
  longPress = false;

  bool r = digitalRead(b.pin);

  if (r != b.lastRead) {
    b.lastRead = r;
    b.lastChangeMs = millis();
  }

  // If stable for debounce
  if ((millis() - b.lastChangeMs) > DEBOUNCE_MS && r != b.stableLevel) {
    b.stableLevel = r;

    if (b.stableLevel == LOW) {
      // pressed
      b.pressedAtMs = millis();
      b.wasPressed = true;
    } else {
      // released
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

// -------- UI --------
static void lcd_show(Level lvl, float v, float od) {
  lcd.setCursor(0,0);
  lcd.print("LVL:");
  lcd.print(level_name(lvl));
  lcd.print(" V:");
  if (v < 0.01f) lcd.print("0.00");
  else {
    char buf[6];
    dtostrf(v, 4, 2, buf);
    lcd.print(buf);
  }

  lcd.setCursor(0,1);
  lcd.print("OD:");
  if (!V0_valid) {
    lcd.print("----   ");
  } else if (isnan(od)) {
    lcd.print("NaN    ");
  } else {
    char b2[8];
    dtostrf(od, 5, 2, b2);
    lcd.print(b2);
    lcd.print("  ");
  }
}

static void serial_show(Level lvl, float v, float od) {
  Serial.print("Level=");
  Serial.print(level_name(lvl));
  Serial.print("  V=");
  Serial.print(v, 4);
  Serial.print("  V0=");
  if (V0_valid) Serial.print(V0, 4);
  else Serial.print("unset");
  Serial.print("  OD=");
  if (V0_valid && !isnan(od)) Serial.println(od, 4);
  else Serial.println("----");
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

  init_button(btnPower);
  init_button(btnMeasure);

  Serial.begin(115200);
  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Densiometer");
  lcd.setCursor(0,1);
  lcd.print(FW_VERSION);

  setup_analog_reference();

  delay(900);
  lcd.clear();
  blink_status(2);
}

void loop() {
  // Read rotary level selection
  Level newLevel = read_rotary_level();
  if (newLevel != currentLevel) {
    currentLevel = newLevel;
  }

  // Buttons
  bool sp=false, lp=false;
  if (update_button(btnPower, sp, lp)) {
    if (sp || lp) {
      powerEnabled = !powerEnabled;
      blink_status(powerEnabled ? 1 : 2);
    }
  }

  bool spm=false, lpm=false;
  if (update_button(btnMeasure, spm, lpm)) {
    if (lpm) {
      // Long press -> calibrate V0
      if (powerEnabled && currentLevel != LVL_OFF) {
        set_level_outputs(currentLevel);
        delay(80);
        V0 = read_sensor_voltage_avg();
        V0_valid = (V0 > 0.01f);
        blink_status(V0_valid ? 3 : 5);
      } else {
        blink_status(5);
      }
    } else if (spm) {
      // Short press -> measure once
      // nothing here; we’ll do it below in a single pass
    }
  }

  // Apply outputs
  if (powerEnabled && currentLevel != LVL_OFF) {
    set_level_outputs(currentLevel);
    digitalWrite(PIN_LED_STATUS, HIGH);
  } else {
    set_level_outputs(LVL_OFF);
    digitalWrite(PIN_LED_STATUS, LOW);
  }

  // Measure only when user short-presses measure button
  // (We detect short press by checking stable release event; easiest is to re-check just after update)
  // Here: do a low-rate background display even without measurement.
  float v = read_sensor_voltage_avg();
  float od = (V0_valid) ? compute_od(V0, v) : NAN;

  lcd_show(currentLevel, v, od);
  serial_show(currentLevel, v, od);

  delay(250);
}
