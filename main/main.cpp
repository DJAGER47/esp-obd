#include <stdio.h>
#include <sys/lock.h>
#include <sys/param.h>
#include <unistd.h>

#include "can_subscriber.h"
#include "debug.h"
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

static const char *TAG = "main";

// LCD_BK_LIGHT_PIN
static UI ui_instance(LCD_SCLK_PIN, LCD_MOSI_PIN, LCD_RST_PIN, LCD_DC_PIN, LCD_CS_PIN, GPIO_NUM_NC);
TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с

extern "C" void app_main() {
  ESP_LOGI("APP", "Starting application");

  esp_err_t ret = ui_instance.init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize UI: %s", esp_err_to_name(ret));
    return;
  }

  // Инициализация CAN драйвера
  can_driver.InstallStart();

  ESP_LOGI(TAG, "Application initialized successfully");
  // uint8_t screen_state = 0;
  // uint32_t stack_info_counter = 0;

  while (1) {
    // ui_instance.switch_screen(screen_state);
    // screen_state = (screen_state + 1) % 2;

    // if (++stack_info_counter >= 5000) {
    //   print_debug_info();
    //   print_runtime_stats();
    //   ESP_LOGI(TAG, "\n\n\n");
    //   stack_info_counter = 0;
    // }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
