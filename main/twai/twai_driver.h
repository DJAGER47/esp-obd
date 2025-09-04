#pragma once

#include "driver/twai.h"
#include "twai_errors.h"
#include "twai_interface.h"

class TwaiDriver final : public ITwaiInterface {
 public:
  TwaiDriver(gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t speed_kbps);

  TwaiError install_and_start() override;
  TwaiError transmit(const twai_message_t* message,
                     TickType_t ticks_to_wait) override;
  TwaiError receive(twai_message_t* message, TickType_t ticks_to_wait) override;

 private:
  twai_general_config_t g_config;
  twai_timing_config_t t_config;
  twai_filter_config_t f_config;
  bool driver_installed;
};
