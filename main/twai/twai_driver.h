#pragma once

#include <array>
#include <atomic>

#include "driver/gpio.h"
#include "esp_twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "phy_interface.h"
#include "time.h"

class TwaiDriver final : public IPhyInterface {
 public:
  TwaiDriver(gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t speed_kbps);
  void InstallStart() override;
  TwaiError Transmit(const TwaiFrame& message, Time_ms timeout_ms) override;
  void RegisterSubscriber(ITwaiSubscriber& subscriber);

 private:
  static const int kTxQueueDepth   = 10;
  static const int kMaxSubscribers = 2;

  static bool IRAM_ATTR TxCallback(twai_node_handle_t handle, const twai_tx_done_event_data_t* edata, void* user_ctx);
  static bool IRAM_ATTR RxCallback(twai_node_handle_t handle, const twai_rx_done_event_data_t* edata, void* user_ctx);
  static bool IRAM_ATTR StateChangeCallback(twai_node_handle_t handle,
                                            const twai_state_change_event_data_t* edata,
                                            void* user_ctx);
  static bool IRAM_ATTR ErrorCallback(twai_node_handle_t handle, const twai_error_event_data_t* edata, void* user_ctx);

  void DispatchMessage(const TwaiFrame& message);

  const gpio_num_t tx_pin_;
  const gpio_num_t rx_pin_;
  const uint32_t speed_kbps_;

  bool init_;

  twai_node_handle_t node_handle_;
  QueueHandle_t tx_queue_;
  std::array<ITwaiSubscriber*, kMaxSubscribers> subscribers_;
  std::atomic_bool is_transmitting_;  // Флаг, указывающий, идет ли передача в данный момент
};
