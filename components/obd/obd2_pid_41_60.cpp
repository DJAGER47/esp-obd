#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

/*  Find status this drive cycle
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_41)
Return:
 -------
  * std::optional<uint32_t> - Bit encoded status
*/
std::optional<uint32_t> OBD2::monitorDriveCycleStatus() {
  ResponseType response;
  if (processPID(SERVICE_01, MONITOR_STATUS_THIS_DRIVE_CYCLE, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

/*  Find control module voltage in V
Return:
 -------
  * std::optional<float> - Control module voltage in V
*/
std::optional<float> OBD2::ctrlModVoltage() {
  ResponseType response;
  if (processPID(SERVICE_01, CONTROL_MODULE_VOLTAGE, response)) {
    return {((response[A] << 8) | response[B]) / 1000.0};
  }
  return {};
}

/*  Find absolute load value in %
Return:
 -------
  * std::optional<float> - Absolute load value in %*/
std::optional<float> OBD2::absLoad() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_LOAD_VALUE, response)) {
    return {((response[A] << 8) | response[B]) * 100.0 / 255.0};
  }
  return {};
}

/*  Find commanded air-fuel equivalence
ratio Return:
 -------
  * std::optional<float> - Commanded air-fuel equivalence ratio
*/
std::optional<float> OBD2::commandedAirFuelRatio() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_AIR_COMMANDED_EQUIV_RATIO, response)) {
    return {((response[A] << 8) | response[B]) / 32768.0};
  }
  return {};
}

/*  Find relative throttle position in %
Return:
 -------
  * std::optional<float> - Relative throttle position in %*/
std::optional<float> OBD2::relativeThrottle() {
  ResponseType response;
  if (processPID(SERVICE_01, RELATIVE_THROTTLE_POSITION, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find ambient air temperature in C
Return:
 -------
  * std::optional<float> - Ambient air temperature in C
*/
std::optional<int16_t> OBD2::ambientAirTemp() {
  ResponseType response;
  if (processPID(SERVICE_01, AMBIENT_AIR_TEMP, response)) {
    return {static_cast<int16_t>(response[A]) - 40};
  }
  return {};
}

/*  Find absolute throttle position B in %
Return:
 -------
  * std::optional<float> - Absolute throttle position B in %*/
std::optional<float> OBD2::absThrottlePosB() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_THROTTLE_POSITION_B, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find absolute throttle position C in %
Return:
 -------
  * std::optional<float> - Absolute throttle position C in %*/
std::optional<float> OBD2::absThrottlePosC() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_THROTTLE_POSITION_C, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find absolute throttle position D in %
Return:
 -------
  * std::optional<float> - Absolute throttle position D in %*/
std::optional<float> OBD2::absThrottlePosD() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_THROTTLE_POSITION_D, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find absolute throttle position E in %
Return:
 -------
  * std::optional<float> - Absolute throttle position E in %*/
std::optional<float> OBD2::absThrottlePosE() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_THROTTLE_POSITION_E, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find absolute throttle position F in %
Return:
 -------
  * std::optional<float> - Absolute throttle position F in %*/
std::optional<float> OBD2::absThrottlePosF() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_THROTTLE_POSITION_F, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find commanded throttle actuator in
% Return:
 -------
  * std::optional<float> - Commanded throttle actuator in %*/
std::optional<float> OBD2::commandedThrottleActuator() {
  ResponseType response;
  if (processPID(SERVICE_01, COMMANDED_THROTTLE_ACTUATOR, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find time run with MIL on in min
Return:
 -------
  * std::optional<uint16_t> - Time run with MIL on in min
*/
std::optional<uint16_t> OBD2::timeRunWithMIL() {
  ResponseType response;
  if (processPID(SERVICE_01, TIME_RUN_WITH_MIL_ON, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return {};
}

/*  Find time since trouble codes cleared
in min Return:
 -------
  * std::optional<uint16_t> - Time since trouble codes cleared in min
*/
std::optional<uint16_t> OBD2::timeSinceCodesCleared() {
  ResponseType response;
  if (processPID(SERVICE_01, TIME_SINCE_CODES_CLEARED, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return {};
}

/*
constexpr std::optional<uint8_t> MAX_VALUES_EQUIV_V_I_PRESSURE = 79;  // 0x4F - ratio V mA kPa
[this]() -> std::optional<float> {
        return response[A];
      };
*/

/*  Find maximum value for air flow rate from mass air
flow sensor in g/s Return:
 -------
  * std::optional<float> - Maximum value for air flow rate from mass air flow sensor in g/s
*/
std::optional<uint16_t> OBD2::maxMafRate() {
  ResponseType response;
  if (processPID(SERVICE_01, MAX_MAF_RATE, response)) {
    return {response[A] * 10};
  }
  return {};
}

/*  Find fuel type
(https://en.wikipedia.org/wiki/OBD-II_PIDs#Fuel_Type_Coding) Return:
 -------
  * std::optional<uint8_t> - Bit encoded*/
std::optional<uint8_t> OBD2::fuelType() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_TYPE, response)) {
    return {response[A]};
  }
  return {};
}

/*  Find ethanol fuel in %
Return:
 -------
  * std::optional<float> - Ethanol fuel in %*/
std::optional<float> OBD2::ethanolPercent() {
  ResponseType response;
  if (processPID(SERVICE_01, ETHANOL_FUEL_PERCENT, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find absolute evap. system vapor
pressure in kPa Return:
 -------
  * std::optional<float> - Absolute evap. system vapor pressure in kPa
*/
std::optional<float> OBD2::absEvapSysVapPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_EVAP_SYS_VAPOR_PRESSURE, response)) {
    return {((response[A] << 8) | response[B]) / 200.0};
  }
  return {};
}

/*  Find evap. system vapor pressure in Pa
Return:
 -------
  * std::optional<float> - Evap. system vapor pressure in Pa
*/
std::optional<int32_t> OBD2::evapSysVapPressure2() {
  ResponseType response;
  if (processPID(SERVICE_01, EVAP_SYS_VAPOR_PRESSURE, response)) {
    return {static_cast<int32_t>((response[A] << 8) | response[B]) - 32767};
  }
  return {};
}

/*
[this]() -> std::optional<float> {
        return (response[A] * (100.0 / 128.0)) - 100.0;
      };

constexpr std::optional<uint8_t> SHORT_TERM_SEC_OXY_SENS_TRIM_1_3 = 85;  // 0x55 - %
constexpr std::optional<uint8_t> LONG_TERM_SEC_OXY_SENS_TRIM_1_3  = 86;  // 0x56 - %
constexpr std::optional<uint8_t> SHORT_TERM_SEC_OXY_SENS_TRIM_2_4 = 87;  // 0x57 - %
constexpr std::optional<uint8_t> LONG_TERM_SEC_OXY_SENS_TRIM_2_4  = 88;  // 0x58 - %
*/

/*  Find absolute fuel rail pressure in kPa
Return:
 -------
  * std::optional<float> - absolute fuel rail pressure in kPa
*/
std::optional<uint32_t> OBD2::absFuelRailPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_RAIL_ABS_PRESSURE, response)) {
    return {((response[A] << 8) | response[B]) * 10};
  }
  return {};
}

/*  Find relative accelerator pedal position in %
Return:
 -------
  * std::optional<float> - Relative accelerator pedal position in %*/
std::optional<float> OBD2::relativePedalPos() {
  ResponseType response;
  if (processPID(SERVICE_01, RELATIVE_ACCELERATOR_PEDAL_POS, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find hybrid battery pack remaining life in %
Return:
 -------
  * std::optional<float> - Hybrid battery pack remaining life in %*/
std::optional<float> OBD2::hybridBatLife() {
  ResponseType response;
  if (processPID(SERVICE_01, HYBRID_BATTERY_REMAINING_LIFE, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find engine oil temperature in C
Return:
 -------
  * Engine oil temperature in C
*/
std::optional<int16_t> OBD2::oilTemp() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_OIL_TEMP, response)) {
    return {static_cast<int16_t>(response[A]) - 40.0};
  }
  return {};
}

/*  Find fuel injection timing in degrees
Return:
 -------
  * std::optional<float> - Fuel injection timing in degrees
*/
std::optional<float> OBD2::fuelInjectTiming() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_INJECTION_TIMING, response)) {
    return {(((response[A] << 8) | response[B]) / 128.0) - 210};
  }
  return {};
}

/*  Find engine fuel rate in L/h
Return:
 -------
  * std::optional<float> - Engine fuel rate in L/h
*/
std::optional<float> OBD2::fuelRate() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_FUEL_RATE, response)) {
    return {((response[A] << 8) | response[B]) / 20.0};
  }
  return {};
}

/*  Find emission requirements to which vehicle is
designed Return:
 -------
  * std::optional<uint8_t> - Bit encoded (?)*/
std::optional<uint8_t> OBD2::emissionRqmts() {
  ResponseType response;
  if (processPID(SERVICE_01, EMISSION_REQUIREMENTS, response)) {
    return {response[A]};
  }
  return {};
}