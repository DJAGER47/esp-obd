#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

/**
 * @brief Получает список поддерживаемых PID в диапазоне 61-80
 *
 * Возвращает битовую маску, где каждый бит указывает на поддержку соответствующего PID.
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_60
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID
 */
std::optional<uint32_t> OBD2::supportedPIDs_61_80() {
  return getSupportedPIDs(SUPPORTED_PIDS_61_80);
}

/**
 * @brief Получает требуемый водителем крутящий момент двигателя
 *
 * @return std::optional<int16_t> Крутящий момент в процентах [-125..125%]
 */
std::optional<int16_t> OBD2::demandedTorque() {
  ResponseType response;
  if (processPID(SERVICE_01, DEMANDED_ENGINE_PERCENT_TORQUE, response)) {
    return {static_cast<int16_t>(response[A]) - 125.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает фактический крутящий момент двигателя
 *
 * @return std::optional<int16_t> Крутящий момент в процентах [-125..125%]
 */
std::optional<int16_t> OBD2::torque() {
  ResponseType response;
  if (processPID(SERVICE_01, ACTUAL_ENGINE_TORQUE, response)) {
    return {static_cast<int16_t>(response[A]) - 125.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает опорный крутящий момент двигателя
 *
 * @return std::optional<uint16_t> Крутящий момент в Н·м
 */
std::optional<uint16_t> OBD2::referenceTorque() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_REFERENCE_TORQUE, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return std::nullopt;
}

/**
 * @brief Получает данные о проценте крутящего момента двигателя в различных точках
 *
 * Возвращает массив из 5 значений процента крутящего момента:
 * - Индекс 0: Холостой ход (Idle)
 * - Индекс 1: Точка работы двигателя 1 (Engine point 1)
 * - Индекс 2: Точка работы двигателя 2 (Engine point 2)
 * - Индекс 3: Точка работы двигателя 3 (Engine point 3)
 * - Индекс 4: Точка работы двигателя 4 (Engine point 4)
 *
 * @return std::optional<std::array<int16_t, 5>> Массив процентов крутящего момента [-125..125%]
 */
std::optional<std::array<int16_t, 5>> OBD2::enginePercentTorqueData() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_PERCENT_TORQUE_DATA, response)) {
    std::array<int16_t, 5> result;
    for (size_t i = 0; i < 5; ++i) {
      result[i] = static_cast<int16_t>(response[i]) - 125;
    }
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Получает статус вспомогательных входов/выходов
 *
 * @return std::optional<uint16_t> Битовая маска поддерживаемых функций
 */
std::optional<uint16_t> OBD2::auxSupported() {
  ResponseType response;
  if (processPID(SERVICE_01, AUX_INPUT_OUTPUT_SUPPORTED, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return std::nullopt;
}

/**
 * @brief Получает список поддерживаемых PID в диапазоне 81-100
 *
 * Возвращает битовую маску, где каждый бит указывает на поддержку соответствующего PID.
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_80
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID
 */
std::optional<uint32_t> OBD2::supportedPIDs81_100() {
  return getSupportedPIDs(SUPPORTED_PIDS_81_100);
}

/**
 * @brief Получает список поддерживаемых PID в диапазоне 101-120
 *
 * Возвращает битовую маску, где каждый бит указывает на поддержку соответствующего PID.
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_A0
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID
 */
std::optional<uint32_t> OBD2::supportedPIDs101_120() {
  return getSupportedPIDs(SUPPORTED_PIDS_101_120);
}

/**
 * @brief Получает список поддерживаемых PID в диапазоне 121-140
 *
 * Возвращает битовую маску, где каждый бит указывает на поддержку соответствующего PID.
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_C0
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID
 */
std::optional<uint32_t> OBD2::supportedPIDs121_140() {
  return getSupportedPIDs(SUPPORTED_PIDS_121_140);
}
