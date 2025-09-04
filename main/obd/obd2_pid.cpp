#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esp_log.h"
#include "obd2.h"

// Determine which of PIDs 0x1 through 0x20 are supported (bit encoded)
// uint32_t - Bit encoded booleans of supported PIDs 0x1-0x20
uint32_t OBD2::supportedPIDs_1_20() {
  return (uint32_t)processPID(SERVICE_01, SUPPORTED_PIDS_1_20, 1, 4);
}

// Determine which of PIDs 0x1 through 0x20 are supported (bit encoded)
// uint32_t - Bit encoded booleans of supported PIDs 0x21-0x20
uint32_t OBD2::supportedPIDs_21_40() {
  return (uint32_t)processPID(SERVICE_01, SUPPORTED_PIDS_21_40, 1, 4);
}

// Determine which of PIDs 0x41 through 0x60 are supported (bit encoded)
// uint32_t - Bit encoded booleans of supported PIDs 0x41-0x60
uint32_t OBD2::supportedPIDs_41_60() {
  return (uint32_t)processPID(SERVICE_01, SUPPORTED_PIDS_41_60, 1, 4);
}

// Determine which of PIDs 0x61 through 0x80 are supported (bit encoded)
// uint32_t - Bit encoded booleans of supported PIDs 0x61-0x80
uint32_t OBD2::supportedPIDs_61_80() {
  return (uint32_t)processPID(SERVICE_01, SUPPORTED_PIDS_61_80, 1, 4);
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

/* Selects the appropriate calculation function for a given PID.

 Inputs:
 -------
  * uint16_t pid             - The Parameter ID (PID) from the service

 Return:
 -------
  * double (*func()) - Pointer to a function to be used to calculate the value
 for this PID. Returns nullptr if the PID is calculated using the default
 scaleFactor + Bias formula as implemented in conditionResponse(). (Maintained
 for backward compatibility)*/
double (*OBD2::selectCalculator(uint16_t pid))() {
  switch (pid) {
    case ENGINE_LOAD:
    case ENGINE_COOLANT_TEMP:
    case SHORT_TERM_FUEL_TRIM_BANK_1:
    case LONG_TERM_FUEL_TRIM_BANK_1:
    case SHORT_TERM_FUEL_TRIM_BANK_2:
    case LONG_TERM_FUEL_TRIM_BANK_2:
    case FUEL_PRESSURE:
    case INTAKE_MANIFOLD_ABS_PRESSURE:
    case VEHICLE_SPEED:
    case TIMING_ADVANCE:
    case INTAKE_AIR_TEMP:
    case THROTTLE_POSITION:
    case COMMANDED_EGR:
    case EGR_ERROR:
    case COMMANDED_EVAPORATIVE_PURGE:
    case FUEL_TANK_LEVEL_INPUT:
    case WARM_UPS_SINCE_CODES_CLEARED:
    case ABS_BAROMETRIC_PRESSURE:
    case RELATIVE_THROTTLE_POSITION:
    case AMBIENT_AIR_TEMP:
    case ABS_THROTTLE_POSITION_B:
    case ABS_THROTTLE_POSITION_C:
    case ABS_THROTTLE_POSITION_D:
    case ABS_THROTTLE_POSITION_E:
    case ABS_THROTTLE_POSITION_F:
    case COMMANDED_THROTTLE_ACTUATOR:
    case ETHANOL_FUEL_PERCENT:
    case RELATIVE_ACCELERATOR_PEDAL_POS:
    case HYBRID_BATTERY_REMAINING_LIFE:
    case ENGINE_OIL_TEMP:
    case DEMANDED_ENGINE_PERCENT_TORQUE:
    case ACTUAL_ENGINE_TORQUE:
      return nullptr;

    case ENGINE_RPM:
      return []() -> double {
        return ((response_A << 8) | response_B) / 4.0;
      };

    case MAF_FLOW_RATE:
      return []() -> double {
        return ((response_A << 8) | response_B) / 100.0;
      };

    case OXYGEN_SENSOR_1_A:
    case OXYGEN_SENSOR_2_A:
    case OXYGEN_SENSOR_3_A:
    case OXYGEN_SENSOR_4_A:
    case OXYGEN_SENSOR_5_A:
    case OXYGEN_SENSOR_6_A:
    case OXYGEN_SENSOR_7_A:
    case OXYGEN_SENSOR_8_A:
    case OXYGEN_SENSOR_1_B:
    case OXYGEN_SENSOR_2_B:
    case OXYGEN_SENSOR_3_B:
    case OXYGEN_SENSOR_4_B:
    case OXYGEN_SENSOR_6_B:
    case OXYGEN_SENSOR_7_B:
    case OXYGEN_SENSOR_8_B:
    case OXYGEN_SENSOR_1_C:
    case OXYGEN_SENSOR_2_C:
    case OXYGEN_SENSOR_3_C:
    case OXYGEN_SENSOR_4_C:
    case OXYGEN_SENSOR_5_C:
    case OXYGEN_SENSOR_6_C:
    case OXYGEN_SENSOR_7_C:
    case OXYGEN_SENSOR_8_C:
      return []() -> double {
        return response_A / 200.0;
      };

    case RUN_TIME_SINCE_ENGINE_START:
    case DISTANCE_TRAVELED_WITH_MIL_ON:
    case DIST_TRAV_SINCE_CODES_CLEARED:
    case TIME_RUN_WITH_MIL_ON:
    case TIME_SINCE_CODES_CLEARED:
    case ENGINE_REFERENCE_TORQUE:
      return []() -> double {
        return (response_A << 8) | response_B;
      };

    case FUEL_RAIL_PRESSURE:
      return []() -> double {
        return ((response_A << 8) | response_B) * 0.079;
      };

    case FUEL_RAIL_GUAGE_PRESSURE:
    case FUEL_RAIL_ABS_PRESSURE:
      return []() -> double {
        return ((response_A << 8) | response_B) * 10.0;
      };

    case EVAP_SYSTEM_VAPOR_PRESSURE:
      return []() -> double {
        return ((response_A << 8) | response_B) / 4.0;
      };

    case CATALYST_TEMP_BANK_1_SENSOR_1:
    case CATALYST_TEMP_BANK_2_SENSOR_1:
    case CATALYST_TEMP_BANK_1_SENSOR_2:
    case CATALYST_TEMP_BANK_2_SENSOR_2:
      return []() -> double {
        return (((response_A << 8) | response_B) / 10) - 40.0;
      };

    case CONTROL_MODULE_VOLTAGE:
      return []() -> double {
        return ((response_A << 8) | response_B) / 1000.0;
      };

    case ABS_LOAD_VALUE:
      return []() -> double {
        return ((response_A << 8) | response_B) * (100.0 / 255.0);
      };

    case FUEL_AIR_COMMANDED_EQUIV_RATIO:
      return []() -> double {
        return ((response_A << 8) | response_B) * 2.0 / 65536.0;
      };

    case MAX_VALUES_EQUIV_V_I_PRESSURE:
      return []() -> double {
        return response_A;
      };

    case MAX_MAF_RATE:
      return []() -> double {
        return response_A * 10.0;
      };

    case ABS_EVAP_SYS_VAPOR_PRESSURE:
      return []() -> double {
        return ((response_A << 8) | response_B) / 200.0;
      };

    case SHORT_TERM_SEC_OXY_SENS_TRIM_1_3:
    case LONG_TERM_SEC_OXY_SENS_TRIM_1_3:
    case SHORT_TERM_SEC_OXY_SENS_TRIM_2_4:
    case LONG_TERM_SEC_OXY_SENS_TRIM_2_4:
      return []() -> double {
        return ((double)response_A * (100.0 / 128.0)) - 100.0;
      };

    case FUEL_INJECTION_TIMING:
      return []() -> double {
        return (((response_A << 8) | response_B) / 128) - 210;
      };

    case ENGINE_FUEL_RATE:
      return []() -> double {
        return ((response_A << 8) | response_B) / 20.0;
      };

    default:
      return nullptr;
  }
}