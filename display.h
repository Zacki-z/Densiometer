#pragma once
#include <Arduino.h>

enum Level { LVL_OFF=0, LVL_LOW=1, LVL_MID=2, LVL_HIGH=3 };

const char* level_name(Level l);

void lcd_init();
void lcd_show(Level lvl, float v, bool v0_valid, float od);
void serial_show(Level lvl, float v, bool v0_valid, float v0, float od);
