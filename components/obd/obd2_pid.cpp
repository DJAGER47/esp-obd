#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

/**
 * @brief Определяет, какие PIDs в диапазоне 0x1-0x20 поддерживаются
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PIDs
 */
std::optional<uint32_t> OBD2::supportedPIDs_1_20() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_1_20, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  };
  return {};
}

/**
 * @brief Определяет, какие PIDs в диапазоне 0x21-0x40 поддерживаются
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PIDs
 */
std::optional<uint32_t> OBD2::supportedPIDs_21_40() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_21_40, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

/**
 * @brief Определяет, какие PIDs в диапазоне 0x41-0x60 поддерживаются
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PIDs
 */
std::optional<uint32_t> OBD2::supportedPIDs_41_60() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_41_60, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

/**
 * @brief Определяет, какие PIDs в диапазоне 0x61-0x80 поддерживаются
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PIDs
 */
std::optional<uint32_t> OBD2::supportedPIDs_61_80() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_61_80, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

/**
 * @brief Получает поддерживаемые PID группы 81-100
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID 81-100
 */
std::optional<uint32_t> OBD2::supportedPIDs81_100() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_81_100, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

/**
 * @brief Получает поддерживаемые PID группы 101-120
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID 101-120
 */
std::optional<uint32_t> OBD2::supportedPIDs101_120() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_101_120, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

/**
 * @brief Получает поддерживаемые PID группы 121-140
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID 121-140
 */
std::optional<uint32_t> OBD2::supportedPIDs121_140() {
  ResponseType response;
  if (processPID(SERVICE_01, SUPPORTED_PIDS_121_140, response)) {
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
  const uint8_t pidInterval = (pid / PID_INTERVAL_OFFSET) * PID_INTERVAL_OFFSET;
  std::optional<uint32_t> supportedPids;

  switch (pidInterval) {
    case SUPPORTED_PIDS_1_20:
      supportedPids = supportedPIDs_1_20();
      break;

    case SUPPORTED_PIDS_21_40:
      supportedPids = supportedPIDs_21_40();
      pid           = (pid - SUPPORTED_PIDS_21_40);
      break;

    case SUPPORTED_PIDS_41_60:
      supportedPids = supportedPIDs_41_60();
      pid           = (pid - SUPPORTED_PIDS_41_60);
      break;

    case SUPPORTED_PIDS_61_80:
      supportedPids = supportedPIDs_61_80();
      pid           = (pid - SUPPORTED_PIDS_61_80);
      break;

    default:
      return false;
  }

  if (supportedPids.has_value()) {
    // Bit position calculation: PID 1 is bit 31, PID 2 is bit 30, etc.
    uint8_t bitPosition = 32 - pid;
    return ((supportedPids.value() >> bitPosition) & 0x1) != 0;
  }
  return false;
}
