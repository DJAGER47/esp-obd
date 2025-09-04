#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "obd2.h"

/*  Find driver's demanded engine torque in %
Return:
 -------
  * double - Driver's demanded engine torque in %*/
double OBD2::demandedTorque() {
  return processPID(
      SERVICE_01, DEMANDED_ENGINE_PERCENT_TORQUE, 1, 1, 1, -125.0);
}

/*  Find actual engine torque in %
Return:
 -------
  * double - Actual engine torque in %*/
double OBD2::torque() {
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

/*
constexpr uint8_t ENGINE_PERCENT_TORQUE_DATA     = 100;  // 0x64 - %
*/

/*  Find auxiliary input/output supported
Return:
 -------
  * uint16_t - Bit encoded (?)*/
uint16_t OBD2::auxSupported() {
  return (uint16_t)processPID(SERVICE_01, AUX_INPUT_OUTPUT_SUPPORTED, 1, 2);
}