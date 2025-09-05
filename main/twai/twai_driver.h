#pragma once

#include "driver/gpio.h"
#include "esp_twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "twai_interface.h"

class TwaiDriver final : public ITwaiInterface {
 public:
  TwaiDriver(gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t speed_kbps);

  TwaiError install_and_start() override;
  TwaiError transmit(const TwaiFrame& message, TickType_t ticks_to_wait) override;
  TwaiError receive(TwaiFrame& message, TickType_t ticks_to_wait) override;

 private:
  gpio_num_t tx_pin_;
  gpio_num_t rx_pin_;
  uint32_t speed_kbps_;
  twai_node_handle_t node_handle_;
  QueueHandle_t tx_queue_;
  QueueHandle_t rx_queue_;
  static const int kTxQueueDepth = 10;
  static const int kRxQueueDepth = 10;

  // Статические функции обратного вызова
  static bool TxCallback(twai_node_handle_t handle,
                         const twai_tx_done_event_data_t* edata,
                         void* user_ctx);
  static bool RxCallback(twai_node_handle_t handle,
                         const twai_rx_done_event_data_t* edata,
                         void* user_ctx);
};
