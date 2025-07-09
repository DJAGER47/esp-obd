#include <stdio.h>

#include "driver/gpio.h"
#include "driver/twai.h"
#include "esp_freertos_hooks.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "io.h"
#include "sdkconfig.h"

static const char *TAG     = "CAN_DRIVER";
static uint8_t s_led_state = 0;
/**
 * @brief Управление состоянием светодиода
 */
static void blink_led(void) {
  gpio_set_level(LED_GPIO, s_led_state);
  ESP_LOGD(TAG, "LED state changed to %d", s_led_state);
}

/**
 * @brief Настройка GPIO для светодиода
 */
static void configure_led(void) {
  ESP_LOGI(TAG, "Configuring GPIO LED on pin %d", LED_GPIO);
  gpio_reset_pin(LED_GPIO);
  gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
  ESP_LOGI(TAG, "LED GPIO configured successfully");
}

void can_init() {
  // Конфигурация TWAI драйвера
  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL);
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

void can_receive_task(void *arg) {
  twai_message_t message;
  while (1) {
    // Получение сообщений из очереди
    if (twai_receive(&message, pdMS_TO_TICKS(100)) == ESP_OK) {
      ESP_LOGI(TAG,
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
  }
}

void app_main() {
  ESP_LOGI(TAG, "Starting application");
  configure_led();

  can_init();
  xTaskCreate(can_receive_task, "CAN_RX_TASK", 4096, NULL, 5, NULL);

  while (1) {
    s_led_state = !s_led_state;
    blink_led();
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}