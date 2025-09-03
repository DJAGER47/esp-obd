#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esp_log.h"
#include "obd2.h"
#include

static const char* TAG = "OBD2";

void OBD2::log_print(const char* format, ...) {
#ifdef OBD_DEBUG
  va_list args;
  va_start(args, format);
  ESP_LOGI(TAG, format, args);
  va_end(args);
#endif
}

/* Constructor for the OBD2 Class; initializes OBD2

 Inputs:
 -------
  * uint16_t timeout    - Time in ms to wait for a query response
*/
void OBD2::begin(IsoTp* iso_tp, const uint16_t& timeout) {
  timeout_ms = timeout;
  iso_tp_    = iso_tp;
}

/* Creates a query stack to be sent to OBD2

 Inputs:
 -------
  * uint16_t service - Service number of the queried PID
  * uint32_t pid     - PID number of the queried PID
  * uint8_t num_responses - see function header for "queryPID()"*/
void OBD2::formatQueryArray(const uint8_t& service,
                            const uint16_t& pid,
                            const uint8_t& num_responses) {
  log_print("Service: %d PID: %d", service, pid);

  isMode0x22Query = (service == 0x22 &&
                     pid <= 0xFF);  // mode 0x22 responses always zero-pad the
                                    // pid to 4 chars, even for a 2-char pid

  query[0] = ((service >> 4) & 0xF) + '0';
  query[1] = (service & 0xF) + '0';

  // determine PID length (standard queries have 16-bit PIDs,
  // but some custom queries have PIDs with 32-bit values)
  if (pid & 0xFF00) {
    log_print("Long query detected");

    longQuery = true;

    query[2] = ((pid >> 12) & 0xF) + '0';
    query[3] = ((pid >> 8) & 0xF) + '0';
    query[4] = ((pid >> 4) & 0xF) + '0';
    query[5] = (pid & 0xF) + '0';

    if (specifyNumResponses) {
      if (num_responses > 0xF) {
        query[6] = ((num_responses >> 4) & 0xF) + '0';
        query[7] = (num_responses & 0xF) + '0';
        query[8] = '\0';
      } else {
        query[6] = (num_responses & 0xF) + '0';
        query[7] = '\0';
        query[8] = '\0';
      }
    } else {
      query[6] = '\0';
      query[7] = '\0';
      query[8] = '\0';
    }
  } else {
    log_print("Normal length query detected");

    longQuery = false;

    query[2] = ((pid >> 4) & 0xF) + '0';
    query[3] = (pid & 0xF) + '0';

    if (specifyNumResponses) {
      if (num_responses > 0xF) {
        query[4] = ((num_responses >> 4) & 0xF) + '0';
        query[5] = (num_responses & 0xF) + '0';
        query[6] = '\0';
        query[7] = '\0';
        query[8] = '\0';
      } else {
        query[4] = (num_responses & 0xF) + '0';
        query[5] = '\0';
        query[6] = '\0';
        query[7] = '\0';
        query[8] = '\0';
      }
    } else {
      query[4] = '\0';
      query[5] = '\0';
      query[6] = '\0';
      query[7] = '\0';
      query[8] = '\0';
    }
  }

  log_print("Query string: %s", query);
}

/*
 Description:
 ------------
  * Determines if a time-out has occurred


 Return:
 -------
  * bool - whether or not a time-out has occurred
*/
bool OBD2::timeout() {
  currentTime = millis();
  if ((currentTime - previousTime) >= timeout_ms)
    return true;
  return false;
}
/* converts a decimal or hex char to an int

 Inputs:
 -------
  * uint8_t value - char to be converted

 Return:
 -------
  * uint8_t - int value of parameter "value"*/
uint8_t OBD2::ctoi(uint8_t value) {
  if (value >= 'A')
    return value - 'A' + 10;
  else
    return value - '0';
}

/* Finds and returns the first char index of
  numOccur'th instance of target in str

 Inputs:
 -------
  * char const *str    - string to search target within
  * char const *target - String to search for in str
  * uint8_t numOccur   - Which instance of target in str

 Return:
 -------
  * int8_t - First char index of numOccur'th
  instance of target in str. -1 if there is no
  numOccur'th instance of target in str
*/
int8_t OBD2::nextIndex(char const* str, char const* target, uint8_t numOccur) {
  char const* p = str;
  char const* r = str;
  uint8_t count;

  for (count = 0;; ++count) {
    p = strstr(p, target);

    if (count == (numOccur - 1))
      break;

    if (!p)
      break;

    p++;
  }

  if (!p)
    return -1;

  return p - r;
}

/* Removes all instances of each char in string "remove" from the string "from"

 Inputs:
 -------
  * char *from         - String to remove target(s) from
  * char const *remove - Chars to find/remove
*/
void OBD2::removeChar(char* from, const char* remove) {
  size_t i = 0, j = 0;
  while (from[i]) {
    if (!strchr(remove, from[i]))
      from[j++] = from[i];
    i++;
  }
  from[j] = '\0';
}
/* Converts the OBD2's response into its correct, numerical value. Returns 0 if
 numExpectedBytes > numPayChars

 Inputs:
 -------
  * uint64_t response        - OBD2's response
  * uint8_t numExpectedBytes - Number of valid bytes from the response to
 process
  * double scaleFactor       - Amount to scale the response by
  * float bias               - Amount to bias the response by

 Return:
 -------
  * double - Converted numerical value
*/
double OBD2::conditionResponse(const uint8_t& numExpectedBytes,
                               const double& scaleFactor,
                               const double& bias) {
  uint8_t numExpectedPayChars = numExpectedBytes * 2;
  uint8_t payCharDiff         = numPayChars - numExpectedPayChars;

  if (numExpectedBytes > 8) {
    log_print(
        "WARNING: Number of expected response bytes is greater than 8 - "
        "returning 0");

    return 0;
  }

  if (numPayChars < numExpectedPayChars) {
    log_print(
        "WARNING: Number of payload chars is less than the number of expected "
        "response chars returned by OBD2 - returning 0");

    return 0;
  } else if (numPayChars & 0x1) {
    log_print(
        "WARNING: Number of payload chars returned by OBD2 is an odd value - "
        "returning 0");

    return 0;
  } else if (numExpectedPayChars == numPayChars) {
    if (scaleFactor == 1 && bias == 0)  // No scale/bias needed
      return response;
    else
      return (response * scaleFactor) + bias;
  }

  // If there were more payload bytes returned than we expected, test the first
  // and last bytes in the returned payload and see which gives us a higher
  // value. Sometimes OBD2's return leading zeros and others return trailing
  // zeros. The following approach gives us the best chance at determining where
  // the real data is. Note that if the payload returns BOTH leading and
  // trailing zeros, this will not give accurate results!

  log_print("Looking for lagging zeros");

  uint16_t numExpectedBits  = numExpectedBytes * 8;
  uint64_t laggingZerosMask = 0;

  for (uint16_t i = 0; i < numExpectedBits; i++)
    laggingZerosMask |= (1 << i);

  if (!(laggingZerosMask & response))  // Detect all lagging zeros in `response`
  {
    log_print("Lagging zeros found");

    if (scaleFactor == 1 && bias == 0)  // No scale/bias needed
      return (response >> (4 * payCharDiff));
    else
      return ((response >> (4 * payCharDiff)) * scaleFactor) + bias;
  } else {
    log_print("Lagging zeros not found - assuming leading zeros");

    if (scaleFactor == 1 && bias == 0)  // No scale/bias needed
      return response;
    else
      return (response * scaleFactor) + bias;
  }
}

/* Provides a means to pass in a user-defined function to process the response.
 Used for PIDs that don't use the common scaleFactor + Bias formula to calculate
 the value from the response data. Also useful for processing OEM custom PIDs
 which are too numerous and varied to encode in the lib.

 Inputs:
 -------
  * (*func)() - pointer to function to do calculate response value

 Return:
 -------
  * double - Converted numerical value
*/

double OBD2::conditionResponse(double (*func)()) {
  return func();
}

/* Create a PID query command string and send the command

  Inputs:
  -------
  * uint8_t service       - The diagnostic service ID. 01 is "Show current data"
  * uint16_t pid          - The Parameter ID (PID) from the service
  * uint8_t num_responses - Number of lines of data to receive - see OBD
  datasheet "Talking to the vehicle". This can speed up retrieval of information
  if you know how many responses will be sent. Basically the OBD scanner will
  not wait for more responses if it does not need to go through final timeout.
  Also prevents OBD scanners from sending mulitple of the same response.

  Return:
  -------
  * void
*/
void OBD2::queryPID(const uint8_t& service,
                    const uint16_t& pid,
                    const uint8_t& num_responses) {
  formatQueryArray(service, pid, num_responses);
  sendCommand(query);
}

/* Queries OBD2 for a specific type of vehicle telemetry data

 Inputs:
 -------
  * uint8_t service          - The diagnostic service ID. 01 is "Show current
 data"
  * uint16_t pid             - The Parameter ID (PID) from the service
  * uint8_t num_responses    - Number of lines of data to receive - see OBD
 datasheet "Talking to the vehicle". This can speed up retrieval of information
 if you know how many responses will be sent. Basically the OBD scanner will not
 wait for more responses if it does not need to go through final timeout. Also
 prevents OBD scanners from sending mulitple of the same response.
  * uint8_t numExpectedBytes - Number of valid bytes from the response to
 process
  * float scaleFactor        - Amount to scale the response by
  * float bias               - Amount to bias the response by

 Return:
 -------
  * double - The PID value if successfully received, else 0.0
*/
double OBD2::processPID(const uint8_t& service,
                        const uint16_t& pid,
                        const uint8_t& num_responses,
                        const uint8_t& numExpectedBytes,
                        const double& scaleFactor,
                        const float& bias) {
  if (nb_query_state == SEND_COMMAND) {
    queryPID(service, pid, num_responses);
    nb_query_state = WAITING_RESP;
  } else if (nb_query_state == WAITING_RESP) {
    get_response();
    if (nb_rx_state == OBD_SUCCESS) {
      nb_query_state =
          SEND_COMMAND;  // Reset the query state machine for next command
      findResponse();

      /* This data manipulation seems duplicative of the responseByte_0,
         responseByte_1, etc vars and it is. The duplcation is deliberate to
         provide a clear way for the calculator functions to access the relevant
         data bytes from the response in the format they are commonly expressed
         in and without breaking backward compatability with existing code that
         may be using the responseByte_n vars.

         In addition, we need to place the response values into static vars that
         can be accessed by the (static) calculator functions. A future
         (breaking!) change could be made to eliminate this duplication.
      */
      uint8_t responseBits      = numExpectedBytes * 8;
      uint8_t extractedBytes[8] = {0};  // Store extracted bytes

      // Extract bytes only if shift is non-negative
      for (int i = 0; i < numExpectedBytes; i++) {
        int shiftAmount = responseBits - (8 * (i + 1));  // Compute shift amount
        if (shiftAmount >= 0) {                          //  Ensure valid shift
          extractedBytes[i] =
              (response >> shiftAmount) & 0xFF;  // Extract uint8_t
        }
      }

      // Assign extracted values to response_A, response_B, ..., response_H
      // safely
      response_A = extractedBytes[0];
      response_B = extractedBytes[1];
      response_C = extractedBytes[2];
      response_D = extractedBytes[3];
      response_E = extractedBytes[4];
      response_F = extractedBytes[5];
      response_G = extractedBytes[6];
      response_H = extractedBytes[7];

      double (*calculator)() = selectCalculator(pid);

      if (nullptr == calculator) {
        // Use the default scaleFactor + Bias calculation
        return conditionResponse(numExpectedBytes, scaleFactor, bias);
      } else {
        return conditionResponse(calculator);
      }
    } else if (nb_rx_state != OBD_GETTING_MSG)
      nb_query_state = SEND_COMMAND;  // Error or timeout, so reset the query
                                      // state machine for next command
  }
  return 0.0;
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
      return calculator_0C;

    case MAF_FLOW_RATE:
      return calculator_10;

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
      return calculator_14;

    case RUN_TIME_SINCE_ENGINE_START:
    case DISTANCE_TRAVELED_WITH_MIL_ON:
    case DIST_TRAV_SINCE_CODES_CLEARED:
    case TIME_RUN_WITH_MIL_ON:
    case TIME_SINCE_CODES_CLEARED:
    case ENGINE_REFERENCE_TORQUE:
      return calculator_1F;

    case FUEL_RAIL_PRESSURE:
      return calculator_22;

    case FUEL_RAIL_GUAGE_PRESSURE:
    case FUEL_RAIL_ABS_PRESSURE:
      return calculator_23;

    case EVAP_SYSTEM_VAPOR_PRESSURE:
      return calculator_32;

    case CATALYST_TEMP_BANK_1_SENSOR_1:
    case CATALYST_TEMP_BANK_2_SENSOR_1:
    case CATALYST_TEMP_BANK_1_SENSOR_2:
    case CATALYST_TEMP_BANK_2_SENSOR_2:
      return calculator_3C;

    case CONTROL_MODULE_VOLTAGE:
      return calculator_42;

    case ABS_LOAD_VALUE:
      return calculator_43;

    case FUEL_AIR_COMMANDED_EQUIV_RATIO:
      return calculator_44;

    case MAX_VALUES_EQUIV_V_I_PRESSURE:
      return calculator_4F;

    case MAX_MAF_RATE:
      return calculator_50;

    case ABS_EVAP_SYS_VAPOR_PRESSURE:
      return calculator_53;

    case SHORT_TERM_SEC_OXY_SENS_TRIM_1_3:
    case LONG_TERM_SEC_OXY_SENS_TRIM_1_3:
    case SHORT_TERM_SEC_OXY_SENS_TRIM_2_4:
    case LONG_TERM_SEC_OXY_SENS_TRIM_2_4:
      return calculator_55;

    case FUEL_INJECTION_TIMING:
      return calculator_5D;

    case ENGINE_FUEL_RATE:
      return calculator_5E;

    default:
      return nullptr;
  }
}

/* Determine which of PIDs 0x1 through 0x20 are supported (bit encoded)
Return:
 -------
  * uint32_t - Bit encoded booleans of supported PIDs 0x1-0x20
*/
uint32_t OBD2::supportedPIDs_1_20() {
  return (uint32_t)processPID(SERVICE_01, SUPPORTED_PIDS_1_20, 1, 4);
}

/* Monitor status since DTCs cleared (Includes malfunction indicator
  lamp (MIL) status and number of DTCs). See
 https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_01 for more info
Return:
 -------
  * uint32_t - Bit encoded status
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_01)*/
uint32_t OBD2::monitorStatus() {
  return (uint32_t)processPID(
      SERVICE_01, MONITOR_STATUS_SINCE_DTC_CLEARED, 1, 4);
}

/* Freeze DTC - see https://www.samarins.com/diagnose/freeze-frame.html for
 more info
Return:
 -------
  * uint16_t - Various vehicle information
 (https://www.samarins.com/diagnose/freeze-frame.html)*/
uint16_t OBD2::freezeDTC() {
  return (uint16_t)processPID(SERVICE_01, FREEZE_DTC, 1, 2);
}

/* Freeze DTC - see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_03
 for more info
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
  * float - Engine load %*/
float OBD2::engineLoad() {
  return processPID(SERVICE_01, ENGINE_LOAD, 1, 1, 100.0 / 255.0);
}

/* Find the current engine coolant temp in C
Return:
 -------
  * float - Engine load %*/
float OBD2::engineCoolantTemp() {
  return processPID(SERVICE_01, ENGINE_COOLANT_TEMP, 1, 1, 1, -40.0);
}

/* Find fuel trim %
Return:
 -------
  * float - Fuel trim %*/
float OBD2::shortTermFuelTrimBank_1() {
  return processPID(
      SERVICE_01, SHORT_TERM_FUEL_TRIM_BANK_1, 1, 1, 100.0 / 128.0, -100.0);
}

/* Find fuel trim %
Return:
 -------
  * float - Fuel trim %*/
float OBD2::longTermFuelTrimBank_1() {
  return processPID(
      SERVICE_01, LONG_TERM_FUEL_TRIM_BANK_1, 1, 1, 100.0 / 128.0, -100.0);
}

/* Find fuel trim %
Return:
 -------
  * float - Fuel trim %*/
float OBD2::shortTermFuelTrimBank_2() {
  return processPID(
      SERVICE_01, SHORT_TERM_FUEL_TRIM_BANK_2, 1, 1, 100.0 / 128.0, -100.0);
}

/* Find fuel trim %
Return:
 -------
  * float - Fuel trim %*/
float OBD2::longTermFuelTrimBank_2() {
  return processPID(
      SERVICE_01, LONG_TERM_FUEL_TRIM_BANK_2, 1, 1, 100.0 / 128.0, -100.0);
}

/* Find fuel pressure in kPa
Return:
 -------
  * float - Fuel pressure in kPa
*/
float OBD2::fuelPressure() {
  return processPID(SERVICE_01, FUEL_PRESSURE, 1, 1, 3.0);
}

/* Find intake manifold absolute pressure in kPa
Return:
 -------
  * uint8_t - Intake manifold absolute pressure in kPa
*/
uint8_t OBD2::manifoldPressure() {
  return (uint8_t)processPID(SERVICE_01, INTAKE_MANIFOLD_ABS_PRESSURE, 1, 1);
}

/* Queries and parses received message for/returns vehicle RMP data
Return:
 -------
  * float - Vehicle RPM
*/
float OBD2::rpm() {
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

/*  Queries and parses received message for/returns vehicle speed data (mph)
Return:
 -------
  * float - Vehicle speed in mph
*/
float OBD2::mph() {
  return kph() * KPH_MPH_CONVERT;
}

/*
 float OBD2::timingAdvance() *  Find timing advance in degrees before Top Dead
Center (TDC) Return:
 -------
  * float - Timing advance in degrees before Top Dead Center (TDC)*/
float OBD2::timingAdvance() {
  return processPID(SERVICE_01, TIMING_ADVANCE, 1, 1, 1.0 / 2.0, -64.0);
}

/*  Find intake air temperature in C
Return:
 -------
  * float - Intake air temperature in C
*/
float OBD2::intakeAirTemp() {
  return processPID(SERVICE_01, INTAKE_AIR_TEMP, 1, 1, 1, -40.0);
}

/*  Find mass air flow sensor (MAF) air flow rate rate in g/s
Return:
 -------
  * float - Mass air flow sensor (MAF) air flow rate rate in g/s
*/
float OBD2::mafRate() {
  return processPID(SERVICE_01, MAF_FLOW_RATE, 1, 2, 1.0 / 100.0);
}

/*  Find throttle position in %
Return:
 -------
  * float - Throttle position in %*/
float OBD2::throttle() {
  return processPID(SERVICE_01, THROTTLE_POSITION, 1, 1, 100.0 / 255.0);
}

/*  Find commanded secondary air status
Return:
 -------
  * uint8_t - Bit encoded status
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_12)*/
uint8_t OBD2::commandedSecAirStatus() {
  return (uint8_t)processPID(SERVICE_01, COMMANDED_SECONDARY_AIR_STATUS, 1, 1);
}

/*  Find which oxygen sensors are
present ([A0..A3] == Bank 1, Sensors 1-4. [A4..A7] == Bank 2...) Return:
 -------
  * uint8_t - Bit encoded
*/
uint8_t OBD2::oxygenSensorsPresent_2banks() {
  return (uint8_t)processPID(SERVICE_01, OXYGEN_SENSORS_PRESENT_2_BANKS, 1, 1);
}

/*  Find the OBD standards this vehicle conforms to
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_1C)
Return:
 -------
  * uint8_t - Bit encoded
 (https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_1C)*/
uint8_t OBD2::obdStandards() {
  return (uint8_t)processPID(SERVICE_01, OBD_STANDARDS, 1, 1);
}

/*  Find which oxygen sensors are
present (Similar to PID 13, but [A0..A7] == [B1S1, B1S2, B2S1, B2S2, B3S1, B3S2,
B4S1, B4S2]) Return:
 -------
  * uint8_t - Bit encoded
*/
uint8_t OBD2::oxygenSensorsPresent_4banks() {
  return (uint8_t)processPID(SERVICE_01, OXYGEN_SENSORS_PRESENT_4_BANKS, 1, 1);
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

/* Determine which of PIDs 0x1 through 0x20
are supported (bit encoded) Return:
 -------
  * uint32_t - Bit encoded booleans of supported PIDs 0x21-0x20
*/
uint32_t OBD2::supportedPIDs_21_40() {
  return (uint32_t)processPID(SERVICE_01, SUPPORTED_PIDS_21_40, 1, 4);
}

/*  Find distance traveled with malfunction
indicator lamp (MIL) on in km Return:
 -------
  * uint16_t - Distance traveled with malfunction indicator lamp (MIL) on in km
*/
uint16_t OBD2::distTravelWithMIL() {
  return (uint16_t)processPID(SERVICE_01, DISTANCE_TRAVELED_WITH_MIL_ON, 1, 2);
}

/*  Find fuel Rail Pressure (relative to manifold
vacuum) in kPa Return:
 -------
  * float - Fuel Rail Pressure (relative to manifold vacuum) in kPa
*/
float OBD2::fuelRailPressure() {
  return processPID(SERVICE_01, FUEL_RAIL_PRESSURE, 1, 2, 0.079);
}

/*  Find fuel Rail Gauge Pressure (diesel,
or gasoline direct injection) in kPa Return:
 -------
  * float - Fuel Rail Gauge Pressure (diesel, or gasoline direct injection) in
 kPa
*/
float OBD2::fuelRailGuagePressure() {
  return processPID(SERVICE_01, FUEL_RAIL_GUAGE_PRESSURE, 1, 2, 10.0);
}

/*  Find commanded Exhaust Gas Recirculation (EGR) in
% Return:
 -------
  * float - Commanded Exhaust Gas Recirculation (EGR) in %*/
float OBD2::commandedEGR() {
  return processPID(SERVICE_01, COMMANDED_EGR, 1, 1, 100.0 / 255.0);
}

/*  Find Exhaust Gas Recirculation (EGR) error in %
Return:
 -------
  * float - Exhaust Gas Recirculation (EGR) error in %*/
float OBD2::egrError() {
  return processPID(SERVICE_01, EGR_ERROR, 1, 1, 100.0 / 128.0, -100);
}

/*  Find commanded evaporative purge in %
Return:
 -------
  * float - Commanded evaporative purge in %*/
float OBD2::commandedEvapPurge() {
  return processPID(
      SERVICE_01, COMMANDED_EVAPORATIVE_PURGE, 1, 1, 100.0 / 255.0);
}

/*  Find fuel tank level input in %
Return:
 -------
  * float - Fuel tank level input in %*/
float OBD2::fuelLevel() {
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
  * float - Evap. system vapor pressure in Pa
*/
float OBD2::evapSysVapPressure() {
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

/*  Find catalyst temperature in C
Return:
 -------
  * float - Catalyst temperature in C
*/
float OBD2::catTempB1S1() {
  return processPID(
      SERVICE_01, CATALYST_TEMP_BANK_1_SENSOR_1, 1, 2, 1.0 / 10.0, -40.0);
}

/* Sends a command/query for
 Non-Blocking PID queries

 Inputs:
 -------
  * const char *cmd - Command/query to send to OBD2
*/
void OBD2::sendCommand(const char* cmd) {
  // clear payload buffer
  memset(payload, 0, sizeof(payload));

  // reset input serial buffer and number of received bytes
  recBytes  = 0;
  connected = false;

  // Reset the receive state ready to start receiving a response message
  nb_rx_state = OBD_GETTING_MSG;

  log_print("Sending the following command/query: %s", cmd);

  elm_port->print(cmd);
  elm_port->print('\r');

  iso_tp_->send()

      // prime the timeout timer
      previousTime = millis();
  currentTime      = previousTime;
}

/* Sends a
 command/query and waits for a respoonse (blocking function) Sometimes it's
 desirable to use a blocking command, e.g when sending an AT command. This
 function removes the need for the caller to set up a loop waiting for the
 command to finish. Caller is free to parse the payload string if they need to
 use the response.

 Inputs:
 -------
  * const char *cmd - Command/query to send to OBD2

 Return:
 -------
  * int8_t - the OBD_XXX status of getting the OBD response
*/
int8_t OBD2::sendCommand_Blocking(const char* cmd) {
  sendCommand(cmd);
  uint32_t startTime = millis();
  while (get_response() == OBD_GETTING_MSG) {
    if (millis() - startTime > timeout_ms)
      break;
  }
  return nb_rx_state;
}

/* Non Blocking (NB) receive OBD scanner
response. Must be called repeatedly until the status progresses past
OBD_GETTING_MSG. Return:
 -------
  * int8_t - the OBD_XXX status of getting the OBD response
*/
int8_t OBD2::get_response(void) {
  {
    // buffer the response of the OBD327 until either the
    // end marker is read or a timeout has occurred
    // last valid idx is PAYLOAD_LEN but want to keep one free for terminating
    // '\0' so limit counter to < PAYLOAD_LEN
    if (!elm_port->available()) {
      nb_rx_state = OBD_GETTING_MSG;
      if (timeout())
        nb_rx_state = OBD_TIMEOUT;
    } else {
      char recChar = elm_port->read();

      // display each received character, make non-printables printable
      if (recChar == '\f')
        log_print("\tReceived char: \\f");
      else if (recChar == '\n')
        log_print("\tReceived char: \\n");
      else if (recChar == '\r')
        log_print("\tReceived char: \\r");
      else if (recChar == '\t')
        log_print("\tReceived char: \\t");
      else if (recChar == '\v')
        log_print("\tReceived char: \\v");
      // convert spaces to underscore, easier to see in debug output
      else if (recChar == ' ')
        log_print("\tReceived char: _");
      // display regular printable
      else
        log_print("\tReceived char: %c", recChar);

      // this is the end of the OBD response
      if (recChar == '>') {
        log_print("Delimiter found.");

        nb_rx_state = OBD_MSG_RXD;
      } else if (!isalnum(recChar) && (recChar != ':') && (recChar != '.') &&
                 (recChar != '\r'))
        // Keep only alphanumeric, decimal, colon, CR. These are needed for
        // response parsing decimal places needed to extract floating point
        // numbers, e.g. battery voltage
        nb_rx_state = OBD_GETTING_MSG;  // Discard this character
      else {
        if (recBytes < PAYLOAD_LEN) {
          payload[recBytes] = recChar;
          recBytes++;
          nb_rx_state = OBD_GETTING_MSG;
        } else
          nb_rx_state = OBD_BUFFER_OVERFLOW;
      }
    }

    // Message is still being received (or is timing out), so exit early without
    // doing all the other checks
    if (nb_rx_state == OBD_GETTING_MSG)
      return nb_rx_state;

    // End of response delimiter was found
    if (nb_rx_state == OBD_MSG_RXD) {
      log_print("All chars received: %s", payload);
    }

    if (nb_rx_state == OBD_TIMEOUT) {
      log_print("Timeout detected with overflow of %ld ms",
                (currentTime - previousTime) - timeout_ms);
      return nb_rx_state;
    }

    if (nb_rx_state == OBD_BUFFER_OVERFLOW) {
      log_print("OBD receive buffer overflow (> %d bytes)", PAYLOAD_LEN);
      return nb_rx_state;
    }

    // Now we have successfully received OBD response, check if the payload
    // indicates any OBD errors
    if (nextIndex(payload, RESPONSE_UNABLE_TO_CONNECT) >= 0) {
      log_print("OBD responded with error \"UNABLE TO CONNECT\"");

      nb_rx_state = OBD_UNABLE_TO_CONNECT;
      return nb_rx_state;
    }

    connected = true;

    if (nextIndex(payload, RESPONSE_NO_DATA) >= 0) {
      log_print("OBD responded with error \"NO DATA\"");

      nb_rx_state = OBD_NO_DATA;
      return nb_rx_state;
    }

    if (nextIndex(payload, RESPONSE_STOPPED) >= 0) {
      log_print("OBD responded with error \"STOPPED\"");

      nb_rx_state = OBD_STOPPED;
      return nb_rx_state;
    }

    if (nextIndex(payload, RESPONSE_ERROR) >= 0) {
      log_print("OBD responded with \"ERROR\"");

      nb_rx_state = OBD_GENERAL_ERROR;
      return nb_rx_state;
    }

    nb_rx_state = OBD_SUCCESS;
    // Need to process multiline repsonses, remove '\r' from non multiline resp
    if (NULL != strchr(payload, ':')) {
      parseMultiLineResponse();
    } else {
      removeChar(payload, " \r");
    }
    recBytes = strlen(payload);
    return nb_rx_state;
  }

  /* Parses a buffered multiline response into
   a single line with the specified data
    * Modifies the value of payload for further processing and removes the '\r'
   chars


  */
  void OBD2::parseMultiLineResponse() {
    uint8_t totalBytes    = 0;
    uint8_t bytesReceived = 0;
    char newResponse[PAYLOAD_LEN];
    memset(
        newResponse,
        0,
        PAYLOAD_LEN * sizeof(char));  // Initialize newResponse to empty string
    char line[256] = "";
    char* start    = payload;
    char* end      = strchr(start, '\r');

    do {  // Step 1: Get a line from the response
      memset(line, '\0', 256);
      if (end != NULL) {
        strncpy(line, start, end - start);
        line[end - start] = '\0';
      } else {
        strncpy(line, start, strlen(start));
        line[strlen(start)] = '\0';

        // Exit when there's no more data
        if (strlen(line) == 0)
          break;
      }

      log_print("Found line in response: %s", line);
      // Step 2: Check if this is the first line of the response
      if (0 == totalBytes)
      // Some devices return the response header in the first line instead of
      // the data length, ignore this line Line containing totalBytes indicator
      // is 3 hex chars only, longer first line will be a header.
      {
        if (strlen(line) > 3) {
          log_print("Found header in response line: %s", line);
        } else {
          if (strlen(line) > 0) {
            totalBytes = strtol(line, NULL, 16) * 2;
            log_print("totalBytes = %d", totalBytes);
          }
        }
      }
      // Step 3: Process data response lines
      else {
        if (strchr(line, ':')) {
          char* dataStart     = strchr(line, ':') + 1;
          uint8_t dataLength  = strlen(dataStart);
          uint8_t bytesToCopy = (bytesReceived + dataLength > totalBytes)
                                    ? (totalBytes - bytesReceived)
                                    : dataLength;
          if (bytesReceived + bytesToCopy > PAYLOAD_LEN - 1) {
            bytesToCopy = (PAYLOAD_LEN - 1) - bytesReceived;
          }
          strncat(newResponse, dataStart, bytesToCopy);
          bytesReceived += bytesToCopy;

          log_print("Response data: %s", dataStart);
        }
      }
      if (*(end + 1) == '\0') {
        start = NULL;
      } else {
        start = end + 1;
      }
      end = (start != NULL) ? strchr(start, '\r') : NULL;

    } while ((bytesReceived < totalBytes || 0 == totalBytes) && start != NULL);

    // Replace payload with parsed response, null-terminate after totalBytes
    int nullTermPos =
        (totalBytes < PAYLOAD_LEN - 1) ? totalBytes : PAYLOAD_LEN - 1;
    strncpy(payload, newResponse, nullTermPos);
    payload[nullTermPos] = '\0';  // Ensure null termination
    log_print("Parsed multiline response: %s", payload);
  }

  /*
   *
   Parses the buffered OBD2's response and returns the queried data

   Inputs:
   -------
    * const uint8_t& service - The diagnostic service ID. 01 is "Show current
   data"
    * const uint8_t& pid     - The Parameter ID (PID) from the service
  */
  uint64_t OBD2::findResponse() {
    uint8_t firstDatum = 0;
    char header[7]     = {'\0'};

    if (longQuery) {
      header[0] = query[0] + 4;
      header[1] = query[1];
      header[2] = query[2];
      header[3] = query[3];
      header[4] = query[4];
      header[5] = query[5];
    } else {
      header[0] = query[0] + 4;
      header[1] = query[1];

      if (isMode0x22Query)  // mode 0x22 responses always zero-pad the pid to 4
                            // chars, even for a 2-char pid
      {
        header[2] = '0';
        header[3] = '0';
        header[4] = query[2];
        header[5] = query[3];
      } else {
        header[2] = query[2];
        header[3] = query[3];
      }
    }

    log_print("Expected response header: %s", header);

    int8_t firstHeadIndex  = nextIndex(payload, header, 1);
    int8_t secondHeadIndex = nextIndex(payload, header, 2);

    if (firstHeadIndex >= 0) {
      if (longQuery | isMode0x22Query)
        firstDatum = firstHeadIndex + 6;
      else
        firstDatum = firstHeadIndex + 4;

      // Some OBD327s (such as my own) respond with two
      // "responses" per query. "numPayChars" represents the
      // correct number of bytes returned by the OBD2
      // regardless of how many "responses" were returned
      if (secondHeadIndex >= 0) {
        log_print("Double response detected");

        numPayChars = secondHeadIndex - firstDatum;
      } else {
        log_print("Single response detected");

        numPayChars = strlen(payload) - firstDatum;
      }

      response = 0;
      for (uint8_t i = 0; i < numPayChars; i++) {
        uint8_t payloadIndex = firstDatum + i;
        uint8_t bitsOffset   = 4 * (numPayChars - i - 1);

        log_print("\tProcessing hex nibble: %c", payload[payloadIndex]);
        response =
            response | ((uint64_t)ctoi(payload[payloadIndex]) << bitsOffset);
      }

      // It is useful to have the response bytes
      // broken-out because some PID algorithms (standard
      // and custom) require special operations for each
      // uint8_t returned

      responseByte_0 = response & 0xFF;
      responseByte_1 = (response >> 8) & 0xFF;
      responseByte_2 = (response >> 16) & 0xFF;
      responseByte_3 = (response >> 24) & 0xFF;
      responseByte_4 = (response >> 32) & 0xFF;
      responseByte_5 = (response >> 40) & 0xFF;
      responseByte_6 = (response >> 48) & 0xFF;
      responseByte_7 = (response >> 56) & 0xFF;

      log_print("64-bit response: %llX", response);

      return response;
    }

    log_print("Response not detected");

    return 0;
  }

  /* Prints appropriate error description if an error has
   occurred


  */
  void OBD2::printError() {
    Serial.print(F("Received: "));
    Serial.println(payload);

    if (nb_rx_state == OBD_SUCCESS)
      Serial.println(F("OBD_SUCCESS"));
    else if (nb_rx_state == OBD_NO_RESPONSE)
      Serial.println(F("ERROR: OBD_NO_RESPONSE"));
    else if (nb_rx_state == OBD_BUFFER_OVERFLOW)
      Serial.println(F("ERROR: OBD_BUFFER_OVERFLOW"));
    else if (nb_rx_state == OBD_UNABLE_TO_CONNECT)
      Serial.println(F("ERROR: OBD_UNABLE_TO_CONNECT"));
    else if (nb_rx_state == OBD_NO_DATA)
      Serial.println(F("ERROR: OBD_NO_DATA"));
    else if (nb_rx_state == OBD_STOPPED)
      Serial.println(F("ERROR: OBD_STOPPED"));
    else if (nb_rx_state == OBD_TIMEOUT)
      Serial.println(F("ERROR: OBD_TIMEOUT"));
    else if (nb_rx_state == OBD_BUFFER_OVERFLOW)
      Serial.println(F("ERROR: BUFFER OVERFLOW"));
    else if (nb_rx_state == OBD_GENERAL_ERROR)
      Serial.println(F("ERROR: OBD_GENERAL_ERROR"));
    else
      Serial.println(F("No error detected"));

    delay(100);
  }

  /* Get the current vehicle battery voltage in Volts
  DC Return:
   -------
    * float - vehicle battery voltage in VDC
  */
  float OBD2::batteryVoltage() {
    if (nb_query_state == SEND_COMMAND) {
      sendCommand(READ_VOLTAGE);
      nb_query_state = WAITING_RESP;
    } else if (nb_query_state == WAITING_RESP) {
      get_response();
      if (nb_rx_state == OBD_SUCCESS) {
        nb_query_state =
            SEND_COMMAND;  // Reset the query state machine for next command
        payload[strlen(payload) - 1] =
            '\0';  // Remove the last char ("V") from the payload value

        if (strncmp(payload, "ATRV", 4) == 0)
          return (float)strtod(payload + 4, NULL);
        else
          return (float)strtod(payload, NULL);
      } else if (nb_rx_state != OBD_GETTING_MSG)
        nb_query_state = SEND_COMMAND;  // Error or timeout, so reset the query
                                        // state machine for next command
    }
    return 0.0;
  }

  /* Read Vehicle Identification Number
   (VIN). This is a blocking function.

   Inputs:
   -------
      * char vin[] - pointer to c-string in which to store VIN
          Note: (allocate memory for 18 character c-string in calling function)

   Return:
   -------
    * int8_t - the OBD_XXX status of getting the VIN
  */
  int8_t OBD2::get_vin_blocking(char vin[]) {
    char temp[3] = {0};
    char* idx;
    uint8_t vin_counter = 0;
    uint8_t ascii_val;

    log_print("Getting VIN...");

    sendCommand("0902");  // VIN is command 0902
    while (get_response() == OBD_GETTING_MSG)
      ;

    // strcpy(payload, "0140:4902013144341:475030305235352:42313233343536");
    if (nb_rx_state == OBD_SUCCESS) {
      memset(vin, 0, 18);
      // **** Decoding ****
      if (strstr(payload, "490201")) {
        // OBD scanner provides this multiline response:
        // 014                        ==> 0x14 = 20 bytes following
        // 0: 49 02 01 31 44 34       ==> 49 02 = Header. 01 = 1 VIN number in
        // message. 31, 44, 34 = First 3 VIN digits 1: 47 50 30 30 52 35 35 ==>
        // 47->35 next 7 VIN digits 2: 42 31 32 33 34 35 36    ==> 42->36 next 7
        // VIN digits
        //
        // The resulitng payload buffer is:
        // "0140:4902013144341:475030305235352:42313233343536" ==>
        // VIN="1D4GP00R55B123456" (17-digits)
        idx = strstr(payload, "490201") +
              6;  // Pointer to first ASCII code digit of first VIN digit
        // Loop over each pair of ASCII code digits. 17 VIN digits + 2 skipped
        // line numbers = 19 loops
        for (int i = 0; i < (19 * 2); i += 2) {
          temp[0] = *(idx + i);      // Get first digit of ASCII code
          temp[1] = *(idx + i + 1);  // Get second digit of ASCII code
          // No need to add string termination, temp[3] always == 0

          if (strstr(temp, ":"))
            continue;  // Skip the second "1:" and third "2:" line numbers

          ascii_val = strtol(temp, 0, 16);  // Convert ASCII code to integer
          snprintf(vin + vin_counter++,
                   sizeof(uint8_t),
                   "%c",
                   ascii_val);  // Convert ASCII code integer back to character
                                // Serial.printf("Chars %s, ascii_val=%d[dec]
                                // 0x%02hhx[hex] ==> VIN=%s\n", temp, ascii_val,
                                // ascii_val, vin);
        }
      }
      log_print("VIN: %s", vin);
    } else {
      log_print("No VIN response");
      printError();
    }
    return nb_rx_state;
  }

  /* Resets the stored DTCs in the ECU. This is a blocking
   function. Note: The SAE spec requires that scan tools verify that a reset is
   intended ("Are you sure?") before sending the mode 04 reset command to the
   vehicle. See p.32 of OBD2 datasheet.

   Inputs:
   -------
      * void

   Return:
   -------
    * bool - Indicates the success (or not) of the reset command.*/
  bool OBD2::resetDTC() {
    if (sendCommand_Blocking("04") == OBD_SUCCESS) {
      if (strstr(payload, "44") != NULL) {
        log_print("OBD: DTC successfully reset.");

        return true;
      }
    } else {
      log_print("OBD: Resetting DTC codes failed.");
    }

    return false;
  }

  /* Get the list of current
   DTC codes. This method is blocking by default, but can be run in non-blocking
   mode if desired with optional boolean argument. Typical use involves calling
   the monitorStatus() function first to get the number of DTC current codes
   stored, then calling this function to retrieve those codes. This would  not
   typically be done in NB mode in a loop, but optional NB mode is supported.

    * To check the results of this query, inspect the DTC_Response struct:
   DTC_Response.codesFound will contain the number of codes present and
   DTC_Response.codes is an array of 5 char codes that were retrieved.

   Inputs:
   -------
    * bool isBlocking - optional arg to set (non)blocking mode - defaults to
   true / blocking mode
  */
  void OBD2::currentDTCCodes(const bool& isBlocking) {
    char* idx;
    char codeType      = '\0';
    char codeNumber[5] = {0};
    char temp[6]       = {0};

    if (isBlocking)  // In blocking mode, we loop here until get_response() is
                     // past OBD_GETTING_MSG state
    {
      sendCommand("03");  // Check DTC is always Service 03 with no PID
      while (get_response() == OBD_GETTING_MSG)
        ;
    } else {
      if (nb_query_state == SEND_COMMAND) {
        sendCommand("03");
        nb_query_state = WAITING_RESP;
      }

      else if (nb_query_state == WAITING_RESP)
        get_response();
    }

    if (nb_rx_state == OBD_SUCCESS) {
      nb_query_state =
          SEND_COMMAND;  // Reset the query state machine for next command
      memset(DTC_Response.codes, 0, DTC_CODE_LEN * DTC_MAX_CODES);

      if (strstr(payload, "43") !=
          NULL)  // Successful response to Mode 03 request
      {
        // OBD scanner will provide a response that contains one or more lines
        // indicating the codes present. Each response line will start with "43"
        // indicating it is a response to a Mode 03 request. See p. 31 of OBD2
        // datasheet for details and lookup table of code types.

        uint8_t codesFound =
            strlen(payload) /
            8;  // Each code found returns 8 chars starting with "43"
        idx =
            strstr(payload, "43") +
            4;  // Pointer to first DTC code digit (third char in the response)

        if (codesFound >
            DTC_MAX_CODES)  // I don't think the OBD is capable of returning
        {                   // more than 0xF (16) codes, but just in case...
          codesFound = DTC_MAX_CODES;
          log_print("DTC response truncated at %d codes.", DTC_MAX_CODES);
        }

        DTC_Response.codesFound = codesFound;

        for (int i = 0; i < codesFound; i++) {
          memset(temp, 0, sizeof(temp));
          memset(codeNumber, 0, sizeof(codeNumber));

          codeType      = *idx;        // Get first digit of second uint8_t
          codeNumber[0] = *(idx + 1);  // Get second digit of second uint8_t
          codeNumber[1] = *(idx + 2);  // Get first digit of third uint8_t
          codeNumber[2] = *(idx + 3);  // Get second digit of third uint8_t

          switch (codeType)  // Set the correct type prefix for the code
          {
            case '0':
              strcat(temp, "P0");
              break;

            case '1':
              strcat(temp, "P1");
              break;

            case '2':
              strcat(temp, "P2");
              break;
            case '3':
              strcat(temp, "P3");
              break;

            case '4':
              strcat(temp, "C0");
              break;

            case '5':
              strcat(temp, "C1");
              break;

            case '6':
              strcat(temp, "C2");
              break;

            case '7':
              strcat(temp, "C3");
              break;

            case '8':
              strcat(temp, "B0");
              break;

            case '9':
              strcat(temp, "B1");
              break;

            case 'A':
              strcat(temp, "B2");
              break;

            case 'B':
              strcat(temp, "B3");
              break;

            case 'C':
              strcat(temp, "U0");
              break;

            case 'D':
              strcat(temp, "U1");
              break;

            case 'E':
              strcat(temp, "U2");
              break;

            case 'F':
              strcat(temp, "U3");
              break;

            default:
              break;
          }

          strcat(temp, codeNumber);  // Append the code number to the prefix
          strcpy(DTC_Response.codes[i],
                 temp);   // Add the fully parsed code to the list (array)
          idx = idx + 8;  // reset idx to start of next code

          log_print("OBD: Found code: %s", temp);
        }
      } else {
        log_print("OBD: DTC response received with no valid data.");
      }
      return;
    } else if (nb_rx_state != OBD_GETTING_MSG) {
      nb_query_state = SEND_COMMAND;  // Error or timeout, so reset the query
                                      // state machine for next command

      log_print("OBD: Getting current DTC codes failed.");
      printError();
    }
  }
