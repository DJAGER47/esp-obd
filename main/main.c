#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "ina226.h"
#include "x9c103s.h"

static const char *TAG = "example";

// Конфигурация X9C103S
#define X9C103S_CS_PIN 7
#define X9C103S_UD_PIN 6
#define X9C103S_INC_PIN 9

// GPIO для светодиода
#define LED_GPIO CONFIG_BLINK_GPIO
static uint8_t s_led_state = 0;

static void blink_led(void)
{
    gpio_set_level(LED_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Configuring GPIO LED on pin %d", LED_GPIO);
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    /* Initialize INA226 with configuration */
    ina226_config_t ina_cfg = {
        .shunt_resistance = 0.1f, // Сопротивление шунта 0.1 Ом
        .max_current = 2.0f       // Максимальный ток 2A
    };

    if (ina226_init(&ina_cfg) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize INA226");
        return; // Прерываем выполнение при ошибке инициализации
    }

    /* Initialize X9C103S */
    x9c103s_dev_t pot = {
        .cs_pin = X9C103S_CS_PIN,
        .ud_pin = X9C103S_UD_PIN,
        .inc_pin = X9C103S_INC_PIN
    };
    if (x9c103s_init(&pot) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize X9C103S");
    }

    /* Configure the peripheral according to the LED type */
    configure_led();

    // float max_power = 0;
    // uint8_t optimal_resistance = 0;

    // // Начинаем с максимального сопротивления
    // x9c103s_set_resistance(&pot, 99);
    // vTaskDelay(100 / portTICK_PERIOD_MS);

    // for (int res = 99; res >= 0; res -= 10)
    // {
    //     float voltage, current, power;

    //     // Устанавливаем текущее сопротивление
    //     x9c103s_set_resistance(&pot, res);
    //     vTaskDelay(50 / portTICK_PERIOD_MS);

    //     // Читаем показания
    //     if (ina226_read_values(&voltage, &current, &power) == ESP_OK)
    //     {
    //         // Логируем данные для Serial Plotter
    //         printf("RES:%d,V:%.2f,I:%.2f,P:%.2f\n",
    //                res, voltage, current, power);

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
    // ESP_LOGI(TAG, "Optimal resistance: %d, Max power: %.2fW",
    //          optimal_resistance, max_power);

    while (1) {
        // Устанавливаем оптимальное сопротивление
        // x9c103s_set_resistance(&pot, optimal_resistance);
        
        float voltage, current, power;
        if (ina226_read_values(&voltage, &current, &power) == ESP_OK) {
            printf("OPTIMAL: V:%.2f, I:%.2f, P:%.2f\n",
                   voltage, current, power);
        }
        
        // Моргаем светодиодом
        s_led_state = !s_led_state;
        blink_led();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
