#include <stdio.h>
#include <sys/lock.h>
#include <sys/param.h>
#include <unistd.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "io.h"
#include "iso_tp.h"
#include "obd2.h"
#include "twai_driver.h"
#include "ui.h"

static UI ui_instance(LCD_SCLK_PIN, LCD_MOSI_PIN, LCD_RST_PIN, LCD_DC_PIN, LCD_CS_PIN, LCD_BK_LIGHT_PIN);

// Глобальный экземпляр CAN драйвера
// TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с

static const char *TAG = "OBD2";
extern "C" void app_main() {
  ESP_LOGI("APP", "Starting application");

  esp_err_t ret = ui_instance.init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize UI: %s", esp_err_to_name(ret));
    return;
  }

  ESP_LOGI(TAG, "Application initialized successfully");
  uint8_t gpio_state = 0;
  while (1) {
    ui_instance.switch_screen(gpio_state);
    gpio_state = gpio_state ? 0 : 1;
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
