#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include "esp_err.h"
#include "iso_tp.h"

class OBD2 final {
 public:
  OBD2(IIsoTp& driver, uint16_t tx_id = 0x7DF, uint16_t rx_id = 0x7E8);

  // pid
  std::optional<uint32_t> supportedPIDs_1_20();
  std::optional<uint32_t> supportedPIDs_21_40();
  std::optional<uint32_t> supportedPIDs_41_60();
  std::optional<uint32_t> supportedPIDs_61_80();
  bool isPidSupported(uint8_t pid);

  // 1 - 20
  std::optional<uint32_t> monitorStatus();
  std::optional<uint16_t> freezeDTC();
  std::optional<uint16_t> fuelSystemStatus();
  std::optional<float> engineLoad();
  std::optional<int16_t> engineCoolantTemp();
  std::optional<float> shortTermFuelTrimBank_1();
  std::optional<float> longTermFuelTrimBank_1();
  std::optional<float> shortTermFuelTrimBank_2();
  std::optional<float> longTermFuelTrimBank_2();
  std::optional<uint16_t> fuelPressure();
  std::optional<uint8_t> manifoldPressure();
  std::optional<float> rpm();
  std::optional<uint8_t> kph();
  std::optional<float> timingAdvance();
  std::optional<int16_t> intakeAirTemp();
  std::optional<float> mafRate();
  std::optional<float> throttle();
  std::optional<uint8_t> commandedSecAirStatus();
  std::optional<uint8_t> oxygenSensorsPresent_2banks();
  std::optional<uint8_t> obdStandards();
  std::optional<uint8_t> oxygenSensorsPresent_4banks();
  std::optional<bool> auxInputStatus();
  std::optional<uint16_t> runTime();

  // 21 - 40
  std::optional<uint16_t> distTravelWithMIL();
  std::optional<float> fuelRailPressure();
  std::optional<uint32_t> fuelRailGuagePressure();
  std::optional<float> commandedEGR();
  std::optional<float> egrError();
  std::optional<float> commandedEvapPurge();
  std::optional<float> fuelLevel();
  std::optional<uint8_t> warmUpsSinceCodesCleared();
  std::optional<uint16_t> distSinceCodesCleared();
  std::optional<float> evapSysVapPressure();
  std::optional<uint8_t> absBaroPressure();
  std::optional<float> catTempB1S1();
  std::optional<float> catTempB2S1();
  std::optional<float> catTempB1S2();
  std::optional<float> catTempB2S2();

  // 41 - 60
  std::optional<uint32_t> monitorDriveCycleStatus();
  std::optional<float> ctrlModVoltage();
  std::optional<float> absLoad();
  std::optional<float> commandedAirFuelRatio();
  std::optional<float> relativeThrottle();
  std::optional<int16_t> ambientAirTemp();
  std::optional<float> absThrottlePosB();
  std::optional<float> absThrottlePosC();
  std::optional<float> absThrottlePosD();
  std::optional<float> absThrottlePosE();
  std::optional<float> absThrottlePosF();
  std::optional<float> commandedThrottleActuator();
  std::optional<uint16_t> timeRunWithMIL();
  std::optional<uint16_t> timeSinceCodesCleared();
  std::optional<uint16_t> maxMafRate();
  std::optional<uint8_t> fuelType();
  std::optional<float> ethanolPercent();
  std::optional<float> absEvapSysVapPressure();
  std::optional<int32_t> evapSysVapPressure2();
  std::optional<uint32_t> absFuelRailPressure();
  std::optional<float> relativePedalPos();
  std::optional<float> hybridBatLife();
  std::optional<int16_t> oilTemp();
  std::optional<float> fuelInjectTiming();
  std::optional<float> fuelRate();
  std::optional<uint8_t> emissionRqmts();

  // 61 - 80
  std::optional<int16_t> demandedTorque();
  std::optional<int16_t> torque();
  std::optional<uint16_t> referenceTorque();
  std::optional<uint16_t> auxSupported();

 private:
  using ResponseType = std::array<uint8_t, 8>;

  static const bool OBD_DEBUG = true;

  static constexpr size_t A = 0;
  static constexpr size_t B = 1;
  static constexpr size_t C = 2;
  static constexpr size_t D = 3;
  static constexpr size_t E = 4;
  static constexpr size_t F = 5;
  static constexpr size_t G = 6;
  static constexpr size_t H = 7;

  //-------------------------------------------------------------------------------------//
  // PIDs (https://en.wikipedia.org/wiki/OBD-II_PIDs)
  //-------------------------------------------------------------------------------------//
  static const uint8_t SERVICE_01 = 1;  // Show current data
  static const uint8_t SERVICE_02 = 2;  // Show freeze frame data
  static const uint8_t SERVICE_03 = 3;  // Show stored Diagnostic Trouble Codes
  // 04	Clear Diagnostic Trouble Codes and stored values
  // 05	Test results, oxygen sensor monitoring (non CAN only)
  // 06	Test results, other component/system monitoring (Test results, oxygen sensor monitoring for
  // CAN only)
  // 07	Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)
  // 08	Control operation of on-board component/system
  // 09	Request vehicle information
  // 0A	Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)

  // UDS >= 0x10

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
  static const uint8_t OXYGEN_SENSOR_1_A = 20;  // 0x14 - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_2_A = 21;  // 0x15 - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_3_A = 22;  // 0x16 - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_4_A = 23;  // 0x17 - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_5_A = 24;  // 0x18 - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_6_A = 25;  // 0x19 - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_7_A = 26;  // 0x1A - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_8_A = 27;  // 0x1B - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OBD_STANDARDS     = 28;  // 0x1C - bit encoded
  static const uint8_t OXYGEN_SENSORS_PRESENT_4_BANKS = 29;  // 0x1D - bit encoded
  static const uint8_t AUX_INPUT_STATUS               = 30;  // 0x1E - bit encoded
  static const uint8_t RUN_TIME_SINCE_ENGINE_START    = 31;  // 0x1F - sec

  static const uint8_t SUPPORTED_PIDS_21_40          = 32;  // 0x20 - bit encoded
  static const uint8_t DISTANCE_TRAVELED_WITH_MIL_ON = 33;  // 0x21 - km
  static const uint8_t FUEL_RAIL_PRESSURE            = 34;  // 0x22 - kPa
  static const uint8_t FUEL_RAIL_GUAGE_PRESSURE      = 35;  // 0x23 - kPa
  static const uint8_t OXYGEN_SENSOR_1_B = 36;  // 0x24 - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_2_B = 37;  // 0x25 - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_3_B = 38;  // 0x26 - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_4_B = 39;  // 0x27 - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_5_B = 40;  // 0x28 - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_6_B = 41;  // 0x29 - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_7_B = 42;  // 0x2A - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_8_B = 43;  // 0x2B - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t COMMANDED_EGR     = 44;  // 0x2C - %
  static const uint8_t EGR_ERROR         = 45;  // 0x2D - %
  static const uint8_t COMMANDED_EVAPORATIVE_PURGE   = 46;  // 0x2E - %
  static const uint8_t FUEL_TANK_LEVEL_INPUT         = 47;  // 0x2F - %
  static const uint8_t WARM_UPS_SINCE_CODES_CLEARED  = 48;  // 0x30 - count
  static const uint8_t DIST_TRAV_SINCE_CODES_CLEARED = 49;  // 0x31 - km
  static const uint8_t EVAP_SYSTEM_VAPOR_PRESSURE    = 50;  // 0x32 - Pa
  static const uint8_t ABS_BAROMETRIC_PRESSURE       = 51;  // 0x33 - kPa
  static const uint8_t OXYGEN_SENSOR_1_C = 52;  // 0x34 - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_2_C = 53;  // 0x35 - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_3_C = 54;  // 0x36 - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_4_C = 55;  // 0x37 - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_5_C = 56;  // 0x38 - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_6_C = 57;  // 0x39 - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_7_C = 58;  // 0x3A - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_8_C = 59;  // 0x3B - ratio mA ❌ НЕ РЕАЛИЗОВАН
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

  static const uint8_t SUPPORTED_PIDS_61_80           = 96;  // 0x60 - bit encoded
  static const uint8_t DEMANDED_ENGINE_PERCENT_TORQUE = 97;  // 0x61 - %
  static const uint8_t ACTUAL_ENGINE_TORQUE           = 98;  // 0x62 - %
  static const uint8_t ENGINE_REFERENCE_TORQUE        = 99;  // 0x63 - Nm
  static const uint8_t ENGINE_PERCENT_TORQUE_DATA = 100;  // 0x64 - % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t AUX_INPUT_OUTPUT_SUPPORTED = 101;  // 0x65 - bit encoded

  void queryPID(uint8_t service, uint8_t pid);
  bool processPID(uint8_t service, uint16_t pid, ResponseType& response);

  void log_print(const char* format, ...);
  void log_print_buffer(uint32_t id, uint8_t* buffer, uint16_t len);

  const uint16_t tx_id_;
  const uint16_t rx_id_;
  IIsoTp& iso_tp_;
};
