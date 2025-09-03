#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esp_log.h"
#include "obd2.h"

/*  Find catalyst temperature in C
Return:
 -------
  * float - Catalyst temperature in C
*/
float OBD2::catTempB2S1() {
  return processPID(
      SERVICE_01, CATALYST_TEMP_BANK_2_SENSOR_1, 1, 2, 1.0 / 10.0, -40.0);
}

/*  Find catalyst temperature in C
Return:
 -------
  * float - Catalyst temperature in C
*/
float OBD2::catTempB1S2() {
  return processPID(
      SERVICE_01, CATALYST_TEMP_BANK_1_SENSOR_2, 1, 2, 1.0 / 10.0, -40.0);
}

/*  Find catalyst temperature in C
Return:
 -------
  * float - Catalyst temperature in C
*/
float OBD2::catTempB2S2() {
  return processPID(
      SERVICE_01, CATALYST_TEMP_BANK_2_SENSOR_2, 1, 2, 1.0 / 10.0, -40.0);
}

/* Determine which of PIDs 0x41 through
0x60 are supported (bit encoded) Return:
 -------
  * uint32_t - Bit encoded booleans of supported PIDs 0x41-0x60
*/
uint32_t OBD2::supportedPIDs_41_60() {
  return (uint32_t)processPID(SERVICE_01, SUPPORTED_PIDS_41_60, 1, 4);
}

/*  Find status this drive cycle
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_41)
Return:
 -------
  * uint32_t - Bit encoded status
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_41)*/
uint32_t OBD2::monitorDriveCycleStatus() {
  return (uint32_t)processPID(
      SERVICE_01, MONITOR_STATUS_THIS_DRIVE_CYCLE, 1, 4);
}

/*  Find control module voltage in V
Return:
 -------
  * float - Control module voltage in V
*/
float OBD2::ctrlModVoltage() {
  return processPID(SERVICE_01, CONTROL_MODULE_VOLTAGE, 1, 2, 1.0 / 1000.0);
}

/*  Find absolute load value in %
Return:
 -------
  * float - Absolute load value in %*/
float OBD2::absLoad() {
  return processPID(SERVICE_01, ABS_LOAD_VALUE, 1, 2, 100.0 / 255.0);
}

/*  Find commanded air-fuel equivalence
ratio Return:
 -------
  * float - Commanded air-fuel equivalence ratio
*/
float OBD2::commandedAirFuelRatio() {
  return processPID(
      SERVICE_01, FUEL_AIR_COMMANDED_EQUIV_RATIO, 1, 2, 2.0 / 65536.0);
}

/*  Find relative throttle position in %
Return:
 -------
  * float - Relative throttle position in %*/
float OBD2::relativeThrottle() {
  return processPID(
      SERVICE_01, RELATIVE_THROTTLE_POSITION, 1, 1, 100.0 / 255.0);
}

/*  Find ambient air temperature in C
Return:
 -------
  * float - Ambient air temperature in C
*/
float OBD2::ambientAirTemp() {
  return processPID(SERVICE_01, AMBIENT_AIR_TEMP, 1, 1, 1, -40);
}

/*  Find absolute throttle position B in %
Return:
 -------
  * float - Absolute throttle position B in %*/
float OBD2::absThrottlePosB() {
  return processPID(SERVICE_01, ABS_THROTTLE_POSITION_B, 1, 1, 100.0 / 255.0);
}

/*  Find absolute throttle position C in %
Return:
 -------
  * float - Absolute throttle position C in %*/
float OBD2::absThrottlePosC() {
  return processPID(SERVICE_01, ABS_THROTTLE_POSITION_C, 1, 1, 100.0 / 255.0);
}

/*  Find absolute throttle position D in %
Return:
 -------
  * float - Absolute throttle position D in %*/
float OBD2::absThrottlePosD() {
  return processPID(SERVICE_01, ABS_THROTTLE_POSITION_D, 1, 1, 100.0 / 255.0);
}

/*  Find absolute throttle position E in %
Return:
 -------
  * float - Absolute throttle position E in %*/
float OBD2::absThrottlePosE() {
  return processPID(SERVICE_01, ABS_THROTTLE_POSITION_E, 1, 1, 100.0 / 255.0);
}

/*  Find absolute throttle position F in %
Return:
 -------
  * float - Absolute throttle position F in %*/
float OBD2::absThrottlePosF() {
  return processPID(SERVICE_01, ABS_THROTTLE_POSITION_F, 1, 1, 100.0 / 255.0);
}

/*  Find commanded throttle actuator in
% Return:
 -------
  * float - Commanded throttle actuator in %*/
float OBD2::commandedThrottleActuator() {
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

/*  Find maximum value for air flow rate from mass air
flow sensor in g/s Return:
 -------
  * float - Maximum value for air flow rate from mass air flow sensor in g/s
*/
float OBD2::maxMafRate() {
  return processPID(SERVICE_01, MAX_MAF_RATE, 1, 1, 10.0);
}

/*  Find fuel type
(https://en.wikipedia.org/wiki/OBD-II_PIDs#Fuel_Type_Coding) Return:
 -------
  * uint8_t - Bit encoded
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Fuel_Type_Coding)*/
uint8_t OBD2::fuelType() {
  return (uint8_t)processPID(SERVICE_01, FUEL_TYPE, 1, 1);
}

/*  Find ethanol fuel in %
Return:
 -------
  * float - Ethanol fuel in %*/
float OBD2::ethanolPercent() {
  return processPID(SERVICE_01, ETHANOL_FUEL_PERCENT, 1, 1, 100.0 / 255.0);
}

/*  Find absolute evap. system vapor
pressure in kPa Return:
 -------
  * float - Absolute evap. system vapor pressure in kPa
*/
float OBD2::absEvapSysVapPressure() {
  return processPID(SERVICE_01, ABS_EVAP_SYS_VAPOR_PRESSURE, 1, 2, 1.0 / 200.0);
}

/*  Find evap. system vapor pressure in Pa
Return:
 -------
  * float - Evap. system vapor pressure in Pa
*/
float OBD2::evapSysVapPressure2() {
  return processPID(SERVICE_01, EVAP_SYS_VAPOR_PRESSURE, 1, 2, 1, -32767);
}

/*  Find absolute fuel rail pressure in kPa
Return:
 -------
  * float - absolute fuel rail pressure in kPa
*/
float OBD2::absFuelRailPressure() {
  return processPID(SERVICE_01, FUEL_RAIL_ABS_PRESSURE, 1, 2, 10.0);
}

/*  Find relative accelerator pedal position in %
Return:
 -------
  * float - Relative accelerator pedal position in %*/
float OBD2::relativePedalPos() {
  return processPID(
      SERVICE_01, RELATIVE_ACCELERATOR_PEDAL_POS, 1, 1, 100.0 / 255.0);
}

/*  Find hybrid battery pack remaining life in %
Return:
 -------
  * float - Hybrid battery pack remaining life in %*/
float OBD2::hybridBatLife() {
  return processPID(
      SERVICE_01, HYBRID_BATTERY_REMAINING_LIFE, 1, 1, 100.0 / 255.0);
}

/*  Find engine oil temperature in C
Return:
 -------
  * float - Engine oil temperature in C
*/
float OBD2::oilTemp() {
  return processPID(SERVICE_01, ENGINE_OIL_TEMP, 1, 1, 1, -40.0);
}

/*  Find fuel injection timing in degrees
Return:
 -------
  * float - Fuel injection timing in degrees
*/
float OBD2::fuelInjectTiming() {
  return processPID(
      SERVICE_01, FUEL_INJECTION_TIMING, 1, 2, 1.0 / 128.0, -210.0);
}

/*  Find engine fuel rate in L/h
Return:
 -------
  * float - Engine fuel rate in L/h
*/
float OBD2::fuelRate() {
  return processPID(SERVICE_01, ENGINE_FUEL_RATE, 1, 2, 1.0 / 20.0);
}

/*  Find emission requirements to which vehicle is
designed Return:
 -------
  * uint8_t - Bit encoded (?)*/
uint8_t OBD2::emissionRqmts() {
  return (uint8_t)processPID(SERVICE_01, EMISSION_REQUIREMENTS, 1, 1);
}

/* Determine which of PIDs 0x61 through
0x80 are supported (bit encoded) Return:
 -------
  * uint32_t - Bit encoded booleans of supported PIDs 0x61-0x80
*/
uint32_t OBD2::supportedPIDs_61_80() {
  return (uint32_t)processPID(SERVICE_01, SUPPORTED_PIDS_61_80, 1, 4);
}

/*  Find driver's demanded engine torque in %
Return:
 -------
  * float - Driver's demanded engine torque in %*/
float OBD2::demandedTorque() {
  return processPID(
      SERVICE_01, DEMANDED_ENGINE_PERCENT_TORQUE, 1, 1, 1, -125.0);
}

/*  Find actual engine torque in %
Return:
 -------
  * float - Actual engine torque in %*/
float OBD2::torque() {
  return processPID(SERVICE_01, ACTUAL_ENGINE_TORQUE, 1, 1, 1, -125.0);
}

/*  Find engine reference torque in Nm
Return:
 -------
  * uint16_t - Engine reference torque in Nm
*/
uint16_t OBD2::referenceTorque() {
  return processPID(SERVICE_01, ENGINE_REFERENCE_TORQUE, 1, 2);
}

/*  Find auxiliary input/output supported
Return:
 -------
  * uint16_t - Bit encoded (?)*/
uint16_t OBD2::auxSupported() {
  return (uint16_t)processPID(SERVICE_01, AUX_INPUT_OUTPUT_SUPPORTED, 1, 2);
}

/* Checks if a particular PID is
   supported by the connected ECU.

    * This is a convenience method that selects the correct
   supportedPIDS_xx_xx() query and parses the bit-encoded result, returning a
   simple Boolean value indicating PID support from the ECU.

   Inputs:
   -------
    * uint8_t pid - the PID to check for support.

   Return:
   -------
    * bool - Whether or not the queried PID is supported by the ECU.*/
bool OBD2::isPidSupported(uint8_t pid) {
  uint8_t pidInterval = (pid / PID_INTERVAL_OFFSET) * PID_INTERVAL_OFFSET;

  switch (pidInterval) {
    case SUPPORTED_PIDS_1_20:
      supportedPIDs_1_20();
      break;

    case SUPPORTED_PIDS_21_40:
      supportedPIDs_21_40();
      pid = (pid - SUPPORTED_PIDS_21_40);
      break;

    case SUPPORTED_PIDS_41_60:
      supportedPIDs_41_60();
      pid = (pid - SUPPORTED_PIDS_41_60);
      break;

    case SUPPORTED_PIDS_61_80:
      supportedPIDs_61_80();
      pid = (pid - SUPPORTED_PIDS_61_80);
      break;

    default:
      break;
  }

  if (nb_rx_state == OBD_SUCCESS) {
    return ((response >> (32 - pid)) & 0x1);
  }
  return false;
}

double OBD2::calculator_0C() {
  return (double)((response_A << 8) | response_B) / 4;
}

double OBD2::calculator_10() {
  return (double)((response_A << 8) | response_B) / 100;
}

double OBD2::calculator_14() {
  return (double)(response_A / 200);
}

double OBD2::calculator_1F() {
  return (double)((response_A << 8) | response_B);
}

double OBD2::calculator_22() {
  return (double)((response_A << 8) | response_B) * 0.079;
}

double OBD2::calculator_23() {
  return (double)((response_A << 8) | response_B) * 10;
}

double OBD2::calculator_32() {
  return (double)((int16_t)((response_A << 8) | response_B)) / 4.0;
}

double OBD2::calculator_3C() {
  return (double)(((response_A << 8) | response_B) / 10) - 40;
}

double OBD2::calculator_42() {
  return (double)((response_A << 8) | response_B) / 1000;
}

double OBD2::calculator_43() {
  return (double)((response_A << 8) | response_B) * (100.0 / 255.0);
}

double OBD2::calculator_44() {
  return ((double)((response_A << 8) | response_B) * 2.0) / 65536.0;
}

double OBD2::calculator_4F() {
  return (double)(response_A);
}

double OBD2::calculator_50() {
  return (double)(response_A * 10.0);
}

double OBD2::calculator_53() {
  return (double)((response_A << 8) | response_B) / 200;
}

double OBD2::calculator_54() {
  return (double)((int16_t)((response_A << 8) | response_B));
}

double OBD2::calculator_55() {
  return ((double)response_A * (100.0 / 128.0)) - 100.0;
}

// calc 23
double OBD2::calculator_59() {
  return (double)((response_A << 8) | response_B) * 10;
}

double OBD2::calculator_5D() {
  return (double)(((response_A << 8) | response_B) / 128) - 210;
}

double OBD2::calculator_5E() {
  return (double)((response_A << 8) | response_B) / 20;
}

double OBD2::calculator_61() {
  return (double)response_A - 125;
}