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
#include "ui2.h"

static const char* const TAG = "main";

// Выбор типа UI: 0 - ST7789, 1 - LD7138
#define USE_LD7138_DISPLAY 0

#if USE_LD7138_DISPLAY
// LCD_BK_LIGHT_PIN для LD7138
static UI2 ui_instance(LCD_SCLK_PIN, LCD_MOSI_PIN, LCD_RST_PIN, LCD_DC_PIN, LCD_CS_PIN, GPIO_NUM_NC);
#else
// LCD_BK_LIGHT_PIN для ST7789
static UI ui_instance(LCD_SCLK_PIN, LCD_MOSI_PIN, LCD_RST_PIN, LCD_DC_PIN, LCD_CS_PIN, GPIO_NUM_NC);
#endif

static TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с
static IsoTp iso_tp(can_driver);                            // ISO-TP протокол поверх CAN
static OBD2 obd2(iso_tp);                                   // OBD2 поверх ISO-TP

// Глобальные переменные для доступа из задачи опроса OBD2
OBD2* obd2_instance = &obd2;
#if USE_LD7138_DISPLAY
UI2* ui_instance_ptr = &ui_instance;
#else
UI* ui_instance_ptr = &ui_instance;
#endif

// Дескриптор задачи опроса данных OBD2
static TaskHandle_t obd_polling_task_handle = nullptr;

struct PidRange {
  const char* name;
  std::function<std::optional<uint32_t>(OBD2*)> query_func;
};

void ServicesPoolingTask() {
  // Массив функций для запроса поддерживаемых PID в разных диапазонах
  // clang-format off
  PidRange pid_ranges[] = {
    {"    1-20", [](OBD2* obd) { return obd->supportedPIDs_1_20(); }},
    {"   21-40", [](OBD2* obd) { return obd->supportedPIDs_21_40(); }},
    {"   41-60", [](OBD2* obd) { return obd->supportedPIDs_41_60(); }},
    {"   61-80", [](OBD2* obd) { return obd->supportedPIDs_61_80(); }},
    {"  81-100", [](OBD2* obd) { return obd->supportedPIDs81_100(); }},
    {" 101-120", [](OBD2* obd) { return obd->supportedPIDs101_120(); }},
    {" 121-140", [](OBD2* obd) { return obd->supportedPIDs121_140(); }},
    {"Service9", [](OBD2* obd) { return obd->supportedPIDs_Service09(); }}
  };
  // clang-format on
  constexpr size_t kPidRangesCount = sizeof(pid_ranges) / sizeof(pid_ranges[0]);
  uint32_t pids_cache[kPidRangesCount];

  while (1) {
    size_t count = 0;
    // Запрашиваем поддерживаемые PID в цикле
    for (size_t i = 0; i < kPidRangesCount; ++i) {
      auto pids = pid_ranges[i].query_func(obd2_instance);
      if (pids.has_value()) {
        pids_cache[i] = pids.value();
        ESP_LOGI(TAG, "Supported PIDs %s: 0x%08X", pid_ranges[i].name, pids.value());
        count++;
      } else {
        ESP_LOGW(TAG, "Failed to read supported PIDs %s", pid_ranges[i].name);
      }
    }

    if (count == kPidRangesCount) {
      ESP_LOGI(TAG, "All supported PIDs read successfully");
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

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

  ServicesPoolingTask();

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
