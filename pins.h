#pragma once
#include <Arduino.h>

// UI
static const uint8_t PIN_LED_STATUS  = 2;
static const uint8_t PIN_BTN_POWER   = 3;  // toggle
static const uint8_t PIN_BTN_MEASURE = 4;  // short=measure, long=cal

// Rotary (one-hot, active LOW with INPUT_PULLUP)
static const uint8_t PIN_ROT_0 = 5; // OFF
static const uint8_t PIN_ROT_1 = 6; // LOW
static const uint8_t PIN_ROT_2 = 7; // MID
static const uint8_t PIN_ROT_3 = 8; // HIGH

// Level outputs (MOSFET gates)
static const uint8_t PIN_MOSFET_1 = 9;
static const uint8_t PIN_MOSFET_2 = 10;

// Sensor
static const uint8_t PIN_SENSOR_AIN = A0;

// LCD
static const uint8_t LCD_ADDR = 0x27;
