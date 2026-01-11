#include <stdio.h>
#include <sys/lock.h>
#include <sys/param.h>
#include <unistd.h>

#include <functional>
#include <optional>

#include "debug.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "io.h"
#include "iso_tp.h"
#include "obd2.h"
#include "obd_data_polling.h"
#include "reset_handler.h"
#include "twai_driver.h"
#include "ui.h"

static const char* const TAG = "main";

// LCD_BK_LIGHT_PIN
static UI ui_instance(LCD_SCLK_PIN, LCD_MOSI_PIN, LCD_RST_PIN, LCD_DC_PIN, LCD_CS_PIN, GPIO_NUM_NC);
static TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с
static IsoTp iso_tp(can_driver);                            // ISO-TP протокол поверх CAN
static OBD2 obd2(iso_tp);                                   // OBD2 поверх ISO-TP

// Глобальные переменные для доступа из задачи опроса OBD2
OBD2* obd2_instance = &obd2;
UI* ui_instance_ptr = &ui_instance;

// Дескриптор задачи опроса данных OBD2
static TaskHandle_t obd_polling_task_handle = nullptr;

extern "C" void app_main() {
  // Проверяем причину перезагрузки
  // if (!check_reset_reason()) {
  //   ESP_LOGE(TAG, "System reset due to error detected. Entering error loop.");
  //   while (1) {
  //     ESP_LOGE(TAG, "ERROR LOOP: System was reset due to a fault. Manual reset required.");
  //     vTaskDelay(pdMS_TO_TICKS(5000));
  //   }
  //   return;
  // }

  ESP_LOGI("APP", "Starting application");

  esp_err_t ret = ui_instance.init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize UI: %s", esp_err_to_name(ret));
    return;
  }

  // Инициализация CAN драйвера
  can_driver.InstallStart();

  ESP_LOGI(TAG, "Application initialized successfully");

  // Даем время на инициализацию CAN шины
  vTaskDelay(pdMS_TO_TICKS(2000));

  // Запрашиваем все поддерживаемые PID и выводим в бинарном виде
  ESP_LOGI(TAG, "Querying all supported PIDs...");

  // Массив функций для запроса поддерживаемых PID в разных диапазонах
  struct PidRange {
    const char* name;
    std::function<std::optional<uint32_t>()> query_func;
  };

  // clang-format off
  PidRange pid_ranges[] = {
    {"    1-20", [&]() { return obd2.supportedPIDs_1_20(); }},
    {"   21-40", [&]() { return obd2.supportedPIDs_21_40(); }},
    {"   41-60", [&]() { return obd2.supportedPIDs_41_60(); }},
    {"   61-80", [&]() { return obd2.supportedPIDs_61_80(); }},
    {"  81-100", [&]() { return obd2.supportedPIDs81_100(); }},
    {" 101-120", [&]() { return obd2.supportedPIDs101_120(); }},
    {" 121-140", [&]() { return obd2.supportedPIDs121_140(); }},
    {"Service9", [&]() { return obd2.supportedPIDs_Service09(); }}
  };
  // clang-format on

  // Запрашиваем поддерживаемые PID в цикле
  for (const auto& range : pid_ranges) {
    auto pids = range.query_func();
    if (pids.has_value()) {
      ESP_LOGI(TAG, "Supported PIDs %s: 0x%08X", range.name, pids.value());
    } else {
      ESP_LOGW(TAG, "Failed to read supported PIDs %s", range.name);
    }
  }

  // Запускаем задачу опроса данных OBD2
  BaseType_t result = xTaskCreate(obd_polling_task,         // Функция задачи
                                  "obd_poll",               // Имя задачи
                                  4096,                     // Размер стека
                                  nullptr,                  // Параметр задачи
                                  5,                        // Приоритет
                                  &obd_polling_task_handle  // Дескриптор задачи
  );

  if (result != pdPASS) {
    ESP_LOGE(TAG, "Failed to create OBD polling task");
    return;
  }

  ESP_LOGI(TAG, "OBD polling task started successfully");

  // Основной цикл приложения
  // uint8_t screen_state        = 0;
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

    // Задержка в основном цикле
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
