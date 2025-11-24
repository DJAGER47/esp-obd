
#include <stddef.h>
#include <stdint.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"

static const char * const TAG = "debug";

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