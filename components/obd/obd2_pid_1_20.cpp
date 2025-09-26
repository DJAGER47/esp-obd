#include <array>
#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

/* Monitor status since DTCs cleared (Includes malfunction indicator
  lamp (MIL) status and number of DTCs). See
 https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_01 for more info

  * std::optional<uint32_t> - Bit encoded status*/
std::optional<uint32_t> OBD2::monitorStatus() {
  ResponseType response;
  if (processPID(SERVICE_01, MONITOR_STATUS_SINCE_DTC_CLEARED, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return {};
}

/* Freeze DTC - see https://www.samarins.com/diagnose/freeze-frame.html for
 more info

  * std::optional<uint16_t> - Various vehicle information*/
std::optional<uint16_t> OBD2::freezeDTC() {
  ResponseType response;
  if (processPID(SERVICE_01, FREEZE_DTC, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return {};
}

/* Freeze DTC - see
https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_03 for more info

  * std::optional<uint16_t> - Bit encoded status*/
std::optional<uint16_t> OBD2::fuelSystemStatus() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_SYSTEM_STATUS, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return {};
}

/* Find the current engine load in %
 * std::optional<float> - Engine load %*/
std::optional<float> OBD2::engineLoad() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_LOAD, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/* Find the current engine coolant temp in C
 * std::optional<float> - Engine load %*/
std::optional<int16_t> OBD2::engineCoolantTemp() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_COOLANT_TEMP, response)) {
    return {static_cast<int16_t>(response[A]) - 40.0};
  }
  return {};
}

/* Find fuel trim %
 * std::optional<float> - Fuel trim %*/
std::optional<float> OBD2::shortTermFuelTrimBank_1() {
  ResponseType response;
  if (processPID(SERVICE_01, SHORT_TERM_FUEL_TRIM_BANK_1, response)) {
    return {(response[A] * 100.0 / 128.0) - 100.0};
  }
  return {};
}

/* Find fuel trim %
 * std::optional<float> - Fuel trim %*/
std::optional<float> OBD2::longTermFuelTrimBank_1() {
  ResponseType response;
  if (processPID(SERVICE_01, LONG_TERM_FUEL_TRIM_BANK_1, response)) {
    return {(response[A] * 100.0 / 128.0) - 100.0};
  }
  return {};
}

/* Find fuel trim %
 * std::optional<float> - Fuel trim %*/
std::optional<float> OBD2::shortTermFuelTrimBank_2() {
  ResponseType response;
  if (processPID(SERVICE_01, SHORT_TERM_FUEL_TRIM_BANK_2, response)) {
    return {(response[A] * 100.0 / 128.0) - 100.0};
  }
  return {};
}

/* Find fuel trim %
 * std::optional<float> - Fuel trim %*/
std::optional<float> OBD2::longTermFuelTrimBank_2() {
  ResponseType response;
  if (processPID(SERVICE_01, LONG_TERM_FUEL_TRIM_BANK_2, response)) {
    return {(response[A] * 100.0 / 128.0) - 100.0};
  }
  return {};
}

/* Find fuel pressure in kPa
 * std::optional<float> - Fuel pressure in kPa
 */
std::optional<uint16_t> OBD2::fuelPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_PRESSURE, response)) {
    return {static_cast<uint16_t>(response[A]) * 3.0};
  }
  return {};
}

/* Find intake manifold absolute pressure in kPa
 * std::optional<uint8_t> - Intake manifold absolute pressure in kPa
 */
std::optional<uint8_t> OBD2::manifoldPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, INTAKE_MANIFOLD_ABS_PRESSURE, response)) {
    return {response[A]};
  }
  return {};
}

/* Queries and parses received message for/returns vehicle RMP data
 * std::optional<float> - Vehicle RPM
 */
std::optional<float> OBD2::rpm() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_RPM, response)) {
    return {((response[A] << 8) | response[B]) / 4.0};
  }
  return {};
}

/*  Queries and parses received message for/returns vehicle speed data (kph)
 * int32_t - Vehicle speed in kph
 */
std::optional<uint8_t> OBD2::kph() {
  ResponseType response;
  if (processPID(SERVICE_01, VEHICLE_SPEED, response)) {
    return response[A];
  }
  return {};
}

/*
 std::optional<float> OBD2::timingAdvance() *  Find timing advance in degrees before Top Dead
Center (TDC)
  * std::optional<float> - Timing advance in degrees before Top Dead Center (TDC)*/
std::optional<float> OBD2::timingAdvance() {
  ResponseType response;
  if (processPID(SERVICE_01, TIMING_ADVANCE, response)) {
    return {response[A] / 2.0 - 64.0};
  }
  return {};
}

/*  Find intake air temperature in C
 * std::optional<float> - Intake air temperature in C
 */
std::optional<int16_t> OBD2::intakeAirTemp() {
  ResponseType response;
  if (processPID(SERVICE_01, INTAKE_AIR_TEMP, response)) {
    return {static_cast<int16_t>(response[A]) - 40.0};
  }
  return {};
}

/*  Find mass air flow sensor (MAF) air flow rate rate in g/s
 * std::optional<float> - Mass air flow sensor (MAF) air flow rate rate in g/s
 */
std::optional<float> OBD2::mafRate() {
  ResponseType response;
  if (processPID(SERVICE_01, MAF_FLOW_RATE, response)) {
    return {((response[A] << 8) | response[B]) / 100.0};
  }
  return {};
}

/*  Find throttle position in %
Return:
 -------
  * std::optional<float> - Throttle position in %*/
std::optional<float> OBD2::throttle() {
  ResponseType response;
  if (processPID(SERVICE_01, THROTTLE_POSITION, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return {};
}

/*  Find commanded secondary air status

  * std::optional<uint8_t> - Bit encoded status
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_12)*/
std::optional<uint8_t> OBD2::commandedSecAirStatus() {
  ResponseType response;
  if (processPID(SERVICE_01, COMMANDED_SECONDARY_AIR_STATUS, response)) {
    return response[A];
  }
  return {};
}

/*  Find which oxygen sensors are
present ([A0..A3] == Bank 1, Sensors 1-4. [A4..A7] == Bank 2...) Return:
 -------
  * std::optional<uint8_t> - Bit encoded
*/
std::optional<uint8_t> OBD2::oxygenSensorsPresent_2banks() {
  ResponseType response;
  if (processPID(SERVICE_01, OXYGEN_SENSORS_PRESENT_2_BANKS, response)) {
    return response[A];
  }
  return {};
}

/*
[this]() -> std::optional<float> {
        return response[A] / 200.0;
      };
constexpr std::optional<uint8_t> OXYGEN_SENSOR_1_A              = 20;  // 0x14 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_2_A              = 21;  // 0x15 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_3_A              = 22;  // 0x16 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_4_A              = 23;  // 0x17 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_5_A              = 24;  // 0x18 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_6_A              = 25;  // 0x19 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_7_A              = 26;  // 0x1A - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_8_A              = 27;  // 0x1B - V %
*/

/*  Find the OBD standards this vehicle conforms to
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_1C)
  * std::optional<uint8_t> - Bit encoded*/
std::optional<uint8_t> OBD2::obdStandards() {
  ResponseType response;
  if (processPID(SERVICE_01, OBD_STANDARDS, response)) {
    return response[A];
  }
  return {};
}

/*  Find which oxygen sensors are
present (Similar to PID 13, but [A0..A7] == [B1S1, B1S2, B2S1, B2S2, B3S1,
B3S2, B4S1, B4S2]) Return:
 -------
  * std::optional<uint8_t> - Bit encoded
*/
std::optional<uint8_t> OBD2::oxygenSensorsPresent_4banks() {
  ResponseType response;
  if (processPID(SERVICE_01, OXYGEN_SENSORS_PRESENT_4_BANKS, response)) {
    return response[A];
  }
  return {};
}

/*  Find Power Take Off (PTO) status
 * bool - Power Take Off (PTO) status
 */
std::optional<bool> OBD2::auxInputStatus() {
  ResponseType response;
  if (processPID(SERVICE_01, AUX_INPUT_STATUS, response)) {
    return static_cast<bool>(response[A]);
  }
  return {};
}

/*  Find run time since engine start in s
 * std::optional<uint16_t> - Run time since engine start in s
 */
std::optional<uint16_t> OBD2::runTime() {
  ResponseType response;
  if (processPID(SERVICE_01, RUN_TIME_SINCE_ENGINE_START, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return {};
}
