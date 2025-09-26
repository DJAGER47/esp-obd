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
std::optional<uint32_t> OBD2::getSupportedPIDs(uint8_t pid) {
  ResponseType response;
  if (processPID(SERVICE_01, pid, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

/**
 * @brief Проверяет, поддерживается ли указанный PID ECU
 *
 * Вспомогательный метод, который выбирает соответствующий запрос supportedPIDS_xx_xx()
 * и анализирует битовую маску результата.
 *
 * @param pid PID для проверки поддержки
 * @return bool True если PID поддерживается, иначе False
 */
bool OBD2::isPidSupported(uint8_t pid) {
  std::optional<uint32_t> supportedPids;
  uint8_t adjustedPid = pid;

  // Определяем диапазон PID и получаем соответствующую битовую маску
  if (pid >= 0x01 && pid <= 0x20) {
    supportedPids = getSupportedPIDs(SUPPORTED_PIDS_1_20);
  } else if (pid >= 0x21 && pid <= 0x40) {
    supportedPids = getSupportedPIDs(SUPPORTED_PIDS_21_40);
    adjustedPid   = pid - 0x20;  // Нормализуем к диапазону 1-32
  } else if (pid >= 0x41 && pid <= 0x60) {
    supportedPids = getSupportedPIDs(SUPPORTED_PIDS_41_60);
    adjustedPid   = pid - 0x40;  // Нормализуем к диапазону 1-32
  } else if (pid >= 0x61 && pid <= 0x80) {
    supportedPids = getSupportedPIDs(SUPPORTED_PIDS_61_80);
    adjustedPid   = pid - 0x60;  // Нормализуем к диапазону 1-32
  } else if (pid >= 0x81 && pid <= 0xA0) {
    supportedPids = getSupportedPIDs(SUPPORTED_PIDS_81_100);
    adjustedPid   = pid - 0x80;  // Нормализуем к диапазону 1-32
  } else if (pid >= 0xA1 && pid <= 0xC0) {
    supportedPids = getSupportedPIDs(SUPPORTED_PIDS_101_120);
    adjustedPid   = pid - 0xA0;  // Нормализуем к диапазону 1-32
  } else if (pid >= 0xC1 && pid <= 0xE0) {
    supportedPids = getSupportedPIDs(SUPPORTED_PIDS_121_140);
    adjustedPid   = pid - 0xC0;  // Нормализуем к диапазону 1-32
  } else {
    return false;  // PID вне поддерживаемых диапазонов
  }

  if (supportedPids.has_value()) {
    // Расчет позиции бита: PID 1 это бит 31, PID 2 это бит 30, и т.д.
    uint8_t bitPosition = 32 - adjustedPid;
    return ((supportedPids.value() >> bitPosition) & 0x1) != 0;
  }
  return false;
}
