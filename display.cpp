#include "display.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "pins.h"
#include "version.h"

static LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

const char* level_name(Level l) {
  switch (l) {
    case LVL_OFF: return "OFF ";
    case LVL_LOW: return "LOW ";
    case LVL_MID: return "MID ";
    case LVL_HIGH:return "HIGH";
    default: return "UNK ";
  }
}

void lcd_init() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(FW_NAME);
  lcd.setCursor(0,1);
  lcd.print(FW_VERSION);
  delay(900);
  lcd.clear();
}

void lcd_show(Level lvl, float v, bool v0_valid, float od) {
  lcd.setCursor(0,0);
  lcd.print("LVL:");
  lcd.print(level_name(lvl));
  lcd.print(" V:");
  char buf[8];
  dtostrf(v, 4, 2, buf);
  lcd.print(buf);

  lcd.setCursor(0,1);
  lcd.print("OD:");
  if (!v0_valid) {
    lcd.print("----   ");
  } else if (isnan(od)) {
    lcd.print("NaN    ");
  } else {
    char b2[10];
    dtostrf(od, 5, 2, b2);
    lcd.print(b2);
    lcd.print("  ");
  }
}

void serial_show(Level lvl, float v, bool v0_valid, float v0, float od) {
  Serial.print("Level=");
  Serial.print(level_name(lvl));
  Serial.print(" V=");
  Serial.print(v, 4);
  Serial.print(" V0=");
  if (v0_valid) Serial.print(v0, 4);
  else Serial.print("unset");
  Serial.print(" OD=");
  if (v0_valid && !isnan(od)) Serial.println(od, 4);
  else Serial.println("----");
}
