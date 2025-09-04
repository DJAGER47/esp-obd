#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "io.h"
#include "iso-tp.h"
#include "obd2.h"
#include "twai_driver.h"

// Глобальный экземпляр CAN драйвера
TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с

// Функция для нового потока
void iso_tp_task(void *pvParameters) {
  // Создаем экземпляр IsoTp, передавая ссылку на CAN драйвер
  ITwaiInterface &twai_interface = can_driver;
  IsoTp iso_tp(twai_interface);
  OBD2 obd(&iso_tp);

  // Здесь можно добавить логику работы с IsoTp
  // Например, отправка и прием сообщений
  while (true) {
    // Добавьте здесь логику работы с ISO-TP
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static const char *TAG = "OBD2";
extern "C" void app_main() {
  ESP_LOGI("APP", "Starting application");

  // Инициализация CAN драйвера
  TwaiError err = can_driver.install_and_start();
  if (err != TwaiError::OK) {
    ESP_LOGE(TAG,
             "Failed to install and start TWAI driver: %d",
             static_cast<int>(err));
    return;
  }

  // Инициализация OBD2
  // if (!obd.begin(can_driver, true, 1000)) {
  //   ESP_LOGE(TAG, "Failed to initialize OBD2");
  //   return;
  // }

  // Создание нового потока для работы с ISO-TP
  xTaskCreate(iso_tp_task, "iso_tp_task", 4096, NULL, 5, NULL);

  while (true) {
    // twai_message_t message;
    // if (driver.receiveMessage(message)) {
    //   OBD2::PID pid = static_cast<OBD2::PID>(message.data[1]);
    //   parser.parsePID(pid, message);
    // }
    // parser.printTripData();

    // Пример использования OBD2 для запроса RPM двигателя
    // obd.queryPID(0x01, 0x0C);  // Сервис 01, PID 0C (RPM)
    // float rpm = obd.rpm();
    // ESP_LOGI(TAG, "Engine RPM: %.2f", rpm);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}