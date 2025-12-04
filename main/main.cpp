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

// Переменные для отслеживания последовательности пакетов
static uint32_t expected_packet_index = 0;

// LCD_BK_LIGHT_PIN
static UI ui_instance(LCD_SCLK_PIN, LCD_MOSI_PIN, LCD_RST_PIN, LCD_DC_PIN, LCD_CS_PIN, GPIO_NUM_NC);

// Глобальный экземпляр CAN драйвера
TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с

// Функция обратного вызова для обработки CAN сообщений
void can_message_callback(const TwaiFrame &frame) {
  // Проверяем, что это сообщение с идентификатором 0x123 (наш тестовый идентификатор)
  if (frame.id == 0x123 && frame.data_length == 8) {
    // Извлекаем номер пакета из первых 4 байт (little-endian)
    uint32_t packet_index = (uint32_t)frame.data[0] | ((uint32_t)frame.data[1] << 8) | ((uint32_t)frame.data[2] << 16) |
                            ((uint32_t)frame.data[3] << 24);

    // Проверяем последовательность пакетов
    if (packet_index != expected_packet_index) {
      ESP_LOGW(TAG,
               "Packet sequence mismatch: expected %lu, got %lu (diff: %ld)",
               expected_packet_index,
               packet_index,
               (int32_t)(packet_index - expected_packet_index));
      // Обновляем ожидаемый индекс
      expected_packet_index = packet_index + 1;
    } else {
      // Инкрементируем ожидаемый индекс для следующего пакета
      expected_packet_index++;
    }
  }
  TwaiFrame frame_send = frame;
  frame_send.id        = 0x321;
  can_driver.Transmit(frame_send, 10);
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

    // if (++stack_info_counter >= 5000) {
    //   print_debug_info();
    //   print_runtime_stats();
    //   ESP_LOGI(TAG, "\n\n\n");
    //   stack_info_counter = 0;
    // }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
