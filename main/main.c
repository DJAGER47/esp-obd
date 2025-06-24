/**
 * @file main.c
 * @brief Основной файл примера работы с INA226 и X9C103S
 *
 * Содержит инициализацию периферии и основной цикл работы:
 * - Мониторинг напряжения/тока через INA226
 * - Управление цифровым потенциометром X9C103S
 * - Управление светодиодом
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "ina226.h"
#include "x9c103s.h"

static const char *TAG = "example"; ///< Тег для логов

/// Конфигурация X9C103S
#define X9C103S_CS_PIN  7  ///< GPIO для выбора чипа
#define X9C103S_UD_PIN  6  ///< GPIO для направления изменения
#define X9C103S_INC_PIN 9  ///< GPIO для шага изменения

/// GPIO для светодиода (настраивается в menuconfig)
#define LED_GPIO CONFIG_BLINK_GPIO
static uint8_t s_led_state = 0; ///< Текущее состояние светодиода

/**
 * @brief Управление состоянием светодиода
 */
static void blink_led(void)
{
    gpio_set_level(LED_GPIO, s_led_state);
    ESP_LOGD(TAG, "LED state changed to %d", s_led_state);
}

/**
 * @brief Настройка GPIO для светодиода
 */
static void configure_led(void)
{
    ESP_LOGI(TAG, "Configuring GPIO LED on pin %d", LED_GPIO);
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "LED GPIO configured successfully");
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting application");

    ESP_LOGI(TAG, "Initializing INA226...");
    ina226_config_t ina_cfg = {
        .shunt_resistance = 0.1f, ///< Сопротивление шунта 0.1 Ом
        .max_current = 2.0f       ///< Максимальный ток 2A
    };
    esp_err_t ret = ina226_init(&ina_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "INA226 init failed");
    }
    ESP_LOGI(TAG, "INA226 initialized successfully");



    ESP_LOGI(TAG, "Initializing X9C103S...");
    x9c103s_dev_t pot = {
        .cs_pin = X9C103S_CS_PIN,
        .ud_pin = X9C103S_UD_PIN,
        .inc_pin = X9C103S_INC_PIN
    };
    ret = x9c103s_init(&pot);
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
    while (1) {
        float voltage, current, power;
        ret = ina226_read_values(&voltage, &current, &power);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Read failed");
            continue;
        }
        ESP_LOGI(TAG, "Current readings - V: %.2fV, I: %.2fA, P: %.2fW", voltage, current, power);
        
        s_led_state = !s_led_state;
        blink_led();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
