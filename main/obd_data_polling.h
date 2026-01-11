#pragma once

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Предварительное объявление классов
class OBD2;
class UI;

/**
 * @brief Функция задачи FreeRTOS для опроса данных OBD2
 * @param arg Параметр задачи (не используется)
 */
void obd_polling_task(void* arg);