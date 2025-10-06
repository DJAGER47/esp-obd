#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "time.h"

class FreeRtosMutex {
 public:
  void Create();
  bool Lock(TickType_t timeout = portMAX_DELAY);
  bool UnLock();

 private:
  SemaphoreHandle_t mutex_handle_;
  StaticSemaphore_t mutex_buffer_;
};

class FreeRtosLockGuard {
 public:
  FreeRtosLockGuard(const FreeRtosLockGuard&)            = delete;
  FreeRtosLockGuard& operator=(const FreeRtosLockGuard&) = delete;

  explicit FreeRtosLockGuard(FreeRtosMutex& mutex, TickType_t timeout = portMAX_DELAY);
  ~FreeRtosLockGuard();

 private:
  FreeRtosMutex& mutex_;
  bool locked_;
};
