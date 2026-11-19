/**
   @file sys_led.cpp

   @brief Library for system LED control

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "sys_led.h"

sys_led system_led(STATUS_LED_GPIO_NUM, STATUS_LED_ON_DURATION);

/**
 * @brief Construct a new sys led::sys led object
 * 
 * @param uint8_t - pin number for system LED
 * @param uint32_t - duration of LED on
 */
sys_led::sys_led(uint8_t i_pin, uint32_t i_on_duration) {
  pin = i_pin;
  time = 100;
  ledOnDuration = i_on_duration;
  timer = NULL;
}

/**
 * @brief Construct a new sys led::sys led object
 * 
 * @param uint8_t - pin number for system LED
 * @param uint32_t - duration of LED on
 * @param Logs * - pointer to log class
 */
sys_led::sys_led(uint8_t i_pin, uint32_t i_on_duration, Logs *i_log) {
  pin = i_pin;
  time = 100;
  log = i_log;
  ledOnDuration = i_on_duration;
  timer = NULL;
}

/**
 * @brief Init system LED
 *
 */
void sys_led::init() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, STATUS_LED_OFF_PIN_LEVEL);
}

/**
 * @brief Start blinking of the system LED. The LED is toggled from the esp_timer task, without own task
 *
 * @return bool - status
 */
bool sys_led::start() {
  esp_timer_create_args_t timer_args = {};
  timer_args.callback = &sys_led::timerCallback;
  timer_args.arg = this;
  timer_args.dispatch_method = ESP_TIMER_TASK;
  timer_args.name = "SystemLed";

  if (ESP_OK != esp_timer_create(&timer_args, &timer)) {
    timer = NULL;
    return false;
  }

  /* first toggle immediately, same as the previous SystemLed task */
  toggle();
  return (ESP_OK == esp_timer_start_once(timer, (uint64_t)getTimer() * 1000));
}

/**
 * @brief Callback of the esp_timer. Toggle system LED and start the timer again with the next time
 *
 * @param void * - pointer to the sys_led object
 */
void sys_led::timerCallback(void *i_arg) {
  sys_led *led = (sys_led *)i_arg;

  led->toggle();
  esp_timer_start_once(led->timer, (uint64_t)led->getTimer() * 1000);
}

/**
 * @brief Toggle system LED
 * 
 */
void sys_led::toggle() {
  digitalWrite(pin, !digitalRead(pin));
}

/**
 * @brief Set system LED
 * 
 * @param bool - state of LED
 */
void sys_led::set(bool state) {
  digitalWrite(pin, state);
}

/**
 * @brief Get system LED
 * 
 * @return bool - state of LED
 */
bool sys_led::get() {
  return digitalRead(pin);
}

/**
 * @brief Set timer for system LED
 * 
 * @param uint32_t - time in ms
 */
void sys_led::setTimer(uint32_t i_time) {
  time = i_time;
}

/**
 * @brief Get timer for next start task for system LED
 * 
 * @return uint32_t - time in ms
 */
uint32_t sys_led::getTimer() {
  uint32_t tmp = 0;

  if (digitalRead(pin) == STATUS_LED_OFF_PIN_LEVEL) {
    tmp = ledOnDuration;
  } else {
    tmp = time;
  }

  return tmp;
}

/* EOF */