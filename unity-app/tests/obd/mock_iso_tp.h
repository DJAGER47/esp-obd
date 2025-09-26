#pragma once

#include <cstring>
#include <queue>
#include <vector>

#include "iso_tp_interface.h"

static const uint8_t SERVICE_01 = 1;  // Show current data

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

// Мок-класс для IsoTp для тестирования OBD2 протокола
// Наследуется от IIsoTp и полностью мокает поведение IsoTp
class MockIsoTp : public IIsoTp {
 public:
  MockIsoTp() {
    reset();
  }

  // Переопределенные виртуальные методы из IIsoTp
  bool send(Message& message) override {
    send_called       = true;
    last_sent_message = message;
    sent_messages.push_back(message);
    return send_result;
  }

  bool receive(Message& message, size_t size_buffer) override {
    receive_called = true;
    if (!receive_messages.empty()) {
      message = receive_messages.front();
      receive_messages.pop();
      return receive_result;
    }
    return false;  // Нет сообщений для получения
  }

  // Методы для управления состоянием мока
  void reset() {
    sent_messages.clear();
    while (!receive_messages.empty()) {
      receive_messages.pop();
    }
    send_called    = false;
    receive_called = false;
    send_result    = true;
    receive_result = false;  // false означает успех для receive
  }

  void add_receive_message(const Message& message) {
    receive_messages.push(message);
  }

  void set_send_result(bool result) {
    send_result = result;
  }

  void set_receive_result(bool result) {
    receive_result = result;
  }

  // Публичные поля для проверки в тестах
  std::vector<Message> sent_messages;
  std::queue<Message> receive_messages;
  Message last_sent_message;
  bool send_called    = false;
  bool receive_called = false;
  bool send_result    = true;
  bool receive_result = false;
};

// Вспомогательные функции для создания OBD2 сообщений

// Создание OBD2 ответа для одного байта данных
inline IIsoTp::Message create_obd_response_1_byte(uint32_t rx_id,
                                                  uint8_t service,
                                                  uint8_t pid,
                                                  uint8_t data) {
  IIsoTp::Message msg;
  msg.tx_id   = 0x7DF;  // Стандартный OBD2 запрос ID
  msg.rx_id   = rx_id;
  msg.len     = 4;
  msg.data    = new uint8_t[4];
  msg.data[0] = service + 0x40;  // Положительный ответ
  msg.data[1] = pid;
  msg.data[2] = data;
  msg.data[3] = 0x00;  // Заполнение
  return msg;
}

// Создание OBD2 ответа для двух байт данных
inline IIsoTp::Message create_obd_response_2_bytes(
    uint32_t rx_id, uint8_t service, uint8_t pid, uint8_t data_a, uint8_t data_b) {
  IIsoTp::Message msg;
  msg.tx_id   = 0x7DF;
  msg.rx_id   = rx_id;
  msg.len     = 8;
  msg.data    = new uint8_t[8];
  msg.data[0] = 4;
  msg.data[1] = service + 0x40;
  msg.data[2] = pid;
  msg.data[3] = data_a;
  msg.data[4] = data_b;
  msg.data[5] = 0x00;
  msg.data[6] = 0x00;
  msg.data[7] = 0x00;
  return msg;
}

// Создание OBD2 ответа для четырех байт данных (например, для supportedPIDs)
inline IIsoTp::Message create_obd_response_4_bytes(uint32_t rx_id,
                                                   uint8_t service,
                                                   uint8_t pid,
                                                   uint8_t data_a,
                                                   uint8_t data_b,
                                                   uint8_t data_c,
                                                   uint8_t data_d) {
  IIsoTp::Message msg;
  msg.tx_id   = 0x7DF;
  msg.rx_id   = rx_id;
  msg.len     = 7;
  msg.data    = new uint8_t[7];
  msg.data[0] = service + 0x40;
  msg.data[1] = pid;
  msg.data[2] = data_a;
  msg.data[3] = data_b;
  msg.data[4] = data_c;
  msg.data[5] = data_d;
  msg.data[6] = 0x00;
  return msg;
}

// Создание сообщения об ошибке OBD2
inline IIsoTp::Message create_obd_error_response(uint32_t rx_id,
                                                 uint8_t service,
                                                 uint8_t error_code) {
  IIsoTp::Message msg;
  msg.tx_id   = 0x7DF;
  msg.rx_id   = rx_id;
  msg.len     = 3;
  msg.data    = new uint8_t[3];
  msg.data[0] = 0x7F;  // Негативный ответ
  msg.data[1] = service;
  msg.data[2] = error_code;
  return msg;
}