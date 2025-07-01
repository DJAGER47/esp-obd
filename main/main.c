#include <stdio.h>

#include "driver/gpio.h"
#include "esp_freertos_hooks.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ina226.h"
#include "io.h"
#include "sdkconfig.h"
#include "x9c103s.h"

static const char *TAG = "MAIN";

static uint8_t s_led_state = 0;  ///< Текущее состояние светодиода

/**
 * @brief Функция, выполняемая в idle-режиме
 * @return true - продолжить idle, false - выйти
 */
bool idle_hook(void) {
  static uint32_t counter = 0;

  // Проверяем состояние каждые 100 вызовов
  if (++counter % 100 == 0) {
    // Мониторинг свободной памяти
    ESP_LOGD(TAG, "Free heap: %d bytes", esp_get_free_heap_size());

    // Мониторинг минимального свободного стека
    ESP_LOGD(
        TAG, "Minimum free stack: %d bytes", uxTaskGetStackHighWaterMark(NULL));
  }

  // Переход в light sleep с пробуждением по событиям
  // esp_sleep_enable_timer_wakeup(pdMS_TO_TICKS(1) * portTICK_PERIOD_MS *
  // 1000); // 1 тик ОС
  esp_sleep_enable_timer_wakeup(1000000);
  esp_light_sleep_start();

  return false;
}

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

void app_main(void) {
  ESP_LOGI(TAG, "Starting application");

  // Регистрация idle hook
  esp_err_t ret = esp_register_freertos_idle_hook(idle_hook);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register idle hook");
  } else {
    ESP_LOGI(TAG, "Idle hook registered successfully");
  }

  ESP_LOGI(TAG, "Initializing INA226...");
  ina226_config_t ina_cfg = {
      .shunt_resistance = 0.1f,  ///< Сопротивление шунта 100 мл.Ом
      .max_current      = 1      ///< Максимальный ток
  };
  ina226_init(&ina_cfg);
  ESP_LOGI(TAG, "INA226 initialized successfully");

  ESP_LOGI(TAG, "Initializing X9C103S...");
  x9c103s_dev_t pot = {.cs_pin  = X9C103S_CS_PIN,
                       .ud_pin  = X9C103S_UD_PIN,
                       .inc_pin = X9C103S_INC_PIN};
  ret               = x9c103s_init(&pot);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "X9C103S init failed");
  }
  ESP_LOGI(TAG, "X9C103S initialized successfully");

  configure_led();

  //-----------------------------------------------------------------------

  // float max_power = 0;
  // uint8_t optimal_resistance = 99; // Начинаем с максимального сопротивления

  // ESP_LOGI(TAG, "Starting resistance optimization...");
  // x9c103s_set_resistance(&pot, optimal_resistance);
  // vTaskDelay(100 / portTICK_PERIOD_MS);

  // // Поиск оптимального сопротивления (шаг 10%)
  // for (int res = 99; res >= 0; res -= 10)
  // {
  //     float voltage, current, power;

  //     // Устанавливаем текущее сопротивление
  //     x9c103s_set_resistance(&pot, res);
  //     vTaskDelay(50 / portTICK_PERIOD_MS);

  //     // Читаем показания
  //     if (ina226_read_values(&voltage, &current, &power) == ESP_OK)
  //     {
  //         // Логируем данные
  //         ESP_LOGI(TAG, "Resistance: %d%%, V: %.2fV, I: %.2fA, P: %.2fW",
  //                 res, voltage, current, power);

  //         // Ищем максимальную мощность
  //         if (power > max_power)
  //         {
  //             max_power = power;
  //             optimal_resistance = res;
  //         }

  //         // Мигаем светодиодом
  //         s_led_state = !s_led_state;
  //         blink_led();
  //     }
  //     vTaskDelay(500 / portTICK_PERIOD_MS);
  // }

  // // Устанавливаем оптимальное сопротивление
  // x9c103s_set_resistance(&pot, optimal_resistance);
  // ESP_LOGI(TAG, "Optimal resistance: %d%%, Max power: %.2fW",
  //          optimal_resistance, max_power);

  // Основной цикл работы
  //-----------------------------------------------------------------------

  while (1) {
    float voltage, current, power;
    ina226_read_values(&voltage, &current, &power);
    printf("Current readings - V: %.2fV, I: %.2fA, P: %.2fW\n\r",
           voltage,
           current,
           power);

    s_led_state = !s_led_state;
    blink_led();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
