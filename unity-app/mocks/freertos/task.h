#pragma once

#include <chrono>
#include <cstring>
#include <thread>

// Заменяем TickType_t на uint32_t
typedef uint32_t TickType_t;

// Заменяем portTICK_PERIOD_MS на 1 (1 тик = 1 мс)
#define portTICK_PERIOD_MS 1

// Заменяем pdMS_TO_TICKS на простое преобразование миллисекунд в тики (в данном случае тики =
// миллисекунды)
#define pdMS_TO_TICKS(ms)  (ms)

// Заменяем xTaskGetTickCount на реализацию для Linux
inline uint32_t xTaskGetTickCount() {
  auto now      = std::chrono::high_resolution_clock::now();
  auto duration = now.time_since_epoch();
  auto millis   = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  return static_cast<uint32_t>(millis);
}

// Заменяем vTaskDelay на std::this_thread::sleep_for
inline void vTaskDelay(uint32_t ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Заменяем esp_rom_delay_us на std::this_thread::sleep_for
inline void esp_rom_delay_us(uint32_t us) {
  std::this_thread::sleep_for(std::chrono::microseconds(us));
}