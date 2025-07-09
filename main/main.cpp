#include <stdio.h>

#include "LEDController.hpp"
#include "PIDParser.hpp"
#include "RingBuffer.hpp"
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
      led(gpio_num_t(8)) {}  // GPIO8 для светодиода

  void init() {
    // Конфигурация TWAI драйвера
    twai_general_config_t g_config =
        TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL);
    g_config.tx_queue_len   = 5;
    g_config.rx_queue_len   = 20;
    g_config.alerts_enabled = TWAI_ALERT_RX_DATA;

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
  RingBuffer<twai_message_t, RX_BUFFER_SIZE> rx_buffer;
  PIDParser pid_parser;

  static void IRAM_ATTR isr_handler(void* arg) {
    CANDriver* self = static_cast<CANDriver*>(arg);
    twai_message_t message;

    while (twai_receive(&message, 0) == ESP_OK) {
      self->rx_buffer.push(message);
    }
  }

  static void receive_task(void* arg) {
    CANDriver* self = static_cast<CANDriver*>(arg);

    while (true) {
      twai_message_t message;
      if (self->rx_buffer.pop(&message)) {
        if (message.identifier == OBD2_CAN_ID) {
          // Определяем PID (первый байт данных)
          uint32_t pid = message.data[0];
          float value  = self->pid_parser.parsePID(pid, message);

          ESP_LOGI(TAG,
                   "PID: 0x%X, Value: %.2f, Data: %02X %02X %02X %02X %02X "
                   "%02X %02X %02X",
                   pid,
                   value,
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
  }
};

extern "C" void app_main() {
  ESP_LOGI("APP", "Starting application");

  CANDriver driver;
  driver.init();
  driver.start_receive_task();
  driver.run();
}