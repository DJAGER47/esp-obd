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
 * @brief Получает список поддерживаемых PID в диапазоне 121-140
 *
 * Возвращает битовую маску, где каждый бит указывает на поддержку соответствующего PID.
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID
 */
std::optional<uint32_t> OBD2::supportedPIDs121_140() {
  return getSupportedPIDs(SUPPORTED_PIDS_121_140);
}