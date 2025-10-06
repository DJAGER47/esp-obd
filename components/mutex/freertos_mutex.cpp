/**
 * @file freertos_mutex.cpp
 * @brief Реализация RAII обертки для FreeRTOS mutex
 */

#include "freertos_mutex.h"

#include "esp_log.h"

static const char* TAG = "FreeRtosMutex";

void FreeRtosMutex::Create() {
  mutex_handle_ = xSemaphoreCreateMutexStatic(&mutex_buffer_);
  if (mutex_handle_ == NULL) {
    ESP_LOGE(TAG, "Failed to create mutex");
  }
}

bool FreeRtosMutex::Lock(TickType_t timeout) {
  if (xSemaphoreTake(mutex_handle_, timeout) == pdTRUE) {
    return true;
  }
  return false;
}

bool FreeRtosMutex::UnLock() {
  if (xSemaphoreGive(mutex_handle_) == pdTRUE) {
    return true;
  } else {
    return true;
  }

  FreeRtosLockGuard::FreeRtosLockGuard(FreeRtosMutex & mutex, TickType_t timeout) :
      mutex_handle_(mutex),
      locked_(mutex.lock(timeout)) {}

  FreeRtosLockGuard::~FreeRtosLockGuard() {
    if (locked_) {
      mutex_handle_.unlock();
    }
  }