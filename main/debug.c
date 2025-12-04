#include <stddef.h>
#include <stdint.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"

static const char * const TAG = "debug";


/**
 * @brief Выводит статистику выполнения задач и список задач.
 *
 * Функция выводит две таблицы:
 * 1. Статистика выполнения задач (Runtime Stats) - показывает время выполнения каждой задачи.
 * 2. Список задач (Task List) - показывает состояние, приоритет, номер ядра и использование стека для каждой задачи.
 *
 * Информация выводится через ESP_LOGI с тегом "debug".
 * Если не удалось выделить память для буфера статистики, выводится сообщение об ошибке.
 *
 * @note Функция выделяет динамическую память и освобождает её после использования.
 */
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

/**
 * @brief Выводит полную отладочную информацию о системе в виде единой таблицы.
 *
 * Функция выводит информацию о всех задачах в одной таблице:
 * - Имя задачи
 * - Использование стека (использовано/всего байт)
 * - Время выполнения (в процентах)
 *
 * Информация выводится через ESP_LOGI с тегом "debug".
 */
void print_debug_info(void) {
  ESP_LOGI(TAG, "=== Debug Information ===");
  
  UBaseType_t uxNumberOfTasks = uxTaskGetNumberOfTasks();
  ESP_LOGI(TAG, "Number of tasks: %lu", (unsigned long)uxNumberOfTasks);

  TaskStatus_t *pxTaskStatusArray = pvPortMalloc(uxNumberOfTasks * sizeof(TaskStatus_t));
  if (pxTaskStatusArray == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory");
    return;
  }

  uint32_t ulTotalRunTime;
  UBaseType_t uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxNumberOfTasks, &ulTotalRunTime);

  ESP_LOGI(TAG, "%-15s | %12s | %8s", "Task", "Stack (B)", "CPU%");
  ESP_LOGI(TAG, "----------------+--------------+---------");

  for (UBaseType_t x = 0; x < uxArraySize; x++) {
    UBaseType_t uxStackHighWaterMark = uxTaskGetStackHighWaterMark(pxTaskStatusArray[x].xHandle);
    uint32_t stack_used = pxTaskStatusArray[x].usStackHighWaterMark - uxStackHighWaterMark;
    
    uint32_t cpu_percent = (ulTotalRunTime > 0)
      ? (pxTaskStatusArray[x].ulRunTimeCounter * 100 / ulTotalRunTime) : 0;

    ESP_LOGI(TAG, "%-15s | %5lu/%5lu | %7lu",
             pxTaskStatusArray[x].pcTaskName,
             (unsigned long)stack_used,
             (unsigned long)pxTaskStatusArray[x].usStackHighWaterMark,
             (unsigned long)cpu_percent);
  }

  vPortFree(pxTaskStatusArray);
  ESP_LOGI(TAG, "=== End Debug Information ===");
}