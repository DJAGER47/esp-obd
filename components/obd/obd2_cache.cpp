#include <cctype>
#include <cinttypes>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iso_tp.h"
#include "obd2.h"

static const char* const TAG = "OBD2";

/**
 * @brief Проверяет, поддерживается ли PID
 *
 * @param pid Parameter ID (PID)
 * @return bool True, если PID поддерживается, иначе False
 */
bool OBD2::IsPidSupported(uint8_t pid) {
  const uint32_t kCurrentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
  const bool kIsCacheExpired  = (kCurrentTime - pid_support_cache_.last_update_time) > kPidCacheTimeotMs;

  if (!pid_support_cache_.initialized || kIsCacheExpired) {
    UpdatePidSupportCache();
  }

  // PID 0 используется для запроса поддерживаемых PID 1-20, поэтому вычитаем 1
  const uint8_t adjusted_pid  = (pid == 0) ? 0 : pid - 1;
  const uint8_t array_index   = adjusted_pid / 32;
  const uint32_t bit_position = 1 << (31 - (adjusted_pid % 32));

  if (array_index >= 7) {
    return false;
  }

  return (pid_support_cache_.supported_pids[array_index] & bit_position) != 0;
}

/**
 * @brief Обновляет кэш поддерживаемых PID, запрашивая информацию у автомобиля
 */
void OBD2::UpdatePidSupportCache() {
  for (int i = 0; i < 7; i++) {
    pid_support_cache_.supported_pids[i] = 0;
  }

  // Запрашиваем поддерживаемые PID по группам
  // Группа 1-20 (PID 0x00)
  auto pids_1_20 = GetSupportedPids(SUPPORTED_PIDS_1_20);
  if (pids_1_20.has_value()) {
    pid_support_cache_.supported_pids[0] = pids_1_20.value();
  }

  if (pids_1_20.has_value() && (pids_1_20.value() & 1)) {
    // Группа 21-40 (PID 0x20)
    auto pids_21_40 = GetSupportedPids(SUPPORTED_PIDS_21_40);
    if (pids_21_40.has_value()) {
      pid_support_cache_.supported_pids[1] = pids_21_40.value();
    }

    if (pids_21_40.has_value() && (pids_21_40.value() & 1)) {
      // Группа 41-60 (PID 0x40)
      auto pids_41_60 = GetSupportedPids(SUPPORTED_PIDS_41_60);
      if (pids_41_60.has_value()) {
        pid_support_cache_.supported_pids[2] = pids_41_60.value();
      }

      if (pids_41_60.has_value() && (pids_41_60.value() & 1)) {
        // Группа 61-80 (PID 0x60)
        auto pids_61_80 = GetSupportedPids(SUPPORTED_PIDS_61_80);
        if (pids_61_80.has_value()) {
          pid_support_cache_.supported_pids[3] = pids_61_80.value();
        }

        if (pids_61_80.has_value() && (pids_61_80.value() & 1)) {
          // Группа 81-100 (PID 0x80)
          auto pids_81_100 = GetSupportedPids(SUPPORTED_PIDS_81_100);
          if (pids_81_100.has_value()) {
            pid_support_cache_.supported_pids[4] = pids_81_100.value();
          }

          if (pids_81_100.has_value() && (pids_81_100.value() & 1)) {
            // Группа 101-120 (PID 0xA0)
            auto pids_101_120 = GetSupportedPids(SUPPORTED_PIDS_101_120);
            if (pids_101_120.has_value()) {
              pid_support_cache_.supported_pids[5] = pids_101_120.value();
            }

            if (pids_101_120.has_value() && (pids_101_120.value() & 1)) {
              // Группа 121-140 (PID 0xC0)
              auto pids_121_140 = GetSupportedPids(SUPPORTED_PIDS_121_140);
              if (pids_121_140.has_value()) {
                pid_support_cache_.supported_pids[6] = pids_121_140.value();
              }
            }
          }
        }
      }
    }
  }

  // Обновляем время последнего обновления и помечаем кэш как инициализированный
  pid_support_cache_.last_update_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  pid_support_cache_.initialized      = true;
}
