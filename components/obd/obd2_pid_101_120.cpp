#include <array>
#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

/**
 * @brief Получает список поддерживаемых PID в диапазоне 101-120
 *
 * Возвращает битовую маску, где каждый бит указывает на поддержку соответствующего PID.
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID
 */
std::optional<uint32_t> OBD2::supportedPIDs101_120() {
  return getSupportedPIDs(SUPPORTED_PIDS_101_120);
}