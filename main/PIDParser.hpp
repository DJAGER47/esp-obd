#pragma once

#include <cstdint>

#include "driver/twai.h"

class PIDParser {
 public:
  PIDParser() = default;

  // Парсинг PID 0x2F - Fuel Tank Level Input
  float parseFuelConsumption(const twai_message_t& message) {
    // Расчет расхода топлива в литрах
    // Формат данных зависит от конкретного автомобиля
    // Здесь пример для простого случая - процент заполнения бака
    return static_cast<float>(message.data[0]) *
           0.39215686f;  // 0-100% -> 0.0-1.0
  }

  // Парсинг PID 0x31 - Distance Traveled Since Codes Cleared
  float parseDistanceTraveled(const twai_message_t& message) {
    // Расчет пройденного расстояния в километрах
    // Формат: 2 байта, значение в км
    return static_cast<float>((message.data[0] << 8) | message.data[1]);
  }

  // Общий метод для парсинга любого PID
  float parsePID(uint32_t pid, const twai_message_t& message) {
    switch (pid) {
      case 0x2F:
        return parseFuelConsumption(message);
      case 0x31:
        return parseDistanceTraveled(message);
      default:
        return 0.0f;
    }
  }
};