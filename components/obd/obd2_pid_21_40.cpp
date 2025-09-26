#include <array>
#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

/*  Find distance traveled with malfunction
indicator lamp (MIL) on in km Return:
 -------
  * std::optional<uint16_t> - Distance traveled with malfunction indicator lamp (MIL) on in
km
*/
std::optional<uint16_t> OBD2::distTravelWithMIL() {
  ResponseType response;
  if (processPID(SERVICE_01, DISTANCE_TRAVELED_WITH_MIL_ON, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return {};
}

/*  Find fuel Rail Pressure (relative to manifold
vacuum) in kPa Return:
 -------
  * std::optional<float> - Fuel Rail Pressure (relative to manifold vacuum) in kPa
*/
std::optional<float> OBD2::fuelRailPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_RAIL_PRESSURE, response)) {
    return {((response[A] << 8) | response[B]) * 0.079};
  }
  return {};
}

/*  Find fuel Rail Gauge Pressure (diesel,
or gasoline direct injection) in kPa Return:
 -------  * std::optional<float> - Fuel Rail Gauge Pressure (diesel, or gasoline direct injection)
in kPa
*/
std::optional<uint32_t> OBD2::fuelRailGuagePressure() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_RAIL_GUAGE_PRESSURE, response)) {
    return {((response[A] << 8) | response[B]) * 10};
  }
  return {};
}

/*
[this]() -> std::optional<float> {
        return response[A] / 200.0;
      };
constexpr std::optional<uint8_t> OXYGEN_SENSOR_1_B             = 36;  // 0x24 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_2_B             = 37;  // 0x25 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_3_B             = 38;  // 0x26 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_4_B             = 39;  // 0x27 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_5_B             = 40;  // 0x28 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_6_B             = 41;  // 0x29 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_7_B             = 42;  // 0x2A - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_8_B             = 43;  // 0x2B - ratio V
*/

/*  Find commanded Exhaust Gas Recirculation (EGR) in
% Return:
 -------
  * std::optional<float> - Commanded Exhaust Gas Recirculation (EGR) in %*/
std::optional<float> OBD2::commandedEGR() {
  ResponseType response;
  if (processPID(SERVICE_01, COMMANDED_EGR, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find Exhaust Gas Recirculation (EGR) error in %
Return:
 -------
  * std::optional<float> - Exhaust Gas Recirculation (EGR) error in %*/
std::optional<float> OBD2::egrError() {
  ResponseType response;
  if (processPID(SERVICE_01, EGR_ERROR, response)) {
    return {(response[A] * 100.0 / 255.0) - 100.0};
  }
  return {};
}

/*  Find commanded evaporative purge in %
Return:
 -------
  * std::optional<float> - Commanded evaporative purge in %*/
std::optional<float> OBD2::commandedEvapPurge() {
  ResponseType response;
  if (processPID(SERVICE_01, COMMANDED_EVAPORATIVE_PURGE, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find fuel tank level input in %
Return:
 -------
  * std::optional<float> - Fuel tank level input in %*/
std::optional<float> OBD2::fuelLevel() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_TANK_LEVEL_INPUT, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find num warm-ups since codes
cleared Return:
 -------
  * std::optional<uint8_t> - Num warm-ups since codes cleared
*/
std::optional<uint8_t> OBD2::warmUpsSinceCodesCleared() {
  ResponseType response;
  if (processPID(SERVICE_01, WARM_UPS_SINCE_CODES_CLEARED, response)) {
    return {response[A]};
  }
  return {};
}

/*  Find distance traveled since codes
cleared in km Return:
 -------
  * std::optional<uint16_t> - Distance traveled since codes cleared in km
*/
std::optional<uint16_t> OBD2::distSinceCodesCleared() {
  ResponseType response;
  if (processPID(SERVICE_01, DIST_TRAV_SINCE_CODES_CLEARED, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return {};
}

/*  Find evap. system vapor pressure in Pa
Return:
 -------
  * std::optional<float> - Evap. system vapor pressure in Pa
*/
std::optional<float> OBD2::evapSysVapPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, EVAP_SYSTEM_VAPOR_PRESSURE, response)) {
    return {((response[A] << 8) | response[B]) / 4.0};
  }
  return {};
}

/*  Find absolute barometric pressure in kPa
Return:
 -------
  * std::optional<uint8_t> - Absolute barometric pressure in kPa
*/
std::optional<uint8_t> OBD2::absBaroPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_BAROMETRIC_PRESSURE, response)) {
    return {response[A]};
  }
  return {};
}

/*
[this]() -> std::optional<float> {
        return response[A] / 200.0;
      };
constexpr std::optional<uint8_t> OXYGEN_SENSOR_1_C             = 52;  // 0x34 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_2_C             = 53;  // 0x35 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_3_C             = 54;  // 0x36 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_4_C             = 55;  // 0x37 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_5_C             = 56;  // 0x38 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_6_C             = 57;  // 0x39 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_7_C             = 58;  // 0x3A - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_8_C             = 59;  // 0x3B - ratio mA
*/

//  Find catalyst temperature in C
std::optional<float> OBD2::catTempB1S1() {
  ResponseType response;
  if (processPID(SERVICE_01, CATALYST_TEMP_BANK_1_SENSOR_1, response)) {
    return {(((response[A] << 8) | response[B]) / 10.0) - 40.0};
  }
  return {};
}

//  Find catalyst temperature in C
std::optional<float> OBD2::catTempB2S1() {
  ResponseType response;
  if (processPID(SERVICE_01, CATALYST_TEMP_BANK_2_SENSOR_1, response)) {
    return {(((response[A] << 8) | response[B]) / 10.0) - 40.0};
  }
  return {};
}

//  Find catalyst temperature in C
std::optional<float> OBD2::catTempB1S2() {
  ResponseType response;
  if (processPID(SERVICE_01, CATALYST_TEMP_BANK_1_SENSOR_2, response)) {
    return {(((response[A] << 8) | response[B]) / 10.0) - 40.0};
  }
  return {};
}

//  Find catalyst temperature in C
std::optional<float> OBD2::catTempB2S2() {
  ResponseType response;
  if (processPID(SERVICE_01, CATALYST_TEMP_BANK_2_SENSOR_2, response)) {
    return {(((response[A] << 8) | response[B]) / 10.0) - 40.0};
  }
  return {};
}