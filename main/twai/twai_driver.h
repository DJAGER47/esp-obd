#pragma once

class TwaiDriver final : public ITwaiInterface {
 public:
  TwaiDriver(gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t speed_kbps);

  TwaiError install_and_start() override;
  TwaiError transmit(?? message,
                     TickType_t ticks_to_wait) override;
  TwaiError receive(?? message, TickType_t ticks_to_wait) override;

 private:
};
