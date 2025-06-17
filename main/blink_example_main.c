
/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "ina226.h"
#include "x9c103s.h"

static const char *TAG = "example";

// Конфигурация X9C103S
#define X9C103S_CS_PIN  7
#define X9C103S_UD_PIN  6
#define X9C103S_INC_PIN 9  // Изменено с GPIO5 из-за конфликта с Wi-Fi

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 0;

#ifdef CONFIG_BLINK_LED_STRIP

static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

#else
#error "unsupported LED type"
#endif

void app_main(void)
{
    /* Initialize INA226 with configuration */
    ina226_config_t ina_cfg = {
        .shunt_resistance = 0.1f,  // Сопротивление шунта 0.1 Ом
        .max_current = 2.0f        // Максимальный ток 2A
    };
    if (ina226_init(&ina_cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize INA226");
        return;  // Прерываем выполнение при ошибке инициализации
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

    while (1) {
        float voltage, current, power;
        if (ina226_read_values(&voltage, &current, &power) == ESP_OK) {
            ESP_LOGI(TAG, "Voltage: %.2fV, Current: %.2fA, Power: %.2fW",
                    voltage, current, power);
            
            // Регулируем сопротивление в зависимости от напряжения
            uint8_t res_value = (voltage > 3.3) ? 70 : 30;
            x9c103s_set_resistance(&pot, res_value);
        }

        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}
