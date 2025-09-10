#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "obd2.h"

/* Monitor status since DTCs cleared (Includes malfunction indicator
  lamp (MIL) status and number of DTCs). See
 https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_01 for more info
Return:
 -------
  * uint32_t - Bit encoded status*/
uint32_t OBD2::monitorStatus() {
  return (uint32_t)processPID(
      SERVICE_01, MONITOR_STATUS_SINCE_DTC_CLEARED, 1, 4);
}

/* Freeze DTC - see https://www.samarins.com/diagnose/freeze-frame.html for
 more info
Return:
 -------
  * uint16_t - Various vehicle information*/
uint16_t OBD2::freezeDTC() {
  return (uint16_t)processPID(SERVICE_01, FREEZE_DTC, 1, 2);
}

/* Freeze DTC - see
https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_03 for more info
Return:
 -------
  * uint16_t - Bit encoded status
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_03)*/
uint16_t OBD2::fuelSystemStatus() {
  return (uint16_t)processPID(SERVICE_01, FUEL_SYSTEM_STATUS, 1, 2);
}

/* Find the current engine load in %
Return:
 -------
  * double - Engine load %*/
double OBD2::engineLoad() {
  return processPID(SERVICE_01, ENGINE_LOAD, 1, 1, 100.0 / 255.0);
}

/* Find the current engine coolant temp in C
Return:
 -------
  * double - Engine load %*/
double OBD2::engineCoolantTemp() {
  return processPID(SERVICE_01, ENGINE_COOLANT_TEMP, 1, 1, 1, -40.0);
}

/* Find fuel trim %
Return:
 -------
  * double - Fuel trim %*/
double OBD2::shortTermFuelTrimBank_1() {
  return processPID(
      SERVICE_01, SHORT_TERM_FUEL_TRIM_BANK_1, 1, 1, 100.0 / 128.0, -100.0);
}

/* Find fuel trim %
Return:
 -------
  * double - Fuel trim %*/
double OBD2::longTermFuelTrimBank_1() {
  return processPID(
      SERVICE_01, LONG_TERM_FUEL_TRIM_BANK_1, 1, 1, 100.0 / 128.0, -100.0);
}

/* Find fuel trim %
Return:
 -------
  * double - Fuel trim %*/
double OBD2::shortTermFuelTrimBank_2() {
  return processPID(
      SERVICE_01, SHORT_TERM_FUEL_TRIM_BANK_2, 1, 1, 100.0 / 128.0, -100.0);
}

/* Find fuel trim %
Return:
 -------
  * double - Fuel trim %*/
double OBD2::longTermFuelTrimBank_2() {
  return processPID(
      SERVICE_01, LONG_TERM_FUEL_TRIM_BANK_2, 1, 1, 100.0 / 128.0, -100.0);
}

/* Find fuel pressure in kPa
Return:
 -------
  * double - Fuel pressure in kPa
*/
double OBD2::fuelPressure() {
  return processPID(SERVICE_01, FUEL_PRESSURE, 1, 1, 3.0);
}

/* Find intake manifold absolute pressure in kPa
Return:
 -------
  * uint8_t - Intake manifold absolute pressure in kPa
*/
uint8_t OBD2::manifoldPressure() {
  return static_cast<uint8_t>(
      processPID(SERVICE_01, INTAKE_MANIFOLD_ABS_PRESSURE, 1, 1));
}

/* Queries and parses received message for/returns vehicle RMP data
Return:
 -------
  * double - Vehicle RPM
*/
double OBD2::rpm() {
  return processPID(SERVICE_01, ENGINE_RPM, 1, 2, 1.0 / 4.0);
}

/*  Queries and parses received message for/returns vehicle speed data (kph)
Return:
 -------
  * int32_t - Vehicle speed in kph
*/
int32_t OBD2::kph() {
  return (int32_t)processPID(SERVICE_01, VEHICLE_SPEED, 1, 1);
}

/*
 double OBD2::timingAdvance() *  Find timing advance in degrees before Top Dead
Center (TDC) Return:
 -------
  * double - Timing advance in degrees before Top Dead Center (TDC)*/
double OBD2::timingAdvance() {
  return processPID(SERVICE_01, TIMING_ADVANCE, 1, 1, 1.0 / 2.0, -64.0);
}

/*  Find intake air temperature in C
Return:
 -------
  * double - Intake air temperature in C
*/
double OBD2::intakeAirTemp() {
  return processPID(SERVICE_01, INTAKE_AIR_TEMP, 1, 1, 1, -40.0);
}

/*  Find mass air flow sensor (MAF) air flow rate rate in g/s
Return:
 -------
  * double - Mass air flow sensor (MAF) air flow rate rate in g/s
*/
double OBD2::mafRate() {
  return processPID(SERVICE_01, MAF_FLOW_RATE, 1, 2, 1.0 / 100.0);
}

/*  Find throttle position in %
Return:
 -------
  * double - Throttle position in %*/
double OBD2::throttle() {
  return processPID(SERVICE_01, THROTTLE_POSITION, 1, 1, 100.0 / 255.0);
}

/*  Find commanded secondary air status
Return:
 -------
  * uint8_t - Bit encoded status
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_12)*/
uint8_t OBD2::commandedSecAirStatus() {
  return static_cast<uint8_t>(
      processPID(SERVICE_01, COMMANDED_SECONDARY_AIR_STATUS, 1, 1));
}

/*  Find which oxygen sensors are
present ([A0..A3] == Bank 1, Sensors 1-4. [A4..A7] == Bank 2...) Return:
 -------
  * uint8_t - Bit encoded
*/
uint8_t OBD2::oxygenSensorsPresent_2banks() {
  return static_cast<uint8_t>(
      processPID(SERVICE_01, OXYGEN_SENSORS_PRESENT_2_BANKS, 1, 1));
}

/*
constexpr uint8_t OXYGEN_SENSOR_1_A              = 20;  // 0x14 - V %
constexpr uint8_t OXYGEN_SENSOR_2_A              = 21;  // 0x15 - V %
constexpr uint8_t OXYGEN_SENSOR_3_A              = 22;  // 0x16 - V %
constexpr uint8_t OXYGEN_SENSOR_4_A              = 23;  // 0x17 - V %
constexpr uint8_t OXYGEN_SENSOR_5_A              = 24;  // 0x18 - V %
constexpr uint8_t OXYGEN_SENSOR_6_A              = 25;  // 0x19 - V %
constexpr uint8_t OXYGEN_SENSOR_7_A              = 26;  // 0x1A - V %
constexpr uint8_t OXYGEN_SENSOR_8_A              = 27;  // 0x1B - V %
*/

/*  Find the OBD standards this vehicle conforms to
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_1C)

Return:
 -------
  * uint8_t - Bit encoded
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_1C)*/
uint8_t OBD2::obdStandards() {
  return static_cast<uint8_t>(processPID(SERVICE_01, OBD_STANDARDS, 1, 1));
}

/*  Find which oxygen sensors are
present (Similar to PID 13, but [A0..A7] == [B1S1, B1S2, B2S1, B2S2, B3S1,
B3S2, B4S1, B4S2]) Return:
 -------
  * uint8_t - Bit encoded
*/
uint8_t OBD2::oxygenSensorsPresent_4banks() {
  return static_cast<uint8_t>(
      processPID(SERVICE_01, OXYGEN_SENSORS_PRESENT_4_BANKS, 1, 1));
}

/*  Find Power Take Off (PTO) status
Return:
 -------
  * bool - Power Take Off (PTO) status
*/
bool OBD2::auxInputStatus() {
  return (bool)processPID(SERVICE_01, AUX_INPUT_STATUS, 1, 1);
}

/*  Find run time since engine start in s
Return:
 -------
  * uint16_t - Run time since engine start in s
*/
uint16_t OBD2::runTime() {
  return (uint16_t)processPID(SERVICE_01, RUN_TIME_SINCE_ENGINE_START, 1, 2);
}
