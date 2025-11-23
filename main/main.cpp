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

static const char *TAG = "main";

void print_stack_usage(void) {
  ESP_LOGI(TAG, "=== Stack Usage Information ===");

  // Получаем количество задач
  UBaseType_t uxNumberOfTasks = uxTaskGetNumberOfTasks();
  ESP_LOGI(TAG, "Number of tasks: %lu", (unsigned long)uxNumberOfTasks);

  // Выделяем память для массива статусов задач
  TaskStatus_t *pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc(uxNumberOfTasks * sizeof(TaskStatus_t));

  if (pxTaskStatusArray != NULL) {
    // Получаем информацию о всех задачах
    UBaseType_t uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxNumberOfTasks, NULL);

    // Выводим информацию о каждой задаче
    for (UBaseType_t x = 0; x < uxArraySize; x++) {
      // Получаем максимальное использование стека для задачи
      UBaseType_t uxStackHighWaterMark = uxTaskGetStackHighWaterMark(pxTaskStatusArray[x].xHandle);

      // Рассчитываем процент использования стека
      uint32_t stack_usage_percent = 0;
      if (pxTaskStatusArray[x].usStackHighWaterMark > 0) {
        stack_usage_percent = 100 - (uxStackHighWaterMark * 100 / pxTaskStatusArray[x].usStackHighWaterMark);
      }

      ESP_LOGI(TAG,
               "Task: %-15s | Stack: %4lu/%4lu bytes | Usage: %3lu%% | Priority: %u",
               pxTaskStatusArray[x].pcTaskName,
               (unsigned long)(pxTaskStatusArray[x].usStackHighWaterMark - uxStackHighWaterMark),
               (unsigned long)pxTaskStatusArray[x].usStackHighWaterMark,
               (unsigned long)stack_usage_percent,
               pxTaskStatusArray[x].uxCurrentPriority);
    }

    // Освобождаем память
    vPortFree(pxTaskStatusArray);
  } else {
    ESP_LOGE(TAG, "Failed to allocate memory for task status array");
  }

  ESP_LOGI(TAG, "=== End Stack Usage Information ===");
}

void print_runtime_stats(void) {
  ESP_LOGI(TAG, "=== Runtime Statistics ===");

  // Выделяем буфер для статистики
  char *stats_buffer = (char *)pvPortMalloc(1024);

  if (stats_buffer != NULL) {
    // Получаем статистику выполнения задач
    vTaskGetRunTimeStats(stats_buffer);
    ESP_LOGI(TAG, "Runtime Stats:\n%s", stats_buffer);

    // Получаем список задач
    vTaskList(stats_buffer);
    ESP_LOGI(TAG, "Task List:\n%s", stats_buffer);

    // Освобождаем память
    vPortFree(stats_buffer);
  } else {
    ESP_LOGE(TAG, "Failed to allocate memory for stats buffer");
  }

  ESP_LOGI(TAG, "=== End Runtime Statistics ===");
}

static UI ui_instance(LCD_SCLK_PIN, LCD_MOSI_PIN, LCD_RST_PIN, LCD_DC_PIN, LCD_CS_PIN, LCD_BK_LIGHT_PIN);

// Глобальный экземпляр CAN драйвера
// TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с

extern "C" void app_main() {
  ESP_LOGI("APP", "Starting application");

  esp_err_t ret = ui_instance.init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize UI: %s", esp_err_to_name(ret));
    return;
  }

  ESP_LOGI(TAG, "Application initialized successfully");
  uint8_t screen_state        = 0;
  uint32_t stack_info_counter = 0;

  while (1) {
    ui_instance.switch_screen(screen_state);
    screen_state = (screen_state + 1) % 2;

    if (++stack_info_counter >= 12) {
      print_stack_usage();
      print_runtime_stats();
      stack_info_counter = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
