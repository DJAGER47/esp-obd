#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <queue>

#include "phy_interface.h"
#include "task.h"

// Типы данных FreeRTOS
using BaseType_t    = int32_t;
using UBaseType_t   = uint32_t;
using QueueHandle_t = void*;

// Структура для хранения элемента очереди
struct QueueItem {
  uint8_t data[64];  // Буфер для данных
  size_t size;       // Размер данных
};

// Структура для хранения очереди с информацией о размере
struct Queue {
  std::queue<QueueItem> items;
  UBaseType_t max_size;
};

// Глобальная переменная для хранения очередей
inline std::queue<Queue*> g_queues;

// Создание очереди
inline QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize) {
  auto queue      = new Queue();
  queue->max_size = uxQueueLength;
  g_queues.push(queue);
  return static_cast<QueueHandle_t>(queue);
}

// Отправка элемента в очередь
inline BaseType_t xQueueSend(QueueHandle_t xQueue,
                             const void* pvItemToQueue,
                             TickType_t xTicksToWait) {
  if (xQueue == nullptr) {
    return pdFALSE;
  }

  auto queue = static_cast<Queue*>(xQueue);

  // Проверка на переполнение
  if (queue->items.size() >= queue->max_size) {
    return pdFALSE;
  }

  // Создаем новый элемент очереди
  QueueItem item;
  memset(item.data, 0, sizeof(item.data));

  // Копируем данные в элемент очереди
  const TwaiFrame* frame = static_cast<const TwaiFrame*>(pvItemToQueue);
  memcpy(item.data, frame, sizeof(TwaiFrame));
  item.size = sizeof(TwaiFrame);

  // Добавляем элемент в очередь
  queue->items.push(item);

  return pdTRUE;
}

// Отправка элемента в очередь из ISR
inline BaseType_t xQueueSendFromISR(QueueHandle_t xQueue,
                                    const void* pvItemToQueue,
                                    BaseType_t* pxHigherPriorityTaskWoken) {
  if (pxHigherPriorityTaskWoken != nullptr) {
    *pxHigherPriorityTaskWoken = pdFALSE;
  }
  return xQueueSend(xQueue, pvItemToQueue, 0);
}

// Получение элемента из очереди
inline BaseType_t xQueueReceive(QueueHandle_t xQueue, void* pvBuffer, TickType_t xTicksToWait) {
  if (xQueue == nullptr) {
    return pdFALSE;
  }

  auto queue = static_cast<Queue*>(xQueue);

  // Проверка на пустую очередь
  if (queue->items.empty()) {
    return pdFALSE;
  }

  // Получаем первый элемент из очереди
  QueueItem item = queue->items.front();
  queue->items.pop();

  // Копируем данные в буфер
  TwaiFrame* frame = static_cast<TwaiFrame*>(pvBuffer);
  memcpy(frame, item.data, sizeof(TwaiFrame));

  return pdTRUE;
}

// Удаление очереди
inline void vQueueDelete(QueueHandle_t xQueue) {
  if (xQueue != nullptr) {
    auto queue = static_cast<Queue*>(xQueue);
    delete queue;
  }
}