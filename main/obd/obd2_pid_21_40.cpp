#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "obd2.h"

/*  Find distance traveled with malfunction
indicator lamp (MIL) on in km Return:
 -------
  * uint16_t - Distance traveled with malfunction indicator lamp (MIL) on in
km
*/
uint16_t OBD2::distTravelWithMIL() {
  return (uint16_t)processPID(SERVICE_01, DISTANCE_TRAVELED_WITH_MIL_ON, 1, 2);
}

/*  Find fuel Rail Pressure (relative to manifold
vacuum) in kPa Return:
 -------
  * double - Fuel Rail Pressure (relative to manifold vacuum) in kPa
*/
double OBD2::fuelRailPressure() {
  return processPID(SERVICE_01, FUEL_RAIL_PRESSURE, 1, 2, 0.079);
}

/*  Find fuel Rail Gauge Pressure (diesel,
or gasoline direct injection) in kPa Return:
 -------
  * double - Fuel Rail Gauge Pressure (diesel, or gasoline direct injection) in
 kPa
*/
double OBD2::fuelRailGuagePressure() {
  return processPID(SERVICE_01, FUEL_RAIL_GUAGE_PRESSURE, 1, 2, 10.0);
}

/*
constexpr uint8_t OXYGEN_SENSOR_1_B             = 36;  // 0x24 - ratio V
constexpr uint8_t OXYGEN_SENSOR_2_B             = 37;  // 0x25 - ratio V
constexpr uint8_t OXYGEN_SENSOR_3_B             = 38;  // 0x26 - ratio V
constexpr uint8_t OXYGEN_SENSOR_4_B             = 39;  // 0x27 - ratio V
constexpr uint8_t OXYGEN_SENSOR_5_B             = 40;  // 0x28 - ratio V
constexpr uint8_t OXYGEN_SENSOR_6_B             = 41;  // 0x29 - ratio V
constexpr uint8_t OXYGEN_SENSOR_7_B             = 42;  // 0x2A - ratio V
constexpr uint8_t OXYGEN_SENSOR_8_B             = 43;  // 0x2B - ratio V
*/

/*  Find commanded Exhaust Gas Recirculation (EGR) in
% Return:
 -------
  * double - Commanded Exhaust Gas Recirculation (EGR) in %*/
double OBD2::commandedEGR() {
  return processPID(SERVICE_01, COMMANDED_EGR, 1, 1, 100.0 / 255.0);
}

/*  Find Exhaust Gas Recirculation (EGR) error in %
Return:
 -------
  * double - Exhaust Gas Recirculation (EGR) error in %*/
double OBD2::egrError() {
  return processPID(SERVICE_01, EGR_ERROR, 1, 1, 100.0 / 128.0, -100);
}

/*  Find commanded evaporative purge in %
Return:
 -------
  * double - Commanded evaporative purge in %*/
double OBD2::commandedEvapPurge() {
  return processPID(
      SERVICE_01, COMMANDED_EVAPORATIVE_PURGE, 1, 1, 100.0 / 255.0);
}

/*  Find fuel tank level input in %
Return:
 -------
  * double - Fuel tank level input in %*/
double OBD2::fuelLevel() {
  return processPID(SERVICE_01, FUEL_TANK_LEVEL_INPUT, 1, 1, 100.0 / 255.0);
}

/*  Find num warm-ups since codes
cleared Return:
 -------
  * uint8_t - Num warm-ups since codes cleared
*/
uint8_t OBD2::warmUpsSinceCodesCleared() {
  return (uint8_t)processPID(SERVICE_01, WARM_UPS_SINCE_CODES_CLEARED, 1, 1);
}

/*  Find distance traveled since codes
cleared in km Return:
 -------
  * uint16_t - Distance traveled since codes cleared in km
*/
uint16_t OBD2::distSinceCodesCleared() {
  return (uint16_t)processPID(SERVICE_01, DIST_TRAV_SINCE_CODES_CLEARED, 1, 2);
}

/*  Find evap. system vapor pressure in Pa
Return:
 -------
  * double - Evap. system vapor pressure in Pa
*/
double OBD2::evapSysVapPressure() {
  return processPID(SERVICE_01, EVAP_SYSTEM_VAPOR_PRESSURE, 1, 2, 1.0 / 4.0);
}

/*  Find absolute barometric pressure in kPa
Return:
 -------
  * uint8_t - Absolute barometric pressure in kPa
*/
uint8_t OBD2::absBaroPressure() {
  return (uint8_t)processPID(SERVICE_01, ABS_BAROMETRIC_PRESSURE, 1, 1);
}

/*
constexpr uint8_t OXYGEN_SENSOR_1_C             = 52;  // 0x34 - ratio mA
constexpr uint8_t OXYGEN_SENSOR_2_C             = 53;  // 0x35 - ratio mA
constexpr uint8_t OXYGEN_SENSOR_3_C             = 54;  // 0x36 - ratio mA
constexpr uint8_t OXYGEN_SENSOR_4_C             = 55;  // 0x37 - ratio mA
constexpr uint8_t OXYGEN_SENSOR_5_C             = 56;  // 0x38 - ratio mA
constexpr uint8_t OXYGEN_SENSOR_6_C             = 57;  // 0x39 - ratio mA
constexpr uint8_t OXYGEN_SENSOR_7_C             = 58;  // 0x3A - ratio mA
constexpr uint8_t OXYGEN_SENSOR_8_C             = 59;  // 0x3B - ratio mA
*/

//  Find catalyst temperature in C
double OBD2::catTempB1S1() {
  return processPID(
      SERVICE_01, CATALYST_TEMP_BANK_1_SENSOR_1, 1, 2, 1.0 / 10.0, -40.0);
}

//  Find catalyst temperature in C
double OBD2::catTempB2S1() {
  return processPID(
      SERVICE_01, CATALYST_TEMP_BANK_2_SENSOR_1, 1, 2, 1.0 / 10.0, -40.0);
}

//  Find catalyst temperature in C
double OBD2::catTempB1S2() {
  return processPID(
      SERVICE_01, CATALYST_TEMP_BANK_1_SENSOR_2, 1, 2, 1.0 / 10.0, -40.0);
}

//  Find catalyst temperature in C
double OBD2::catTempB2S2() {
  return processPID(
      SERVICE_01, CATALYST_TEMP_BANK_2_SENSOR_2, 1, 2, 1.0 / 10.0, -40.0);
}