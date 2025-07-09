#include <stdio.h>

#include "LEDController.hpp"
#include "driver/gpio.h"
#include "driver/twai.h"
#include "esp_freertos_hooks.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "io.h"

class CANDriver {
 public:
  CANDriver() :
      led(gpio_num_t(8)),  // GPIO8 для светодиода
      rx_head(0),
      rx_tail(0) {}

  void init() {
    // LED инициализируется в конструкторе

    // Конфигурация TWAI драйвера
    twai_general_config_t g_config = {.mode           = TWAI_MODE_NORMAL,
                                      .tx_io          = GPIO_NUM_5,
                                      .rx_io          = GPIO_NUM_4,
                                      .clkout_io      = TWAI_IO_UNUSED,
                                      .bus_off_io     = TWAI_IO_UNUSED,
                                      .tx_queue_len   = 5,
                                      .rx_queue_len   = 20,
                                      .alerts_enabled = TWAI_ALERT_RX_DATA,
                                      .clkout_divider = 0};

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Инициализация драйвера
    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to install driver: %s", esp_err_to_name(ret));
      return;
    }

    // Запуск драйвера
    ret = twai_start();
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to start driver: %s", esp_err_to_name(ret));
      return;
    }

    // Регистрация обработчика прерываний
    twai_reconfigure_alerts(TWAI_ALERT_RX_DATA, nullptr);
    // В ESP-IDF v5.5 прямой регистрации callback нет, используем альтернативный
    // подход

    ESP_LOGI(TAG, "TWAI driver initialized successfully");
  }

  void start_receive_task() {
    xTaskCreate(
        &CANDriver::receive_task, "CAN_RX_TASK", 4096, this, 5, nullptr);
  }

  void run() {
    while (true) {
      led.toggle();
      vTaskDelay(250 / portTICK_PERIOD_MS);
    }
  }

 private:
  static constexpr const char* TAG        = "CAN_DRIVER";
  static constexpr size_t RX_BUFFER_SIZE  = 32;
  static constexpr uint32_t OBD2_CAN_ID   = 0x7E8;
  static constexpr uint32_t OBD2_CAN_MASK = 0x7FF;

  LEDController led;
  twai_message_t rx_buffer[RX_BUFFER_SIZE];
  volatile uint8_t rx_head;
  volatile uint8_t rx_tail;

  static void IRAM_ATTR isr_handler(void* arg) {
    CANDriver* self = static_cast<CANDriver*>(arg);
    twai_message_t message;

    while (twai_receive(&message, 0) == ESP_OK) {
      uint8_t next_head = (self->rx_head + 1) % RX_BUFFER_SIZE;
      if (next_head != self->rx_tail) {
        self->rx_buffer[self->rx_head] = message;
        self->rx_head                  = next_head;
      }
    }
  }

  static void receive_task(void* arg) {
    CANDriver* self = static_cast<CANDriver*>(arg);

    while (true) {
      twai_message_t message;
      if (self->rx_head != self->rx_tail) {
        twai_message_t message = self->rx_buffer[self->rx_tail];
        self->rx_tail          = (self->rx_tail + 1) % RX_BUFFER_SIZE;
        if (message.identifier == OBD2_CAN_ID) {
          ESP_LOGI(
              TAG,
              "Received: ID: 0x%X, DLC: %d, Data: %02X %02X %02X %02X %02X "
              "%02X %02X %02X",
              message.identifier,
              message.data_length_code,
              message.data[0],
              message.data[1],
              message.data[2],
              message.data[3],
              message.data[4],
              message.data[5],
              message.data[6],
              message.data[7]);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
      }
    }
  };

  extern "C" void app_main() {
    ESP_LOGI("APP", "Starting application");

    CANDriver driver;
    driver.init();
    driver.start_receive_task();
    driver.run();
  }