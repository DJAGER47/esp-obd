#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

/*  Find driver's demanded engine torque in %
Return:
 -------
  * std::optional<float> - Driver's demanded engine torque in %*/
std::optional<int16_t> OBD2::demandedTorque() {
  ResponseType response;
  if (processPID(SERVICE_01, DEMANDED_ENGINE_PERCENT_TORQUE, response)) {
    return {static_cast<int16_t>(response[A]) - 125.0};
  }
  return {};
}

/*  Find actual engine torque in %
Return:
 -------
  * std::optional<float> - Actual engine torque in %*/
std::optional<int16_t> OBD2::torque() {
  ResponseType response;
  if (processPID(SERVICE_01, ACTUAL_ENGINE_TORQUE, response)) {
    return {static_cast<int16_t>(response[A]) - 125.0};
  }
  return {};
}

/*  Find engine reference torque in Nm
Return:
 -------
  * std::optional<uint16_t> - Engine reference torque in Nm
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

/*  Find auxiliary input/output supported
Return:
 -------
  * std::optional<uint16_t> - Bit encoded (?)*/
std::optional<uint16_t> OBD2::auxSupported() {
  ResponseType response;
  if (processPID(SERVICE_01, AUX_INPUT_OUTPUT_SUPPORTED, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return {};
}