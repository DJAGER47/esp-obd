#pragma once

#include <array>

#include "driver/gpio.h"
#include "esp_twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "phy_interface.h"
#include "time.h"

class TwaiDriver final : public IPhyInterface {
 public:
  TwaiDriver(gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t speed_kbps);
  TwaiError InstallStart() override;
  TwaiError Transmit(const TwaiFrame& message, Time_ms timeout_ms) override;
  TwaiError RegisterSubscriber(ITwaiSubscriber& subscriber);

 private:
  static const int kTxQueueDepth   = 10;
  static const int kMaxSubscribers = 8;

  static bool TxCallback(twai_node_handle_t handle,
                         const twai_tx_done_event_data_t* edata,
                         void* user_ctx);
  static bool RxCallback(twai_node_handle_t handle,
                         const twai_rx_done_event_data_t* edata,
                         void* user_ctx);

  void DispatchMessage(const TwaiFrame& message);

  gpio_num_t tx_pin_;
  gpio_num_t rx_pin_;
  uint32_t speed_kbps_;
  twai_node_handle_t node_handle_;
  QueueHandle_t tx_queue_;
  std::array<ITwaiSubscriber*, kMaxSubscribers> subscribers_;
};
