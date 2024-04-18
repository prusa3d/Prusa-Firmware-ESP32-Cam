/**
   @file sys_led.h

   @brief Library for system LED control

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#ifndef _SYS_LED_H_
#define _SYS_LED_H_

#include "log.h"
#include "mcu_cfg.h"
#include "arduino.h"

class sys_led {
private:
  uint8_t pin;            ///< pin number for system LED
  uint32_t time;          ///< speed blinking time system LED
  uint32_t ledOnDuration; ///< duration of LED on
  Logs *log;              ///< pointer to log class

public:
  sys_led(uint8_t, uint32_t);
  sys_led(uint8_t, uint32_t, Logs *);
  ~sys_led(){};

  void init();
  void toggle();
  void set(bool);
  bool get();
  void setTimer(uint32_t);
  uint32_t getTimer();
};

extern sys_led system_led;

#endif

/* EOF */