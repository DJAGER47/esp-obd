#pragma once

#include <stdint.h>

#include "freertos/FreeRTOS.h"

using Time_ms = uint32_t;

TickType_t convertToFreeRtosTick(Time_ms timeout);