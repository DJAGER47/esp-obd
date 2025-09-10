#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "obd2.h"

/*  Find status this drive cycle
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_41)
Return:
 -------
  * uint32_t - Bit encoded status
*/
uint32_t OBD2::monitorDriveCycleStatus() {
  return (uint32_t)processPID(
      SERVICE_01, MONITOR_STATUS_THIS_DRIVE_CYCLE, 1, 4);
}

/*  Find control module voltage in V
Return:
 -------
  * double - Control module voltage in V
*/
double OBD2::ctrlModVoltage() {
  return processPID(SERVICE_01, CONTROL_MODULE_VOLTAGE, 1, 2, 1.0 / 1000.0);
}

/*  Find absolute load value in %
Return:
 -------
  * double - Absolute load value in %*/
double OBD2::absLoad() {
  return processPID(SERVICE_01, ABS_LOAD_VALUE, 1, 2, 100.0 / 255.0);
}

/*  Find commanded air-fuel equivalence
ratio Return:
 -------
  * double - Commanded air-fuel equivalence ratio
*/
double OBD2::commandedAirFuelRatio() {
  return processPID(
      SERVICE_01, FUEL_AIR_COMMANDED_EQUIV_RATIO, 1, 2, 2.0 / 65536.0);
}

/*  Find relative throttle position in %
Return:
 -------
  * double - Relative throttle position in %*/
double OBD2::relativeThrottle() {
  return processPID(
      SERVICE_01, RELATIVE_THROTTLE_POSITION, 1, 1, 100.0 / 255.0);
}

/*  Find ambient air temperature in C
Return:
 -------
  * double - Ambient air temperature in C
*/
double OBD2::ambientAirTemp() {
  return processPID(SERVICE_01, AMBIENT_AIR_TEMP, 1, 1, 1, -40);
}

/*  Find absolute throttle position B in %
Return:
 -------
  * double - Absolute throttle position B in %*/
double OBD2::absThrottlePosB() {
  return processPID(SERVICE_01, ABS_THROTTLE_POSITION_B, 1, 1, 100.0 / 255.0);
}

/*  Find absolute throttle position C in %
Return:
 -------
  * double - Absolute throttle position C in %*/
double OBD2::absThrottlePosC() {
  return processPID(SERVICE_01, ABS_THROTTLE_POSITION_C, 1, 1, 100.0 / 255.0);
}

/*  Find absolute throttle position D in %
Return:
 -------
  * double - Absolute throttle position D in %*/
double OBD2::absThrottlePosD() {
  return processPID(SERVICE_01, ABS_THROTTLE_POSITION_D, 1, 1, 100.0 / 255.0);
}

/*  Find absolute throttle position E in %
Return:
 -------
  * double - Absolute throttle position E in %*/
double OBD2::absThrottlePosE() {
  return processPID(SERVICE_01, ABS_THROTTLE_POSITION_E, 1, 1, 100.0 / 255.0);
}

/*  Find absolute throttle position F in %
Return:
 -------
  * double - Absolute throttle position F in %*/
double OBD2::absThrottlePosF() {
  return processPID(SERVICE_01, ABS_THROTTLE_POSITION_F, 1, 1, 100.0 / 255.0);
}

/*  Find commanded throttle actuator in
% Return:
 -------
  * double - Commanded throttle actuator in %*/
double OBD2::commandedThrottleActuator() {
  return processPID(
      SERVICE_01, COMMANDED_THROTTLE_ACTUATOR, 1, 1, 100.0 / 255.0);
}

/*  Find time run with MIL on in min
Return:
 -------
  * uint16_t - Time run with MIL on in min
*/
uint16_t OBD2::timeRunWithMIL() {
  return (uint16_t)processPID(SERVICE_01, TIME_RUN_WITH_MIL_ON, 1, 2);
}

/*  Find time since trouble codes cleared
in min Return:
 -------
  * uint16_t - Time since trouble codes cleared in min
*/
uint16_t OBD2::timeSinceCodesCleared() {
  return (uint16_t)processPID(SERVICE_01, TIME_SINCE_CODES_CLEARED, 1, 2);
}

/*
constexpr uint8_t MAX_VALUES_EQUIV_V_I_PRESSURE = 79;  // 0x4F - ratio V mA kPa
*/

/*  Find maximum value for air flow rate from mass air
flow sensor in g/s Return:
 -------
  * double - Maximum value for air flow rate from mass air flow sensor in g/s
*/
double OBD2::maxMafRate() {
  return processPID(SERVICE_01, MAX_MAF_RATE, 1, 1, 10.0);
}

/*  Find fuel type
(https://en.wikipedia.org/wiki/OBD-II_PIDs#Fuel_Type_Coding) Return:
 -------
  * uint8_t - Bit encoded
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Fuel_Type_Coding)*/
uint8_t OBD2::fuelType() {
  return static_cast<uint8_t>(processPID(SERVICE_01, FUEL_TYPE, 1, 1));
}

/*  Find ethanol fuel in %
Return:
 -------
  * double - Ethanol fuel in %*/
double OBD2::ethanolPercent() {
  return processPID(SERVICE_01, ETHANOL_FUEL_PERCENT, 1, 1, 100.0 / 255.0);
}

/*  Find absolute evap. system vapor
pressure in kPa Return:
 -------
  * double - Absolute evap. system vapor pressure in kPa
*/
double OBD2::absEvapSysVapPressure() {
  return processPID(SERVICE_01, ABS_EVAP_SYS_VAPOR_PRESSURE, 1, 2, 1.0 / 200.0);
}

/*  Find evap. system vapor pressure in Pa
Return:
 -------
  * double - Evap. system vapor pressure in Pa
*/
double OBD2::evapSysVapPressure2() {
  return processPID(SERVICE_01, EVAP_SYS_VAPOR_PRESSURE, 1, 2, 1, -32767);
}

/*
constexpr uint8_t SHORT_TERM_SEC_OXY_SENS_TRIM_1_3 = 85;  // 0x55 - %
constexpr uint8_t LONG_TERM_SEC_OXY_SENS_TRIM_1_3  = 86;  // 0x56 - %
constexpr uint8_t SHORT_TERM_SEC_OXY_SENS_TRIM_2_4 = 87;  // 0x57 - %
constexpr uint8_t LONG_TERM_SEC_OXY_SENS_TRIM_2_4  = 88;  // 0x58 - %
*/

/*  Find absolute fuel rail pressure in kPa
Return:
 -------
  * double - absolute fuel rail pressure in kPa
*/
double OBD2::absFuelRailPressure() {
  return processPID(SERVICE_01, FUEL_RAIL_ABS_PRESSURE, 1, 2, 10.0);
}

/*  Find relative accelerator pedal position in %
Return:
 -------
  * double - Relative accelerator pedal position in %*/
double OBD2::relativePedalPos() {
  return processPID(
      SERVICE_01, RELATIVE_ACCELERATOR_PEDAL_POS, 1, 1, 100.0 / 255.0);
}

/*  Find hybrid battery pack remaining life in %
Return:
 -------
  * double - Hybrid battery pack remaining life in %*/
double OBD2::hybridBatLife() {
  return processPID(
      SERVICE_01, HYBRID_BATTERY_REMAINING_LIFE, 1, 1, 100.0 / 255.0);
}

/*  Find engine oil temperature in C
Return:
 -------
  * double - Engine oil temperature in C
*/
double OBD2::oilTemp() {
  return processPID(SERVICE_01, ENGINE_OIL_TEMP, 1, 1, 1, -40.0);
}

/*  Find fuel injection timing in degrees
Return:
 -------
  * double - Fuel injection timing in degrees
*/
double OBD2::fuelInjectTiming() {
  return processPID(
      SERVICE_01, FUEL_INJECTION_TIMING, 1, 2, 1.0 / 128.0, -210.0);
}

/*  Find engine fuel rate in L/h
Return:
 -------
  * double - Engine fuel rate in L/h
*/
double OBD2::fuelRate() {
  return processPID(SERVICE_01, ENGINE_FUEL_RATE, 1, 2, 1.0 / 20.0);
}

/*  Find emission requirements to which vehicle is
designed Return:
 -------
  * uint8_t - Bit encoded (?)*/
uint8_t OBD2::emissionRqmts() {
  return static_cast<uint8_t>(
      processPID(SERVICE_01, EMISSION_REQUIREMENTS, 1, 1));
}