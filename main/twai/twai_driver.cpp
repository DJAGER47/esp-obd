#include "twai_driver.h"

#include "esp_log.h"
#include "esp_twai_onchip.h"
#include "string.h"

static const char* TAG = "TwaiDriver";

TwaiDriver::TwaiDriver(gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t speed_kbps) :
    tx_pin_(tx_pin),
    rx_pin_(rx_pin),
    speed_kbps_(speed_kbps),
    node_handle_(nullptr),
    tx_queue_(nullptr),
    rx_queue_(nullptr) {}

ITwaiInterface::TwaiError TwaiDriver::install_and_start() {
  if (node_handle_ != nullptr) {
    ESP_LOGE(TAG, "Driver already installed");
    return ITwaiInterface::TwaiError::ALREADY_INITIALIZED;
  }

  // Создание конфигурации узла TWAI
  twai_onchip_node_config_t node_config = {
      .io_cfg =
          {
              .tx                = tx_pin_,
              .rx                = rx_pin_,
              .quanta_clk_out    = GPIO_NUM_NC,  // Не используется
              .bus_off_indicator = GPIO_NUM_NC   // Не используется
          },
      .clk_src = TWAI_CLK_SRC_DEFAULT,
      .bit_timing =
          {
              .bitrate    = speed_kbps_ * 1000,  // Преобразование из kbps в bps
              .sp_permill = 0,  // Использовать значение по умолчанию
              .ssp_permill = 0  // Использовать значение по умолчанию
          },
      .data_timing = {.bitrate = 0,  // Не используется для классического CAN
                      .sp_permill  = 0,
                      .ssp_permill = 0},
      .fail_retry_cnt = 3,  // Повторить 3 раза при ошибке
      .tx_queue_depth = kTxQueueDepth,
      .intr_priority  = 0,  // Приоритет по умолчанию
      .flags          = {.enable_self_test   = false,
                         .enable_loopback    = false,
                         .enable_listen_only = false,
                         .no_receive_rtr     = false}};

  // Создание узла TWAI
  esp_err_t err = twai_new_node_onchip(&node_config, &node_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create TWAI node: %s", esp_err_to_name(err));
    return ITwaiInterface::TwaiError::DRIVER_INSTALL_FAILED;
  }

  // Регистрация обратных вызовов
  twai_event_callbacks_t callbacks = {
      .on_tx_done      = TxCallback,
      .on_rx_done      = RxCallback,
      .on_state_change = nullptr,  // Не используется
      .on_error        = nullptr   // Не используется
  };
  err = twai_node_register_event_callbacks(node_handle_, &callbacks, this);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register callbacks: %s", esp_err_to_name(err));
    twai_node_delete(node_handle_);
    node_handle_ = nullptr;
    return ITwaiInterface::TwaiError::GENERAL_FAILURE;
  }

  // Создание очередей FreeRTOS
  tx_queue_ = xQueueCreate(kTxQueueDepth, sizeof(ITwaiInterface::TwaiFrame));
  if (tx_queue_ == nullptr) {
    ESP_LOGE(TAG, "Failed to create TX queue");
    twai_node_delete(node_handle_);
    node_handle_ = nullptr;
    return ITwaiInterface::TwaiError::GENERAL_FAILURE;
  }

  rx_queue_ = xQueueCreate(kRxQueueDepth, sizeof(ITwaiInterface::TwaiFrame));
  if (rx_queue_ == nullptr) {
    ESP_LOGE(TAG, "Failed to create RX queue");
    vQueueDelete(tx_queue_);
    tx_queue_ = nullptr;
    twai_node_delete(node_handle_);
    node_handle_ = nullptr;
    return ITwaiInterface::TwaiError::GENERAL_FAILURE;
  }

  // Включение узла
  err = twai_node_enable(node_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable TWAI node: %s", esp_err_to_name(err));
    vQueueDelete(rx_queue_);
    rx_queue_ = nullptr;
    vQueueDelete(tx_queue_);
    tx_queue_ = nullptr;
    twai_node_delete(node_handle_);
    node_handle_ = nullptr;
    return ITwaiInterface::TwaiError::DRIVER_START_FAILED;
  }

  ESP_LOGI(TAG, "TWAI driver installed and started successfully");
  return ITwaiInterface::TwaiError::OK;
}

// Статическая функция обратного вызова для передачи
bool TwaiDriver::TxCallback(twai_node_handle_t handle,
                            const twai_tx_done_event_data_t* edata,
                            void* user_ctx) {
  // В этой реализации мы не делаем ничего специфического при завершении передачи.
  // Освобождение ресурсов и управление очередью происходит в основном коде.
  return false;  // Не требуется переключение контекста
}

// Статическая функция обратного вызова для приема
bool TwaiDriver::RxCallback(twai_node_handle_t handle,
                            const twai_rx_done_event_data_t* edata,
                            void* user_ctx) {
  TwaiDriver* driver = static_cast<TwaiDriver*>(user_ctx);
  if (driver && driver->node_handle_ == handle) {
    // Получение принятого фрейма
    twai_frame_t frame = {};
    frame.buffer = static_cast<uint8_t*>(malloc(64));  // Выделяем буфер для данных
    if (frame.buffer == nullptr) {
      ESP_LOGE(TAG, "Failed to allocate buffer for received frame");
      return false;
    }
    frame.buffer_len = 64;

    esp_err_t err = twai_node_receive_from_isr(handle, &frame);
    if (err == ESP_OK) {
      // Преобразование в TwaiFrame
      ITwaiInterface::TwaiFrame received_frame = {};
      received_frame.id                        = frame.header.id;
      received_frame.is_extended               = frame.header.ide;
      received_frame.is_rtr                    = frame.header.rtr;
      received_frame.is_fd                     = frame.header.fdf;
      received_frame.brs                       = frame.header.brs;
      received_frame.data_length               = frame.header.dlc;
      if (frame.header.dlc <= 64) {
        memcpy(received_frame.data, frame.buffer, frame.header.dlc);
      }

      // Помещение фрейма в очередь приема
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (xQueueSendFromISR(driver->rx_queue_, &received_frame, &xHigherPriorityTaskWoken) !=
          pdTRUE) {
        ESP_LOGW(TAG, "RX queue is full, dropping frame");
      }

      free(frame.buffer);
      return xHigherPriorityTaskWoken == pdTRUE;
    } else {
      ESP_LOGE(TAG, "Failed to receive frame from ISR: %s", esp_err_to_name(err));
      free(frame.buffer);
    }
  }
  return false;  // Не требуется переключение контекста
}

ITwaiInterface::TwaiError TwaiDriver::transmit(const ITwaiInterface::TwaiFrame& message,
                                               TickType_t ticks_to_wait) {
  if (node_handle_ == nullptr) {
    ESP_LOGE(TAG, "Driver not initialized");
    return ITwaiInterface::TwaiError::NOT_INITIALIZED;
  }

  // Преобразование в twai_frame_t
  twai_frame_t frame = {};
  frame.header.id    = message.id;
  frame.header.ide   = message.is_extended;
  frame.header.rtr   = message.is_rtr;
  frame.header.fdf   = message.is_fd;
  frame.header.brs   = message.brs;
  frame.header.dlc   = message.data_length;
  frame.buffer = const_cast<uint8_t*>(message.data);  // twai_node_transmit не изменяет данные
  frame.buffer_len = message.data_length;

  // Преобразование ticks_to_wait в миллисекунды
  int timeout_ms =
      (ticks_to_wait == portMAX_DELAY) ? -1 : (int)(ticks_to_wait * portTICK_PERIOD_MS);

  esp_err_t err = twai_node_transmit(node_handle_, &frame, timeout_ms);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to transmit frame: %s", esp_err_to_name(err));
    if (err == ESP_ERR_TIMEOUT) {
      return ITwaiInterface::TwaiError::TIMEOUT;
    }
    return ITwaiInterface::TwaiError::TRANSMIT_FAILED;
  }

  return ITwaiInterface::TwaiError::OK;
}

ITwaiInterface::TwaiError TwaiDriver::receive(ITwaiInterface::TwaiFrame& message,
                                              TickType_t ticks_to_wait) {
  if (node_handle_ == nullptr) {
    ESP_LOGE(TAG, "Driver not initialized");
    return ITwaiInterface::TwaiError::NOT_INITIALIZED;
  }

  // Получение фрейма из очереди приема
  if (xQueueReceive(rx_queue_, &message, ticks_to_wait) != pdTRUE) {
    return ITwaiInterface::TwaiError::TIMEOUT;
  }

  return ITwaiInterface::TwaiError::OK;
}
