#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

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
  return {};
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
  return {};
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
  return {};
}

/*
constexpr std::optional<uint8_t> ENGINE_PERCENT_TORQUE_DATA     = 100;  // 0x64 - %
*/

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
  return {};
}
