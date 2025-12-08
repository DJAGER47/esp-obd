#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include "esp_err.h"
#include "iso_tp.h"

class OBD2 final {
 public:
  static const bool OBD_DEBUG = false;

  OBD2(IIsoTp& driver, uint16_t tx_id = 0x7DF, uint16_t rx_id = 0x7E8);

  // pid
  bool isPidSupported(uint8_t pid);

  // 1 - 20
  std::optional<uint32_t> supportedPIDs_1_20();
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
  std::optional<float> oxygenSensor1Voltage();
  std::optional<float> oxygenSensor1FuelTrim();
  std::optional<float> oxygenSensor2Voltage();
  std::optional<float> oxygenSensor2FuelTrim();
  std::optional<float> oxygenSensor3Voltage();
  std::optional<float> oxygenSensor3FuelTrim();
  std::optional<float> oxygenSensor4Voltage();
  std::optional<float> oxygenSensor4FuelTrim();
  std::optional<float> oxygenSensor5Voltage();
  std::optional<float> oxygenSensor5FuelTrim();
  std::optional<float> oxygenSensor6Voltage();
  std::optional<float> oxygenSensor6FuelTrim();
  std::optional<float> oxygenSensor7Voltage();
  std::optional<float> oxygenSensor7FuelTrim();
  std::optional<float> oxygenSensor8Voltage();
  std::optional<float> oxygenSensor8FuelTrim();
  std::optional<uint8_t> obdStandards();
  std::optional<uint8_t> oxygenSensorsPresent_4banks();
  std::optional<bool> auxInputStatus();
  std::optional<uint16_t> runTime();

  // 21 - 40
  std::optional<uint32_t> supportedPIDs_21_40();
  std::optional<uint16_t> distTravelWithMIL();
  std::optional<float> fuelRailPressure();
  std::optional<uint32_t> fuelRailGuagePressure();
  std::optional<float> oxygenSensor1Lambda();
  std::optional<float> oxygenSensor1VoltageWide();
  std::optional<float> oxygenSensor2Lambda();
  std::optional<float> oxygenSensor2VoltageWide();
  std::optional<float> oxygenSensor3Lambda();
  std::optional<float> oxygenSensor3VoltageWide();
  std::optional<float> oxygenSensor4Lambda();
  std::optional<float> oxygenSensor4VoltageWide();
  std::optional<float> oxygenSensor5Lambda();
  std::optional<float> oxygenSensor5VoltageWide();
  std::optional<float> oxygenSensor6Lambda();
  std::optional<float> oxygenSensor6VoltageWide();
  std::optional<float> oxygenSensor7Lambda();
  std::optional<float> oxygenSensor7VoltageWide();
  std::optional<float> oxygenSensor8Lambda();
  std::optional<float> oxygenSensor8VoltageWide();
  std::optional<float> commandedEGR();
  std::optional<float> egrError();
  std::optional<float> commandedEvapPurge();
  std::optional<float> fuelLevel();
  std::optional<uint8_t> warmUpsSinceCodesCleared();
  std::optional<uint16_t> distSinceCodesCleared();
  std::optional<float> evapSysVapPressure();
  std::optional<uint8_t> absBaroPressure();
  std::optional<float> oxygenSensor1Current();
  std::optional<float> oxygenSensor2Current();
  std::optional<float> oxygenSensor3Current();
  std::optional<float> oxygenSensor4Current();
  std::optional<float> oxygenSensor5Current();
  std::optional<float> oxygenSensor6Current();
  std::optional<float> oxygenSensor7Current();
  std::optional<float> oxygenSensor8Current();
  std::optional<float> catTempB1S1();
  std::optional<float> catTempB2S1();
  std::optional<float> catTempB1S2();
  std::optional<float> catTempB2S2();

  // 41 - 60
  std::optional<uint32_t> supportedPIDs_41_60();
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
  std::optional<uint8_t> maxEquivalenceRatio();
  std::optional<uint8_t> maxOxygenSensorVoltage();
  std::optional<uint8_t> maxOxygenSensorCurrent();
  std::optional<uint16_t> maxManifoldPressure();
  std::optional<uint16_t> maxMafRate();
  std::optional<uint8_t> fuelType();
  std::optional<float> ethanolPercent();
  std::optional<float> absEvapSysVapPressure();
  std::optional<int32_t> evapSysVapPressure2();
  std::optional<float> shortTermSecOxyTrim13();
  std::optional<float> longTermSecOxyTrim13();
  std::optional<float> shortTermSecOxyTrim24();
  std::optional<float> longTermSecOxyTrim24();
  std::optional<uint32_t> absFuelRailPressure();
  std::optional<float> relativePedalPos();
  std::optional<float> hybridBatLife();
  std::optional<int16_t> oilTemp();
  std::optional<float> fuelInjectTiming();
  std::optional<float> fuelRate();
  std::optional<uint8_t> emissionRqmts();

  // 61 - 80
  std::optional<uint32_t> supportedPIDs_61_80();
  std::optional<int16_t> demandedTorque();
  std::optional<int16_t> torque();
  std::optional<uint16_t> referenceTorque();
  std::optional<std::array<int16_t, 5>> enginePercentTorqueData();
  std::optional<uint16_t> auxSupported();

  std::optional<uint32_t> supportedPIDs81_100();
  std::optional<uint32_t> supportedPIDs101_120();
  std::optional<uint32_t> supportedPIDs121_140();

  // Service 09 - Request vehicle information
  std::optional<uint32_t> supportedPIDs_Service09();
  std::optional<uint8_t> vinMessageCount();
  bool getVIN(char* vin_buffer, size_t buffer_size);

  std::optional<uint8_t> calibrationIdMessageCount();
  bool getCalibrationId(char* calib_buffer, size_t buffer_size);

  std::optional<uint8_t> cvnMessageCount();
  bool getCalibrationVerificationNumbers(uint32_t* cvn_buffer, size_t buffer_size, size_t* count);

  std::optional<uint8_t> performanceTrackingMessageCount();
  bool getPerformanceTrackingSparkIgnition(uint16_t* tracking_buffer, size_t buffer_size, size_t* count);

  std::optional<uint8_t> ecuNameMessageCount();
  bool getEcuName(char* ecu_buffer, size_t buffer_size);

  bool getPerformanceTrackingCompressionIgnition(uint16_t* tracking_buffer, size_t buffer_size, size_t* count);

 private:
  using ResponseType = std::array<uint8_t, 8>;

  // Вспомогательный метод для получения поддерживаемых PID
  std::optional<uint32_t> getSupportedPIDs(uint8_t pid);

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
  static const uint8_t SERVICE_09 = 9;  // 09	Request vehicle information
  // 0A	Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)

  // UDS >= 0x10

  static const uint8_t PID_INTERVAL_OFFSET = 0x20;

  // Full set of PIDs
  static const uint8_t SUPPORTED_PIDS_1_20              = 0x00;  // - bit encoded
  static const uint8_t MONITOR_STATUS_SINCE_DTC_CLEARED = 0x01;  // - bit encoded
  static const uint8_t FREEZE_DTC                       = 0x02;  // -
  static const uint8_t FUEL_SYSTEM_STATUS               = 0x03;  // - bit encoded
  static const uint8_t ENGINE_LOAD                      = 0x04;  // - %
  static const uint8_t ENGINE_COOLANT_TEMP              = 0x05;  // - °C
  static const uint8_t SHORT_TERM_FUEL_TRIM_BANK_1      = 0x06;  // - %
  static const uint8_t LONG_TERM_FUEL_TRIM_BANK_1       = 0x07;  // - %
  static const uint8_t SHORT_TERM_FUEL_TRIM_BANK_2      = 0x08;  // - %
  static const uint8_t LONG_TERM_FUEL_TRIM_BANK_2       = 0x09;  // - %
  static const uint8_t FUEL_PRESSURE                    = 0x0A;  // - kPa
  static const uint8_t INTAKE_MANIFOLD_ABS_PRESSURE     = 0x0B;  // - kPa
  static const uint8_t ENGINE_RPM                       = 0x0C;  // - rpm
  static const uint8_t VEHICLE_SPEED                    = 0x0D;  // - km/h
  static const uint8_t TIMING_ADVANCE                   = 0x0E;  // - ° before TDC
  static const uint8_t INTAKE_AIR_TEMP                  = 0x0F;  // - °C
  static const uint8_t MAF_FLOW_RATE                    = 0x10;  // - g/s
  static const uint8_t THROTTLE_POSITION                = 0x11;  // - %
  static const uint8_t COMMANDED_SECONDARY_AIR_STATUS   = 0x12;  // - bit encoded
  static const uint8_t OXYGEN_SENSORS_PRESENT_2_BANKS   = 0x13;  // - bit encoded
  static const uint8_t OXYGEN_SENSOR_1_A                = 0x14;  // - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_2_A                = 0x15;  // - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_3_A                = 0x16;  // - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_4_A                = 0x17;  // - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_5_A                = 0x18;  // - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_6_A                = 0x19;  // - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_7_A                = 0x1A;  // - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_8_A                = 0x1B;  // - V % ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OBD_STANDARDS                    = 0x1C;  // - bit encoded
  static const uint8_t OXYGEN_SENSORS_PRESENT_4_BANKS   = 0x1D;  // - bit encoded
  static const uint8_t AUX_INPUT_STATUS                 = 0x1E;  // - bit encoded
  static const uint8_t RUN_TIME_SINCE_ENGINE_START      = 0x1F;  // - sec

  // Full set of PIDs
  static const uint8_t SUPPORTED_PIDS_21_40          = 0x20;  // - bit encoded
  static const uint8_t DISTANCE_TRAVELED_WITH_MIL_ON = 0x21;  // - km
  static const uint8_t FUEL_RAIL_PRESSURE            = 0x22;  // - kPa
  static const uint8_t FUEL_RAIL_GUAGE_PRESSURE      = 0x23;  // - kPa
  static const uint8_t OXYGEN_SENSOR_1_B             = 0x24;  // - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_2_B             = 0x25;  // - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_3_B             = 0x26;  // - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_4_B             = 0x27;  // - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_5_B             = 0x28;  // - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_6_B             = 0x29;  // - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_7_B             = 0x2A;  // - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_8_B             = 0x2B;  // - ratio V ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t COMMANDED_EGR                 = 0x2C;  // - %
  static const uint8_t EGR_ERROR                     = 0x2D;  // - %
  static const uint8_t COMMANDED_EVAPORATIVE_PURGE   = 0x2E;  // - %
  static const uint8_t FUEL_TANK_LEVEL_INPUT         = 0x2F;  // - %
  static const uint8_t WARM_UPS_SINCE_CODES_CLEARED  = 0x30;  // - count
  static const uint8_t DIST_TRAV_SINCE_CODES_CLEARED = 0x31;  // - km
  static const uint8_t EVAP_SYSTEM_VAPOR_PRESSURE    = 0x32;  // - Pa
  static const uint8_t ABS_BAROMETRIC_PRESSURE       = 0x33;  // - kPa
  static const uint8_t OXYGEN_SENSOR_1_C             = 0x34;  // - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_2_C             = 0x35;  // - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_3_C             = 0x36;  // - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_4_C             = 0x37;  // - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_5_C             = 0x38;  // - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_6_C             = 0x39;  // - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_7_C             = 0x3A;  // - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t OXYGEN_SENSOR_8_C             = 0x3B;  // - ratio mA ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t CATALYST_TEMP_BANK_1_SENSOR_1 = 0x3C;  // - °C
  static const uint8_t CATALYST_TEMP_BANK_2_SENSOR_1 = 0x3D;  // - °C
  static const uint8_t CATALYST_TEMP_BANK_1_SENSOR_2 = 0x3E;  // - °C
  static const uint8_t CATALYST_TEMP_BANK_2_SENSOR_2 = 0x3F;  // - °C

  // Full set of PIDs
  static const uint8_t SUPPORTED_PIDS_41_60             = 0x40;  // - bit encoded
  static const uint8_t MONITOR_STATUS_THIS_DRIVE_CYCLE  = 0x41;  // - bit encoded
  static const uint8_t CONTROL_MODULE_VOLTAGE           = 0x42;  // - V
  static const uint8_t ABS_LOAD_VALUE                   = 0x43;  // - %
  static const uint8_t FUEL_AIR_COMMANDED_EQUIV_RATIO   = 0x44;  // - ratio
  static const uint8_t RELATIVE_THROTTLE_POSITION       = 0x45;  // - %
  static const uint8_t AMBIENT_AIR_TEMP                 = 0x46;  // - °C
  static const uint8_t ABS_THROTTLE_POSITION_B          = 0x47;  // - %
  static const uint8_t ABS_THROTTLE_POSITION_C          = 0x48;  // - %
  static const uint8_t ABS_THROTTLE_POSITION_D          = 0x49;  // - %
  static const uint8_t ABS_THROTTLE_POSITION_E          = 0x4A;  // - %
  static const uint8_t ABS_THROTTLE_POSITION_F          = 0x4B;  // - %
  static const uint8_t COMMANDED_THROTTLE_ACTUATOR      = 0x4C;  // - %
  static const uint8_t TIME_RUN_WITH_MIL_ON             = 0x4D;  // - min
  static const uint8_t TIME_SINCE_CODES_CLEARED         = 0x4E;  // - min
  static const uint8_t MAX_VALUES_EQUIV_V_I_PRESSURE    = 0x4F;  // - ratio V mA kPa ❌ НЕ РЕАЛИЗОВАН
  static const uint8_t MAX_MAF_RATE                     = 0x50;  // - g/s
  static const uint8_t FUEL_TYPE                        = 0x51;  // - ref table
  static const uint8_t ETHANOL_FUEL_PERCENT             = 0x52;  // - %
  static const uint8_t ABS_EVAP_SYS_VAPOR_PRESSURE      = 0x53;  // - kPa
  static const uint8_t EVAP_SYS_VAPOR_PRESSURE          = 0x54;  // - Pa
  static const uint8_t SHORT_TERM_SEC_OXY_SENS_TRIM_1_3 = 0x55;  // - %
  static const uint8_t LONG_TERM_SEC_OXY_SENS_TRIM_1_3  = 0x56;  // - %
  static const uint8_t SHORT_TERM_SEC_OXY_SENS_TRIM_2_4 = 0x57;  // - %
  static const uint8_t LONG_TERM_SEC_OXY_SENS_TRIM_2_4  = 0x58;  // - %
  static const uint8_t FUEL_RAIL_ABS_PRESSURE           = 0x59;  // - kPa
  static const uint8_t RELATIVE_ACCELERATOR_PEDAL_POS   = 0x5A;  // - %
  static const uint8_t HYBRID_BATTERY_REMAINING_LIFE    = 0x5B;  // - %
  static const uint8_t ENGINE_OIL_TEMP                  = 0x5C;  // - °C
  static const uint8_t FUEL_INJECTION_TIMING            = 0x5D;  // - °
  static const uint8_t ENGINE_FUEL_RATE                 = 0x5E;  // - L/h
  static const uint8_t EMISSION_REQUIREMENTS            = 0x5F;  // - bit encoded

  static const uint8_t SUPPORTED_PIDS_61_80           = 0x60;  // - bit encoded
  static const uint8_t DEMANDED_ENGINE_PERCENT_TORQUE = 0x61;  // - %
  static const uint8_t ACTUAL_ENGINE_TORQUE           = 0x62;  // - %
  static const uint8_t ENGINE_REFERENCE_TORQUE        = 0x63;  // - Nm
  static const uint8_t ENGINE_PERCENT_TORQUE_DATA     = 0x64;  // - %
  static const uint8_t AUX_INPUT_OUTPUT_SUPPORTED     = 0x65;  // - bit encoded

  static const uint8_t SUPPORTED_PIDS_81_100  = 0x80;  // - bit encoded
  static const uint8_t SUPPORTED_PIDS_101_120 = 0xA0;  // - bit encoded
  static const uint8_t SUPPORTED_PIDS_121_140 = 0xC0;  // - bit encoded

  // Service 09 - Request vehicle information
  static const uint8_t SERVICE_09_SUPPORTED_PIDS_01_20             = 0x00;  // - bit encoded
  static const uint8_t SERVICE_09_VIN_MESSAGE_COUNT                = 0x01;  // - count
  static const uint8_t SERVICE_09_VIN                              = 0x02;  // - 17-char ASCII
  static const uint8_t SERVICE_09_CALIB_ID_MESSAGE_COUNT           = 0x03;  // - count
  static const uint8_t SERVICE_09_CALIBRATION_ID                   = 0x04;  // - 16-char ASCII
  static const uint8_t SERVICE_09_CVN_MESSAGE_COUNT                = 0x05;  // - count
  static const uint8_t SERVICE_09_CALIBRATION_VERIFICATION_NUMBERS = 0x06;  // - 4-byte hex
  static const uint8_t SERVICE_09_PERF_TRACK_MESSAGE_COUNT         = 0x07;  // - count
  static const uint8_t SERVICE_09_PERF_TRACK_SPARK_IGNITION        = 0x08;  // - 4-byte values
  static const uint8_t SERVICE_09_ECU_NAME_MESSAGE_COUNT           = 0x09;  // - count
  static const uint8_t SERVICE_09_ECU_NAME                         = 0x0A;  // - 20-char ASCII
  static const uint8_t SERVICE_09_PERF_TRACK_COMPRESSION_IGNITION  = 0x0B;  // - 4-byte values

  void queryPID(uint8_t service, uint8_t pid);
  bool processPID(uint8_t service, uint16_t pid, ResponseType& response);

  void log_print(const char* format, ...);
  void log_print_buffer(uint32_t id, uint8_t* buffer, uint16_t len);

  const uint16_t tx_id_;
  const uint16_t rx_id_;
  IIsoTp& iso_tp_;
};
