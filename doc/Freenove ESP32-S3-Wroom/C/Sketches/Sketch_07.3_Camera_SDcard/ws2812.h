#ifndef __WS2812_H
#define __WS2812_H

#include "Freenove_WS2812_Lib_for_ESP32.h"

#define WS2812_PIN  48

void ws2812Init(void);
void ws2812SetColor(int color);

#endif

