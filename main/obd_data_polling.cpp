#include "obd_data_polling.h"

#include "esp_log.h"
#include "iso_tp.h"
#include "obd2.h"
#include "twai_driver.h"
#include "ui.h"
#include "vehicle_params.h"

static const char* const TAG = "obd_polling";

// Глобальные переменные для доступа из задачи
extern VehicleParams vehicle_params;

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

  while (1) {
    // Запрашиваем обороты двигателя
    auto rpm = obd2.rpm();
    if (rpm.has_value()) {
      vehicle_params.setRpm(rpm.value());
    } else {
      ESP_LOGW(TAG, "Failed to read engine RPM");
    }

    // Запрашиваем скорость автомобиля
    auto speed = obd2.kph();
    if (speed.has_value()) {
      vehicle_params.setSpeed(speed.value());
    } else {
      ESP_LOGW(TAG, "Failed to read vehicle speed");
    }

    // Запрашиваем температуру охлаждающей жидкости
    auto coolant_temp = obd2.engineCoolantTemp();
    if (coolant_temp.has_value()) {
      vehicle_params.setCoolantTemp(coolant_temp.value());
    } else {
      ESP_LOGW(TAG, "Failed to read coolant temperature");
    }

    // Задержка перед следующим опросом
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}