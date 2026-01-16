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

static const char* const TAG = "main";

// Выбор типа UI: 0 - ST7789, 1 - LD7138
#define USE_LD7138_DISPLAY 0

#if USE_LD7138_DISPLAY
#include "ui2.h"
// LCD_BK_LIGHT_PIN для LD7138
static UI2 ui_instance(LCD_SCLK_PIN, LCD_MOSI_PIN, LCD_RST_PIN, LCD_DC_PIN, LCD_CS_PIN, GPIO_NUM_NC);
UI2* ui_instance_ptr = &ui_instance;
#else
#include "ui.h"
// LCD_BK_LIGHT_PIN для ST7789
static UI ui_instance(LCD_SCLK_PIN, LCD_MOSI_PIN, LCD_RST_PIN, LCD_DC_PIN, LCD_CS_PIN, GPIO_NUM_NC);
UI* ui_instance_ptr = &ui_instance;
#endif

static TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с

// Дескриптор задачи опроса данных OBD2
static constexpr uint32_t kObdPollingTaskStackSize = 4096;
static TaskHandle_t obd_polling_task_handle        = nullptr;
static StackType_t obd_polling_task_stack[kObdPollingTaskStackSize];
static StaticTask_t obd_polling_task_tcb;

struct PidRange {
  const char* name;
  std::function<std::optional<uint32_t>(OBD2*)> query_func;
};

void ServicesPoolingTask() {
  // Создаем временные объекты для опроса PID
  IsoTp iso_tp(can_driver);  // ISO-TP протокол поверх CAN
  OBD2 obd2(iso_tp);         // OBD2 поверх ISO-TP

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
  // uint32_t pids_cache[kPidRangesCount];

  while (1) {
    size_t count = 0;
    // Запрашиваем поддерживаемые PID в цикле
    for (size_t i = 0; i < kPidRangesCount; ++i) {
      auto pids = pid_ranges[i].query_func(&obd2);
      if (pids.has_value()) {
        // pids_cache[i] = pids.value();
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
  ui_instance.Init();
  can_driver.InstallStart();
  ESP_LOGI(TAG, "Application initialized successfully");
  vTaskDelay(pdMS_TO_TICKS(2000));

  while (1) {
    ESP_LOGI(TAG, "Querying all supported PIDs...");
    ServicesPoolingTask();

    obd_polling_task_handle = xTaskCreateStatic(obd_polling_task,          // Функция задачи
                                                "obd_poll",                // Имя задачи
                                                kObdPollingTaskStackSize,  // Размер стека
                                                &can_driver,  // Параметр задачи - адрес can_driver
                                                5,            // Приоритет
                                                obd_polling_task_stack,  // Буфер стека
                                                &obd_polling_task_tcb    // Структура TCB
    );
    if (obd_polling_task_handle == nullptr) {
      ESP_LOGI(TAG, "obd_polling_task not created. Restarting in 5 seconds...");
      esp_rom_delay_us(5000000);
      esp_restart();
    }

    // Основной цикл приложения
    uint32_t can_error_rx = 0;
    uint32_t can_error_tx = 0;
    while (1) {
      const uint32_t can_error_rx_now  = can_driver.GetRxErrorCount();
      const uint32_t can_error_tx_now  = can_driver.GetTxErrorCount();
      const uint32_t can_error_rx_diff = can_error_rx_now - can_error_rx;
      const uint32_t can_error_tx_diff = can_error_tx_now - can_error_tx;
      const uint32_t can_error_diff    = can_error_rx_diff + can_error_tx_diff;

      ESP_LOGW(TAG, "CAN drop (rx: %d, tx: %d)", can_error_rx_now, can_error_tx_now);
      can_error_rx = can_error_rx_now;
      can_error_tx = can_error_tx_now;

      if (can_error_diff > 5) {
        vTaskDelete(obd_polling_task_handle);
        ESP_LOGE(
            TAG, "CAN error detected: diff %d (rx: %d, tx: %d)", can_error_diff, can_error_rx_diff, can_error_tx_diff);
        break;
      }
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}
