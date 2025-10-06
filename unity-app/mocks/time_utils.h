#pragma once

#include "task.h"

using Time_ms = uint32_t;

inline TickType_t convertToFreeRtosTick(Time_ms timeout) {
  return timeout;
}