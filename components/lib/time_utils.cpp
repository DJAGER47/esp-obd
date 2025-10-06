#include "time_utils.h"

#include "freertos/FreeRTOS.h"

TickType_t convertToFreeRtosTick(Time_ms timeout) {
  if (timeout == portMAX_DELAY) {
    return portMAX_DELAY;
  }
  if (timeout == portMAX_DELAY) {
    return 0;
  }
  return timeout / portTICK_PERIOD_MS;
}