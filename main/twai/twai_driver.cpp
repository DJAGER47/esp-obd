#include "twai_driver.h"

#include <string.h>

#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_system.h"
#include "esp_twai_onchip.h"
#include "portmacro.h"
#include "time.h"

static const char* const TAG = "TwaiDriver";

TwaiDriver::TwaiDriver(gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t speed_kbps) :
    tx_pin_(tx_pin),
    rx_pin_(rx_pin),
    speed_kbps_(speed_kbps),
    init_(false),
    node_handle_(nullptr),
    tx_queue_(nullptr),
    is_transmitting_(false),
    rx_error_count_(0),
    tx_error_count_(0) {}

void TwaiDriver::InstallStart() {
  twai_onchip_node_config_t node_config = {.io_cfg =
                                               {
                                                   .tx                = tx_pin_,
                                                   .rx                = rx_pin_,
                                                   .quanta_clk_out    = GPIO_NUM_NC,  // Не используется
                                                   .bus_off_indicator = GPIO_NUM_NC   // Не используется
                                               },
                                           .clk_src = TWAI_CLK_SRC_DEFAULT,
                                           .bit_timing =
                                               {
                                                   .bitrate = speed_kbps_ * 1000,  // Преобразование из kbps в bps
                                                   .sp_permill = 0,  // Использовать значение по умолчанию
                                                   .ssp_permill = 0  // Использовать значение по умолчанию
                                               },
                                           .data_timing = {.bitrate = 0,  // Не используется для классического CAN
                                                           .sp_permill  = 0,
                                                           .ssp_permill = 0},
                                           .fail_retry_cnt = 3,  // Повторить 3 раза при ошибке
                                           .tx_queue_depth = kTxQueueDepth,
                                           .intr_priority  = 0,  // Приоритет по умолчанию
                                           .flags          = {
                                                        .enable_self_test   = false,
                                                        .enable_loopback    = false,
                                                        .enable_listen_only = false,
                                                        .no_receive_rtr     = false,
                                           }};

  // Создание узла TWAI
  esp_err_t err = twai_new_node_onchip(&node_config, &node_handle_);
  if (err != ESP_OK) {
    ESP_LOGI(TAG, "TWAI initialization failed. Restarting in 5 seconds...");
    esp_rom_delay_us(5000000);
    esp_restart();
  }

  // Регистрация обратных вызовов
  twai_event_callbacks_t callbacks = {.on_tx_done      = TxCallback,
                                      .on_rx_done      = RxCallback,
                                      .on_state_change = StateChangeCallback,
                                      .on_error        = ErrorCallback};

  err = twai_node_register_event_callbacks(node_handle_, &callbacks, this);
  if (err != ESP_OK) {
    ESP_LOGI(TAG, "TWAI callback registration failed. Restarting in 5 seconds...");
    esp_rom_delay_us(5000000);
    esp_restart();
  }

  // Создание очередей FreeRTOS
  tx_queue_ = xQueueCreate(kTxQueueDepth, sizeof(TwaiFrame));
  if (tx_queue_ == nullptr) {
    ESP_LOGI(TAG, "Failed to create TX queue. Restarting in 5 seconds...");
    esp_rom_delay_us(5000000);
    esp_restart();
  }

  init_ = true;

  // Включение узла
  err = twai_node_enable(node_handle_);
  if (err != ESP_OK) {
    ESP_LOGI(TAG, "Failed to enable TWAI node. Restarting in 5 seconds...");
    esp_rom_delay_us(5000000);
    esp_restart();
  }

  ESP_LOGI(TAG, "TWAI driver installed and started successfully");
}

bool IRAM_ATTR TwaiDriver::TxCallback(twai_node_handle_t handle,
                                      const twai_tx_done_event_data_t* edata,
                                      void* user_ctx) {
  TwaiDriver* driver = static_cast<TwaiDriver*>(user_ctx);
  if (driver && driver->node_handle_ == handle) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    TwaiFrame next_frame;

    if (xQueueReceiveFromISR(driver->tx_queue_, &next_frame, &xHigherPriorityTaskWoken) == pdTRUE) {
      twai_frame_t frame = {};
      frame.header.id    = next_frame.id;
      frame.header.ide   = next_frame.is_extended;
      frame.header.rtr   = next_frame.is_rtr;
      frame.header.fdf   = next_frame.is_fd;
      frame.header.brs   = next_frame.brs;
      frame.header.dlc   = next_frame.data_length;
      frame.buffer       = const_cast<uint8_t*>(next_frame.data);
      frame.buffer_len   = next_frame.data_length;

      // Используем нулевой таймаут, чтобы не блокироваться в прерывании
      esp_err_t err = twai_node_transmit(handle, &frame, 0);
      if (err != ESP_OK) {
        ESP_DRAM_LOGE(TAG, "TxCallback: err %d", err);
        xQueueSendToFrontFromISR(driver->tx_queue_, &next_frame, &xHigherPriorityTaskWoken);
        driver->is_transmitting_.store(false, std::memory_order_relaxed);
      }
    } else {
      driver->is_transmitting_.store(false, std::memory_order_relaxed);
    }
    // Возвращаем true, если нужно переключение контекста
    return xHigherPriorityTaskWoken == pdTRUE;
  }
  return false;
}

bool IRAM_ATTR TwaiDriver::RxCallback(twai_node_handle_t handle,
                                      const twai_rx_done_event_data_t* edata,
                                      void* user_ctx) {
  TwaiDriver* driver = static_cast<TwaiDriver*>(user_ctx);
  if (driver && driver->node_handle_ == handle) {
    twai_frame_t frame       = {};
    TwaiFrame received_frame = {};
    frame.buffer             = received_frame.data;
    frame.buffer_len         = 8;

    esp_err_t err = twai_node_receive_from_isr(handle, &frame);
    if (err == ESP_OK) {
      received_frame.id          = frame.header.id;
      received_frame.is_extended = frame.header.ide;
      received_frame.is_rtr      = frame.header.rtr;
      received_frame.is_fd       = frame.header.fdf;
      received_frame.brs         = frame.header.brs;
      received_frame.data_length = frame.header.dlc;

      driver->DispatchMessage(received_frame);
      return true;
    } else {
      ESP_DRAM_LOGE(TAG, "RxCallback: err %d", err);
    }
  }
  return false;  // Не требуется переключение контекста
}

// Реализация обработчика изменения состояния TWAI
bool IRAM_ATTR TwaiDriver::StateChangeCallback(twai_node_handle_t handle,
                                               const twai_state_change_event_data_t* edata,
                                               void* user_ctx) {
  ESP_DRAM_LOGI(
      TAG, "TWAI state changed: old=%d, new=%d", static_cast<int>(edata->old_sta), static_cast<int>(edata->new_sta));
  if (edata->new_sta == TWAI_ERROR_ACTIVE) {
    ESP_DRAM_LOGE(TAG, "TWAI_ERROR_ACTIVE");
  } else if (edata->new_sta == TWAI_ERROR_WARNING) {
    ESP_DRAM_LOGE(TAG, "TWAI_ERROR_WARNING");
  } else if (edata->new_sta == TWAI_ERROR_PASSIVE) {
    ESP_DRAM_LOGE(TAG, "TWAI_ERROR_PASSIVE");
  } else if (edata->new_sta == TWAI_ERROR_BUS_OFF) {
    ESP_DRAM_LOGE(TAG, "TWAI_ERROR_BUS_OFF");
  }
  return true;
}

// Реализация обработчика ошибок TWAI
bool IRAM_ATTR TwaiDriver::ErrorCallback(twai_node_handle_t handle,
                                         const twai_error_event_data_t* edata,
                                         void* user_ctx) {
  TwaiDriver* driver = static_cast<TwaiDriver*>(user_ctx);
  if (driver && driver->node_handle_ == handle) {
    bool is_rx_error = false;
    bool is_tx_error = false;

    // Ошибки, связанные с передачей (отправкой)
    if (edata->err_flags.arb_lost) {
      ESP_DRAM_LOGE(TAG, "TWAI error: Arbitration lost error");
      is_tx_error = true;
    }
    if (edata->err_flags.ack_err) {
      ESP_DRAM_LOGE(TAG, "TWAI error: ACK error (no ack)");
      is_tx_error = true;
    }

    // Ошибки, связанные с приемом
    if (edata->err_flags.bit_err) {
      ESP_DRAM_LOGE(TAG, "TWAI error: Bit error detected");
      is_rx_error = true;
    }
    if (edata->err_flags.form_err) {
      ESP_DRAM_LOGE(TAG, "TWAI error: Form error detected");
      is_rx_error = true;
    }
    if (edata->err_flags.stuff_err) {
      ESP_DRAM_LOGE(TAG, "TWAI error: Stuff error detected");
      is_rx_error = true;
    }

    // Увеличиваем соответствующие счетчики ошибок
    if (is_rx_error) {
      driver->rx_error_count_.fetch_add(1, std::memory_order_relaxed);
    }
    if (is_tx_error) {
      driver->tx_error_count_.fetch_add(1, std::memory_order_relaxed);
    }

    ESP_DRAM_LOGE(TAG, "\nTWAI error occurred: info=0x%08x", edata->err_flags.val);

    // Получаем информацию о состоянии узла
    twai_node_status_t status;
    twai_node_record_t statistics;
    esp_err_t err = twai_node_get_info(handle, &status, &statistics);

    if (err == ESP_OK) {
      ESP_DRAM_LOGE(TAG,
                    "TWAI Node Status: Error State: %d | TX Error Count: %d | RX Error Count: %d",
                    status.state,
                    status.tx_error_count,
                    status.rx_error_count);
      ESP_DRAM_LOGE(TAG, "TWAI Node Statistics: Bus Error Count: %d", statistics.bus_err_num);
    } else {
      ESP_DRAM_LOGE(TAG, "Failed to get TWAI node info: %s", esp_err_to_name(err));
    }
  }

  // Здесь можно добавить дополнительную логику обработки ошибок
  return true;
}

IPhyInterface::TwaiError TwaiDriver::Transmit(const TwaiFrame& message, Time_ms timeout_ms) {
  if (xQueueSend(tx_queue_, &message, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
    ESP_LOGW(TAG, "Failed to add frame to TX queue: queue full or timeout");
    return IPhyInterface::TwaiError::TIMEOUT;
  }

  // Атомарно проверяем и устанавливаем флаг передачи
  // Для одноядерной системы достаточно relaxed - защита только от прерываний
  bool expected = false;
  if (is_transmitting_.compare_exchange_strong(expected, true, std::memory_order_relaxed, std::memory_order_relaxed)) {
    // Мы успешно захватили право на запуск передачи
    TwaiFrame next_frame;
    if (xQueueReceive(tx_queue_, &next_frame, 0) == pdTRUE) {
      twai_frame_t frame = {};
      frame.header.id    = next_frame.id;
      frame.header.ide   = next_frame.is_extended;
      frame.header.rtr   = next_frame.is_rtr;
      frame.header.fdf   = next_frame.is_fd;
      frame.header.brs   = next_frame.brs;
      frame.header.dlc   = next_frame.data_length;
      frame.buffer       = const_cast<uint8_t*>(next_frame.data);
      frame.buffer_len   = next_frame.data_length;

      esp_err_t result = twai_node_transmit(node_handle_, &frame, timeout_ms);
      if (result != ESP_OK) {
        xQueueSendToFront(tx_queue_, &next_frame, 0);
        is_transmitting_.store(false, std::memory_order_relaxed);
        ESP_DRAM_LOGE(TAG, "Failed to transmit frame: %s", esp_err_to_name(result));
        if (result == ESP_ERR_TIMEOUT) {
          return IPhyInterface::TwaiError::TIMEOUT;
        }
        return IPhyInterface::TwaiError::TRANSMIT_FAILED;
      }
      return IPhyInterface::TwaiError::OK;
    } else {
      is_transmitting_.store(false, std::memory_order_relaxed);
    }
  }
  return IPhyInterface::TwaiError::OK;
}

void TwaiDriver::RegisterSubscriber(ITwaiSubscriber& subscriber) {
  bool has_free_slot = false;
  for (size_t i = 0; i < subscribers_.size(); ++i) {
    if (subscribers_[i] == nullptr) {
      subscribers_[i] = &subscriber;
      has_free_slot   = true;
      break;
    }
  }

  if (!has_free_slot) {
    ESP_LOGE(TAG, "RegisterSubscriber: restart, becose not free slot");
    esp_rom_delay_us(5000000);
    esp_restart();
  }
}

void TwaiDriver::UnRegisterSubscriber(ITwaiSubscriber& subscriber) {
  bool found_subscriber = false;
  for (size_t i = 0; i < subscribers_.size(); ++i) {
    if (subscribers_[i] == &subscriber) {
      subscribers_[i]  = nullptr;
      found_subscriber = true;
      break;
    }
  }

  if (!found_subscriber) {
    ESP_LOGW(TAG, "UnRegisterSubscriber: subscriber not found");
  }
}

void TwaiDriver::DispatchMessage(const TwaiFrame& message) {
  for (size_t i = 0; i < subscribers_.size() && subscribers_[i] != nullptr; ++i) {
    ITwaiSubscriber* subscriber = subscribers_[i];
    if (subscriber != nullptr) {
      if (subscriber->isInterested(message)) {
        QueueHandle_t queue = subscriber->onTwaiMessage();
        if (queue != nullptr) {
          // Помещаем сообщение в очередь подписчика
          BaseType_t xHigherPriorityTaskWoken = pdFALSE;
          if (xQueueSendToBackFromISR(queue, &message, &xHigherPriorityTaskWoken) != pdTRUE) {
            ESP_LOGW(TAG, "Failed to send message to subscriber queue: queue full");
          }
        }
      }
    }
  }
}

void TwaiDriver::ResetErrorCount() {
  rx_error_count_.store(0, std::memory_order_relaxed);
  tx_error_count_.store(0, std::memory_order_relaxed);
}

uint32_t TwaiDriver::GetRxErrorCount() const {
  return rx_error_count_.load(std::memory_order_relaxed);
}

uint32_t TwaiDriver::GetTxErrorCount() const {
  return tx_error_count_.load(std::memory_order_relaxed);
}
