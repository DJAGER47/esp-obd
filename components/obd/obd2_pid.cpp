#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

/**
 * @brief Вспомогательный метод для получения поддерживаемых PID
 *
 * @param pid PID для запроса поддерживаемых PID
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PIDs
 */
std::optional<uint32_t> OBD2::GetSupportedPids(uint8_t pid) {
  ResponseType response;
  if (ProcessPidWithoutCheck(SERVICE_01, pid, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return std::nullopt;
}
