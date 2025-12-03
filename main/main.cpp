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

// Глобальный экземпляр CAN драйвера
TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с

// Функция обратного вызова для обработки CAN сообщений
void can_message_callback(const TwaiFrame &frame) {
  ui_instance.addCanMessageToQueue(frame);
}

// Подписчик на CAN сообщения
CanSubscriber can_subscriber(can_message_callback);

extern "C" void app_main() {
  ESP_LOGI("APP", "Starting application");

  esp_err_t ret = ui_instance.init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize UI: %s", esp_err_to_name(ret));
    return;
  }

  // Инициализация CAN драйвера
  can_driver.InstallStart();

  // Регистрация подписчика на CAN сообщения
  can_driver.RegisterSubscriber(can_subscriber);

  ESP_LOGI(TAG, "Application initialized successfully");
  // uint8_t screen_state = 0;
  // uint32_t stack_info_counter = 0;

  while (1) {
    // Обрабатываем сообщения из очереди CAN подписчика
    can_subscriber.ProcessMessages();

    // ui_instance.switch_screen(screen_state);
    // screen_state = (screen_state + 1) % 2;

    // if (++stack_info_counter >= 12) {
    //   print_stack_usage();
    //   print_runtime_stats();
    //   stack_info_counter = 0;
    // }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
