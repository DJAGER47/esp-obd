#include "x9c103s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "X9C103S"

static void pulse_pin(gpio_num_t pin) {
    gpio_set_level(pin, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(pin, 1);
    vTaskDelay(pdMS_TO_TICKS(1));
}

esp_err_t x9c103s_init(x9c103s_dev_t *dev) {
    // Настройка пинов как выходов
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << dev->cs_pin) | (1ULL << dev->ud_pin) | (1ULL << dev->inc_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) return err;

    // Инициализация потенциометра
    gpio_set_level(dev->cs_pin, 1);
    gpio_set_level(dev->ud_pin, 1); 
    gpio_set_level(dev->inc_pin, 1);

    return ESP_OK;
}

esp_err_t x9c103s_set_resistance(x9c103s_dev_t *dev, uint8_t value) {
    if (value > 99) value = 99;

    // Активируем чип
    gpio_set_level(dev->cs_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Устанавливаем направление (увеличение/уменьшение)
    gpio_set_level(dev->ud_pin, value < 50 ? 1 : 0);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Устанавливаем значение
    uint8_t steps = value < 50 ? value : (99 - value);
    for (int i = 0; i < steps; i++) {
        pulse_pin(dev->inc_pin);
    }

    // Деактивируем чип
    gpio_set_level(dev->cs_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(1));

    return ESP_OK;
}