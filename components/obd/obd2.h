#pragma once

#include <cstdint>
#include <functional>

#include "esp_err.h"
#include "iso_tp.h"

class OBD2 final {
 public:
  // Static constants
  static const bool OBD_DEBUG = false;

  //-------------------------------------------------------------------------------------//
  // PIDs (https://en.wikipedia.org/wiki/OBD-II_PIDs)
  //-------------------------------------------------------------------------------------//
  static const uint8_t SERVICE_01          = 1;
  static const uint8_t SERVICE_02          = 2;
  static const uint8_t SERVICE_03          = 3;
  static const uint8_t PID_INTERVAL_OFFSET = 0x20;

  static const uint8_t SUPPORTED_PIDS_1_20              = 0;   // 0x00 - bit encoded
  static const uint8_t MONITOR_STATUS_SINCE_DTC_CLEARED = 1;   // 0x01 - bit encoded
  static const uint8_t FREEZE_DTC                       = 2;   // 0x02 -
  static const uint8_t FUEL_SYSTEM_STATUS               = 3;   // 0x03 - bit encoded
  static const uint8_t ENGINE_LOAD                      = 4;   // 0x04 - %
  static const uint8_t ENGINE_COOLANT_TEMP              = 5;   // 0x05 - °C
  static const uint8_t SHORT_TERM_FUEL_TRIM_BANK_1      = 6;   // 0x06 - %
  static const uint8_t LONG_TERM_FUEL_TRIM_BANK_1       = 7;   // 0x07 - %
  static const uint8_t SHORT_TERM_FUEL_TRIM_BANK_2      = 8;   // 0x08 - %
  static const uint8_t LONG_TERM_FUEL_TRIM_BANK_2       = 9;   // 0x09 - %
  static const uint8_t FUEL_PRESSURE                    = 10;  // 0x0A - kPa
  static const uint8_t INTAKE_MANIFOLD_ABS_PRESSURE     = 11;  // 0x0B - kPa
  static const uint8_t ENGINE_RPM                       = 12;  // 0x0C - rpm
  static const uint8_t VEHICLE_SPEED                    = 13;  // 0x0D - km/h
  static const uint8_t TIMING_ADVANCE                   = 14;  // 0x0E - ° before TDC
  static const uint8_t INTAKE_AIR_TEMP                  = 15;  // 0x0F - °C
  static const uint8_t MAF_FLOW_RATE                    = 16;  // 0x10 - g/s
  static const uint8_t THROTTLE_POSITION                = 17;  // 0x11 - %
  static const uint8_t COMMANDED_SECONDARY_AIR_STATUS   = 18;  // 0x12 - bit encoded
  static const uint8_t OXYGEN_SENSORS_PRESENT_2_BANKS   = 19;  // 0x13 - bit encoded
  static const uint8_t OXYGEN_SENSOR_1_A                = 20;  // 0x14 - V %
  static const uint8_t OXYGEN_SENSOR_2_A                = 21;  // 0x15 - V %
  static const uint8_t OXYGEN_SENSOR_3_A                = 22;  // 0x16 - V %
  static const uint8_t OXYGEN_SENSOR_4_A                = 23;  // 0x17 - V %
  static const uint8_t OXYGEN_SENSOR_5_A                = 24;  // 0x18 - V %
  static const uint8_t OXYGEN_SENSOR_6_A                = 25;  // 0x19 - V %
  static const uint8_t OXYGEN_SENSOR_7_A                = 26;  // 0x1A - V %
  static const uint8_t OXYGEN_SENSOR_8_A                = 27;  // 0x1B - V %
  static const uint8_t OBD_STANDARDS                    = 28;  // 0x1C - bit encoded
  static const uint8_t OXYGEN_SENSORS_PRESENT_4_BANKS   = 29;  // 0x1D - bit encoded
  static const uint8_t AUX_INPUT_STATUS                 = 30;  // 0x1E - bit encoded
  static const uint8_t RUN_TIME_SINCE_ENGINE_START      = 31;  // 0x1F - sec

  static const uint8_t SUPPORTED_PIDS_21_40          = 32;  // 0x20 - bit encoded
  static const uint8_t DISTANCE_TRAVELED_WITH_MIL_ON = 33;  // 0x21 - km
  static const uint8_t FUEL_RAIL_PRESSURE            = 34;  // 0x22 - kPa
  static const uint8_t FUEL_RAIL_GUAGE_PRESSURE      = 35;  // 0x23 - kPa
  static const uint8_t OXYGEN_SENSOR_1_B             = 36;  // 0x24 - ratio V
  static const uint8_t OXYGEN_SENSOR_2_B             = 37;  // 0x25 - ratio V
  static const uint8_t OXYGEN_SENSOR_3_B             = 38;  // 0x26 - ratio V
  static const uint8_t OXYGEN_SENSOR_4_B             = 39;  // 0x27 - ratio V
  static const uint8_t OXYGEN_SENSOR_5_B             = 40;  // 0x28 - ratio V
  static const uint8_t OXYGEN_SENSOR_6_B             = 41;  // 0x29 - ratio V
  static const uint8_t OXYGEN_SENSOR_7_B             = 42;  // 0x2A - ratio V
  static const uint8_t OXYGEN_SENSOR_8_B             = 43;  // 0x2B - ratio V
  static const uint8_t COMMANDED_EGR                 = 44;  // 0x2C - %
  static const uint8_t EGR_ERROR                     = 45;  // 0x2D - %
  static const uint8_t COMMANDED_EVAPORATIVE_PURGE   = 46;  // 0x2E - %
  static const uint8_t FUEL_TANK_LEVEL_INPUT         = 47;  // 0x2F - %
  static const uint8_t WARM_UPS_SINCE_CODES_CLEARED  = 48;  // 0x30 - count
  static const uint8_t DIST_TRAV_SINCE_CODES_CLEARED = 49;  // 0x31 - km
  static const uint8_t EVAP_SYSTEM_VAPOR_PRESSURE    = 50;  // 0x32 - Pa
  static const uint8_t ABS_BAROMETRIC_PRESSURE       = 51;  // 0x33 - kPa
  static const uint8_t OXYGEN_SENSOR_1_C             = 52;  // 0x34 - ratio mA
  static const uint8_t OXYGEN_SENSOR_2_C             = 53;  // 0x35 - ratio mA
  static const uint8_t OXYGEN_SENSOR_3_C             = 54;  // 0x36 - ratio mA
  static const uint8_t OXYGEN_SENSOR_4_C             = 55;  // 0x37 - ratio mA
  static const uint8_t OXYGEN_SENSOR_5_C             = 56;  // 0x38 - ratio mA
  static const uint8_t OXYGEN_SENSOR_6_C             = 57;  // 0x39 - ratio mA
  static const uint8_t OXYGEN_SENSOR_7_C             = 58;  // 0x3A - ratio mA
  static const uint8_t OXYGEN_SENSOR_8_C             = 59;  // 0x3B - ratio mA
  static const uint8_t CATALYST_TEMP_BANK_1_SENSOR_1 = 60;  // 0x3C - °C
  static const uint8_t CATALYST_TEMP_BANK_2_SENSOR_1 = 61;  // 0x3D - °C
  static const uint8_t CATALYST_TEMP_BANK_1_SENSOR_2 = 62;  // 0x3E - °C
  static const uint8_t CATALYST_TEMP_BANK_2_SENSOR_2 = 63;  // 0x3F - °C

  static const uint8_t SUPPORTED_PIDS_41_60             = 64;  // 0x40 - bit encoded
  static const uint8_t MONITOR_STATUS_THIS_DRIVE_CYCLE  = 65;  // 0x41 - bit encoded
  static const uint8_t CONTROL_MODULE_VOLTAGE           = 66;  // 0x42 - V
  static const uint8_t ABS_LOAD_VALUE                   = 67;  // 0x43 - %
  static const uint8_t FUEL_AIR_COMMANDED_EQUIV_RATIO   = 68;  // 0x44 - ratio
  static const uint8_t RELATIVE_THROTTLE_POSITION       = 69;  // 0x45 - %
  static const uint8_t AMBIENT_AIR_TEMP                 = 70;  // 0x46 - °C
  static const uint8_t ABS_THROTTLE_POSITION_B          = 71;  // 0x47 - %
  static const uint8_t ABS_THROTTLE_POSITION_C          = 72;  // 0x48 - %
  static const uint8_t ABS_THROTTLE_POSITION_D          = 73;  // 0x49 - %
  static const uint8_t ABS_THROTTLE_POSITION_E          = 74;  // 0x4A - %
  static const uint8_t ABS_THROTTLE_POSITION_F          = 75;  // 0x4B - %
  static const uint8_t COMMANDED_THROTTLE_ACTUATOR      = 76;  // 0x4C - %
  static const uint8_t TIME_RUN_WITH_MIL_ON             = 77;  // 0x4D - min
  static const uint8_t TIME_SINCE_CODES_CLEARED         = 78;  // 0x4E - min
  static const uint8_t MAX_VALUES_EQUIV_V_I_PRESSURE    = 79;  // 0x4F - ratio V mA kPa
  static const uint8_t MAX_MAF_RATE                     = 80;  // 0x50 - g/s
  static const uint8_t FUEL_TYPE                        = 81;  // 0x51 - ref table
  static const uint8_t ETHANOL_FUEL_PERCENT             = 82;  // 0x52 - %
  static const uint8_t ABS_EVAP_SYS_VAPOR_PRESSURE      = 83;  // 0x53 - kPa
  static const uint8_t EVAP_SYS_VAPOR_PRESSURE          = 84;  // 0x54 - Pa
  static const uint8_t SHORT_TERM_SEC_OXY_SENS_TRIM_1_3 = 85;  // 0x55 - %
  static const uint8_t LONG_TERM_SEC_OXY_SENS_TRIM_1_3  = 86;  // 0x56 - %
  static const uint8_t SHORT_TERM_SEC_OXY_SENS_TRIM_2_4 = 87;  // 0x57 - %
  static const uint8_t LONG_TERM_SEC_OXY_SENS_TRIM_2_4  = 88;  // 0x58 - %
  static const uint8_t FUEL_RAIL_ABS_PRESSURE           = 89;  // 0x59 - kPa
  static const uint8_t RELATIVE_ACCELERATOR_PEDAL_POS   = 90;  // 0x5A - %
  static const uint8_t HYBRID_BATTERY_REMAINING_LIFE    = 91;  // 0x5B - %
  static const uint8_t ENGINE_OIL_TEMP                  = 92;  // 0x5C - °C
  static const uint8_t FUEL_INJECTION_TIMING            = 93;  // 0x5D - °
  static const uint8_t ENGINE_FUEL_RATE                 = 94;  // 0x5E - L/h
  static const uint8_t EMISSION_REQUIREMENTS            = 95;  // 0x5F - bit encoded

  static const uint8_t SUPPORTED_PIDS_61_80           = 96;   // 0x60 - bit encoded
  static const uint8_t DEMANDED_ENGINE_PERCENT_TORQUE = 97;   // 0x61 - %
  static const uint8_t ACTUAL_ENGINE_TORQUE           = 98;   // 0x62 - %
  static const uint8_t ENGINE_REFERENCE_TORQUE        = 99;   // 0x63 - Nm
  static const uint8_t ENGINE_PERCENT_TORQUE_DATA     = 100;  // 0x64 - %
  static const uint8_t AUX_INPUT_OUTPUT_SUPPORTED     = 101;  // 0x65 - bit encoded

  //-------------------------------------------------------------------------------------//
  // Class constants
  //-------------------------------------------------------------------------------------//
  static const int8_t QUERY_LEN = 9;

  static const int8_t OBD_GENERAL_ERROR     = -1;
  static const int8_t OBD_SUCCESS           = 0;
  static const int8_t OBD_NO_RESPONSE       = 1;
  static const int8_t OBD_BUFFER_OVERFLOW   = 2;
  static const int8_t OBD_GARBAGE           = 3;
  static const int8_t OBD_UNABLE_TO_CONNECT = 4;
  static const int8_t OBD_NO_DATA           = 5;
  static const int8_t OBD_STOPPED           = 6;
  static const int8_t OBD_TIMEOUT           = 7;
  static const int8_t OBD_GETTING_MSG       = 8;
  static const int8_t OBD_MSG_RXD           = 9;

  static const uint8_t DTC_CODE_LEN  = 6;
  static const uint8_t DTC_MAX_CODES = 16;

  const char* const RESPONSE_OK                = "OK";
  const char* const RESPONSE_UNABLE_TO_CONNECT = "UNABLETOCONNECT";
  const char* const RESPONSE_NO_DATA           = "NODATA";
  const char* const RESPONSE_STOPPED           = "STOPPED";
  const char* const RESPONSE_ERROR             = "ERROR";

  // Non-blocking (NB) command states
  typedef enum {
    SEND_COMMAND,
    WAITING_RESP,
    RESPONSE_RECEIVED,
    DECODED_OK,
    ERROR
  } obd_cmd_states;

  // Pointers to existing response uint8_ts, to be used for new calculators
  // without breaking backward compatability with code that may use the above
  // response uint8_ts.

  OBD2(IIsoTp& driver, uint16_t timeout = 1000);

  uint64_t findResponse();
  void queryPID(uint8_t service, uint16_t pid, uint8_t num_responses = 1);
  double processPID(uint8_t service,
                    uint16_t pid,
                    uint8_t num_responses,
                    uint8_t numExpecteduint8_ts,
                    double scaleFactor = 1,
                    double bias        = 0);
  double conditionResponse(uint8_t numExpecteduint8_ts, double scaleFactor = 1, double bias = 0);
  int8_t get_response();

  // pid
  uint32_t supportedPIDs_1_20();
  uint32_t supportedPIDs_21_40();
  uint32_t supportedPIDs_41_60();
  uint32_t supportedPIDs_61_80();
  bool isPidSupported(uint8_t pid);
  std::function<double()> selectCalculator(uint16_t pid);

  // 1 - 20
  uint32_t monitorStatus();
  uint16_t freezeDTC();
  uint16_t fuelSystemStatus();
  double engineLoad();
  double engineCoolantTemp();
  double shortTermFuelTrimBank_1();
  double longTermFuelTrimBank_1();
  double shortTermFuelTrimBank_2();
  double longTermFuelTrimBank_2();
  double fuelPressure();
  uint8_t manifoldPressure();
  double rpm();
  int32_t kph();
  double mph();
  double timingAdvance();
  double intakeAirTemp();
  double mafRate();
  double throttle();
  uint8_t commandedSecAirStatus();
  uint8_t oxygenSensorsPresent_2banks();
  uint8_t obdStandards();
  uint8_t oxygenSensorsPresent_4banks();
  bool auxInputStatus();
  uint16_t runTime();

  // 21 - 40
  uint16_t distTravelWithMIL();
  double fuelRailPressure();
  double fuelRailGuagePressure();
  double commandedEGR();
  double egrError();
  double commandedEvapPurge();
  double fuelLevel();
  uint8_t warmUpsSinceCodesCleared();
  uint16_t distSinceCodesCleared();
  double evapSysVapPressure();
  uint8_t absBaroPressure();
  double catTempB1S1();
  double catTempB2S1();
  double catTempB1S2();
  double catTempB2S2();

  // 41 - 60
  uint32_t monitorDriveCycleStatus();
  double ctrlModVoltage();
  double absLoad();
  double commandedAirFuelRatio();
  double relativeThrottle();
  double ambientAirTemp();
  double absThrottlePosB();
  double absThrottlePosC();
  double absThrottlePosD();
  double absThrottlePosE();
  double absThrottlePosF();
  double commandedThrottleActuator();
  uint16_t timeRunWithMIL();
  uint16_t timeSinceCodesCleared();
  double maxMafRate();
  uint8_t fuelType();
  double ethanolPercent();
  double absEvapSysVapPressure();
  double evapSysVapPressure2();
  double absFuelRailPressure();
  double relativePedalPos();
  double hybridBatLife();
  double oilTemp();
  double fuelInjectTiming();
  double fuelRate();
  uint8_t emissionRqmts();

  // 61 - 80
  double demandedTorque();
  double torque();
  uint16_t referenceTorque();
  uint16_t auxSupported();

 private:
  struct dtcResponse {
    uint8_t codesFound = 0;
    char codes[DTC_MAX_CODES][DTC_CODE_LEN];
  } DTC_Response;

  void sendCommand(IsoTp::Message& cmd);
  void formatQueryArray(uint8_t service, uint16_t pid, uint8_t num_responses);

  uint8_t ctoi(uint8_t value);
  int8_t nextIndex(char const* str, char const* target, uint8_t numOccur = 1);
  void removeChar(char* from, const char* remove);

  void printError();
  void log_print(const char* format, ...);
  void log_print_buffer(uint32_t id, uint8_t* buffer, uint16_t len);
  bool timeout();

  IIsoTp& iso_tp_;

  char payload[128];  // Буфер для приема данных
  int8_t nb_rx_state = OBD_GETTING_MSG;
  uint64_t response  = 0;
  uint16_t recuint8_ts;
  uint8_t numPayChars;
  uint16_t timeout_ms;

  uint8_t response_A;
  uint8_t response_B;
  uint8_t response_C;
  uint8_t response_D;
  uint8_t response_E;
  uint8_t response_F;
  uint8_t response_G;
  uint8_t response_H;

  char query[QUERY_LEN] = {'\0'};
  bool longQuery        = false;
  bool isMode0x22Query  = false;
  uint32_t currentTime;
  uint32_t previousTime;
  obd_cmd_states nb_query_state = SEND_COMMAND;  // Non-blocking query state
};
