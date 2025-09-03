#ifndef TWAI_DRIVER_H
#define TWAI_DRIVER_H

#include "driver/twai.h"
#include "freertos/FreeRTOS.h"

class TwaiDriver {
 public:
  TwaiDriver(gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t speed_kbps);
  ~TwaiDriver();

  esp_err_t install_and_start();
  esp_err_t stop_and_uninstall();
  esp_err_t transmit(const twai_message_t* message, TickType_t ticks_to_wait);
  esp_err_t receive(twai_message_t* message, TickType_t ticks_to_wait);

 private:
  twai_general_config_t g_config;
  twai_timing_config_t t_config;
  twai_filter_config_t f_config;
  bool driver_installed;
};

#endif  // TWAI_DRIVER_H