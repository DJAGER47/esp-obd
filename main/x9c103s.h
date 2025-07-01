#ifndef X9C103S_H
#define X9C103S_H

#include "driver/gpio.h"
#include "esp_err.h"

typedef struct {
  gpio_num_t cs_pin;
  gpio_num_t ud_pin;
  gpio_num_t inc_pin;
} x9c103s_dev_t;

esp_err_t x9c103s_init(x9c103s_dev_t *dev);
esp_err_t x9c103s_set_resistance(x9c103s_dev_t *dev, uint8_t value);

#endif