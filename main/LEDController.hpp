#pragma once
#include "driver/gpio.h"

class LEDController {
 public:
  LEDController(gpio_num_t gpio_pin = GPIO_NUM_2) :
      pin(gpio_pin),
      state(false) {
    configure();
  }

  void configure() {
    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    off();  // Изначально выключаем светодиод
  }

  void on() {
    gpio_set_level(pin, 1);
    state = true;
  }

  void off() {
    gpio_set_level(pin, 0);
    state = false;
  }

  void toggle() {
    state ? off() : on();
  }

  bool get_state() const {
    return state;
  }

 private:
  gpio_num_t pin;
  bool state;
};