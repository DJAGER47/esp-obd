#include "obd_data_polling.h"

#include "esp_log.h"
#include "iso_tp.h"
#include "obd2.h"
#include "twai_driver.h"
#include "ui.h"

static const char* const TAG = "obd_polling";

// Глобальные переменные для доступа из задачи
extern UI* ui_instance_ptr;

void obd_polling_task(void* arg) {
  ESP_LOGI(TAG, "Starting OBD data polling loop");

  // Получаем can_driver из параметра задачи
  TwaiDriver* can_driver = static_cast<TwaiDriver*>(arg);
  if (can_driver == nullptr) {
    ESP_LOGE(TAG, "can_driver parameter is null");
    vTaskDelete(nullptr);
    return;
  }

  IsoTp iso_tp(*can_driver);  // ISO-TP протокол поверх CAN
  OBD2 obd2(iso_tp);          // OBD2 поверх ISO-TP

  float rpm_value        = 0.0;
  int speed_value        = 0;
  int coolant_temp_value = 0;

  while (1) {
    // Запрашиваем обороты двигателя
    auto rpm = obd2.rpm();
    if (rpm.has_value()) {
      rpm_value = rpm.value();
    } else {
      ESP_LOGW(TAG, "Failed to read engine RPM");
    }

    // Запрашиваем скорость автомобиля
    auto speed = obd2.kph();
    if (speed.has_value()) {
      speed_value = speed.value();
    } else {
      ESP_LOGW(TAG, "Failed to read vehicle speed");
    }

    // Запрашиваем температуру охлаждающей жидкости
    auto coolant_temp = obd2.engineCoolantTemp();
    if (coolant_temp.has_value()) {
      coolant_temp_value = coolant_temp.value();
    } else {
      ESP_LOGW(TAG, "Failed to read coolant temperature");
    }

    // Обновляем экран с данными OBD2
    ui_instance_ptr->update_screen0(rpm_value, speed_value, coolant_temp_value);

    // Задержка перед следующим опросом
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}