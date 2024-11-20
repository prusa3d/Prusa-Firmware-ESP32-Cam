
#include "ws2812.h"

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(1, WS2812_PIN, 1, TYPE_GRB);

void ws2812Init(void)
{
  strip.begin();
  strip.setBrightness(10);
  ws2812SetColor(0);
}
void ws2812SetColor(int color)
{
  if(color==0)
  {
    strip.setLedColorData(0, 0, 0, 0);
  }
  else if(color==1)
  {
    strip.setLedColorData(0, 255, 0, 0);
  }
  else if(color==2)
  {
    strip.setLedColorData(0, 0, 255, 0);
  }
  else if(color==3)
  {
    strip.setLedColorData(0, 0, 0, 255);
  }
  strip.show();
}




