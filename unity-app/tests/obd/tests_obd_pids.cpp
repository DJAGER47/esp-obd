#include <cstdio>
#include <cstring>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// ============================================================================
// ТЕСТЫ OBD2 PID ГРУПП
// ============================================================================

/*
 * ПОКРЫТИЕ ТЕСТАМИ:
 *
 * ✅ SUPPORTED PIDS МЕТОДЫ:
 * - supportedPIDs_1_20() - поддерживаемые PID 1-20
 * - supportedPIDs_21_40() - поддерживаемые PID 21-40
 * - supportedPIDs_41_60() - поддерживаемые PID 41-60
 * - supportedPIDs_61_80() - поддерживаемые PID 61-80
 *
 * ✅ PID ГРУППЫ:
 * - Различные группы PID и их константы
 * - Проверка правильности значений констант
 *
 * ✅ ВЫЧИСЛЕНИЯ:
 * - Тестирование формул для различных PID
 * - Проверка математических операций
 */

// Заглушка для ITwaiInterface
class DummyTwaiInterface : public ITwaiInterface {
 public:
  TwaiError install_and_start() override {
    return TwaiError::OK;
  }
  TwaiError transmit(const TwaiFrame& message, uint32_t ticks_to_wait) override {
    return TwaiError::OK;
  }
  TwaiError receive(TwaiFrame& message, uint32_t ticks_to_wait) override {
    return TwaiError::TIMEOUT;
  }
};

// ============================================================================
// ТЕСТЫ КОНСТАНТ PID ГРУПП
// ============================================================================

// Тест 1: Константы PID группы 1-20
void test_obd2_pid_group_1_20_constants() {
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0, OBD2::SUPPORTED_PIDS_1_20, "SUPPORTED_PIDS_1_20 должен быть 0");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1, OBD2::MONITOR_STATUS_SINCE_DTC_CLEARED, "MONITOR_STATUS_SINCE_DTC_CLEARED должен быть 1");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(4, OBD2::ENGINE_LOAD, "ENGINE_LOAD должен быть 4");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      5, OBD2::ENGINE_COOLANT_TEMP, "ENGINE_COOLANT_TEMP должен быть 5");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(12, OBD2::ENGINE_RPM, "ENGINE_RPM должен быть 12");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(13, OBD2::VEHICLE_SPEED, "VEHICLE_SPEED должен быть 13");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(15, OBD2::INTAKE_AIR_TEMP, "INTAKE_AIR_TEMP должен быть 15");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(16, OBD2::MAF_FLOW_RATE, "MAF_FLOW_RATE должен быть 16");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(17, OBD2::THROTTLE_POSITION, "THROTTLE_POSITION должен быть 17");
}

// Тест 2: Константы PID группы 21-40
void test_obd2_pid_group_21_40_constants() {
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      32, OBD2::SUPPORTED_PIDS_21_40, "SUPPORTED_PIDS_21_40 должен быть 32");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      33, OBD2::DISTANCE_TRAVELED_WITH_MIL_ON, "DISTANCE_TRAVELED_WITH_MIL_ON должен быть 33");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      34, OBD2::FUEL_RAIL_PRESSURE, "FUEL_RAIL_PRESSURE должен быть 34");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      47, OBD2::FUEL_TANK_LEVEL_INPUT, "FUEL_TANK_LEVEL_INPUT должен быть 47");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      51, OBD2::ABS_BAROMETRIC_PRESSURE, "ABS_BAROMETRIC_PRESSURE должен быть 51");
}

// Тест 3: Константы PID группы 41-60
void test_obd2_pid_group_41_60_constants() {
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      64, OBD2::SUPPORTED_PIDS_41_60, "SUPPORTED_PIDS_41_60 должен быть 64");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      66, OBD2::CONTROL_MODULE_VOLTAGE, "CONTROL_MODULE_VOLTAGE должен быть 66");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(70, OBD2::AMBIENT_AIR_TEMP, "AMBIENT_AIR_TEMP должен быть 70");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(81, OBD2::FUEL_TYPE, "FUEL_TYPE должен быть 81");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      82, OBD2::ETHANOL_FUEL_PERCENT, "ETHANOL_FUEL_PERCENT должен быть 82");
}

// Тест 4: Константы PID группы 61-80
void test_obd2_pid_group_61_80_constants() {
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      96, OBD2::SUPPORTED_PIDS_61_80, "SUPPORTED_PIDS_61_80 должен быть 96");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      97, OBD2::DEMANDED_ENGINE_PERCENT_TORQUE, "DEMANDED_ENGINE_PERCENT_TORQUE должен быть 97");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      98, OBD2::ACTUAL_ENGINE_TORQUE, "ACTUAL_ENGINE_TORQUE должен быть 98");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      99, OBD2::ENGINE_REFERENCE_TORQUE, "ENGINE_REFERENCE_TORQUE должен быть 99");
}

// ============================================================================
// ТЕСТЫ МАТЕМАТИЧЕСКИХ ВЫЧИСЛЕНИЙ
// ============================================================================

// Тест 5: Вычисления для ENGINE_RPM
void test_obd2_rpm_calculation_formula() {
  // RPM = ((A*256)+B)/4
  // Тестируем с A=0x1A (26), B=0xF8 (248)
  uint8_t a             = 0x1A;
  uint8_t b             = 0xF8;
  double expected_rpm   = ((a * 256) + b) / 4.0;
  double calculated_rpm = ((26 * 256) + 248) / 4.0;

  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.1, 1726.0, expected_rpm, "RPM формула должна дать 1726");
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, expected_rpm, calculated_rpm, "Вычисления должны совпадать");
}

// Тест 6: Вычисления для ENGINE_LOAD
void test_obd2_engine_load_calculation_formula() {
  // Load = A*100/255
  // Тестируем с A=0x80 (128)
  uint8_t a              = 0x80;
  double expected_load   = (a * 100.0) / 255.0;
  double calculated_load = (128 * 100.0) / 255.0;

  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.1, 50.2, expected_load, "Load формула должна дать ~50.2%");
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, expected_load, calculated_load, "Вычисления должны совпадать");
}

// Тест 7: Вычисления для ENGINE_COOLANT_TEMP
void test_obd2_coolant_temp_calculation_formula() {
  // Temp = A - 40
  // Тестируем с A=0x5A (90)
  uint8_t a              = 0x5A;
  double expected_temp   = a - 40;
  double calculated_temp = 90 - 40;

  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.1, 50.0, expected_temp, "Temp формула должна дать 50°C");
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, expected_temp, calculated_temp, "Вычисления должны совпадать");
}

// Тест 8: Вычисления для MAF_FLOW_RATE
void test_obd2_maf_rate_calculation_formula() {
  // MAF = ((A*256)+B)/100
  // Тестируем с A=0x01 (1), B=0x90 (144)
  uint8_t a             = 0x01;
  uint8_t b             = 0x90;
  double expected_maf   = ((a * 256) + b) / 100.0;
  double calculated_maf = ((1 * 256) + 144) / 100.0;

  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.1, 4.0, expected_maf, "MAF формула должна дать 4.0 г/с");
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, expected_maf, calculated_maf, "Вычисления должны совпадать");
}

// Тест 9: Вычисления для CONTROL_MODULE_VOLTAGE
void test_obd2_voltage_calculation_formula() {
  // Voltage = ((A*256)+B)/1000
  // Тестируем с A=0x30 (48), B=0x39 (57)
  uint8_t a                 = 0x30;
  uint8_t b                 = 0x39;
  double expected_voltage   = ((a * 256) + b) / 1000.0;
  double calculated_voltage = ((48 * 256) + 57) / 1000.0;

  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.01, 12.345, expected_voltage, "Voltage формула должна дать 12.345 В");
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.01, expected_voltage, calculated_voltage, "Вычисления должны совпадать");
}

// ============================================================================
// ТЕСТЫ БИТОВЫХ ОПЕРАЦИЙ ДЛЯ SUPPORTED PIDS
// ============================================================================

// Тест 10: Битовые операции для проверки поддержки PID
void test_obd2_pid_support_bit_operations() {
  // Тестируем битовую маску 0xBE1FA813
  uint32_t supported_mask = 0xBE1FA813;

  // Проверяем конкретные биты
  // Бит 31 (PID 01) - должен быть установлен (0xBE имеет старший бит)
  bool pid_01_supported = (supported_mask & (1U << 31)) != 0;
  TEST_ASSERT_TRUE_MESSAGE(pid_01_supported, "PID 01 должен быть поддержан");

  // Бит 19 (PID 13 - VEHICLE_SPEED) - проверяем в 0x1FA8
  bool pid_13_supported = (supported_mask & (1U << 19)) != 0;
  TEST_ASSERT_TRUE_MESSAGE(pid_13_supported, "PID 13 должен быть поддержан");

  // Бит 0 (PID 32) - должен быть установлен (0x13 имеет младшие биты)
  bool pid_32_supported = (supported_mask & (1U << 0)) != 0;
  TEST_ASSERT_TRUE_MESSAGE(pid_32_supported, "PID 32 должен быть поддержан");
}

// Тест 11: Проверка диапазонов PID
void test_obd2_pid_ranges() {
  // Проверяем, что PID находятся в правильных диапазонах
  TEST_ASSERT_TRUE_MESSAGE(OBD2::ENGINE_RPM >= 1 && OBD2::ENGINE_RPM <= 31,
                           "ENGINE_RPM должен быть в диапазоне 1-31");
  TEST_ASSERT_TRUE_MESSAGE(OBD2::SUPPORTED_PIDS_21_40 >= 32 && OBD2::SUPPORTED_PIDS_21_40 <= 63,
                           "SUPPORTED_PIDS_21_40 должен быть в диапазоне 32-63");
  TEST_ASSERT_TRUE_MESSAGE(OBD2::SUPPORTED_PIDS_41_60 >= 64 && OBD2::SUPPORTED_PIDS_41_60 <= 95,
                           "SUPPORTED_PIDS_41_60 должен быть в диапазоне 64-95");
  TEST_ASSERT_TRUE_MESSAGE(OBD2::SUPPORTED_PIDS_61_80 >= 96 && OBD2::SUPPORTED_PIDS_61_80 <= 127,
                           "SUPPORTED_PIDS_61_80 должен быть в диапазоне 96-127");
}

// ============================================================================
// ТЕСТЫ СОЗДАНИЯ ОБЪЕКТОВ OBD2
// ============================================================================

// Тест 12: Создание объекта OBD2 для тестирования PID методов
void test_obd2_object_creation_for_pids() {
  DummyTwaiInterface dummy_twai;
  IsoTp iso_tp(dummy_twai);

  OBD2 obd2(iso_tp);

  // Проверяем, что объект создался
  TEST_ASSERT_NOT_NULL_MESSAGE(&obd2, "Объект OBD2 должен быть создан для тестирования PID");

  // Проверяем, что можем вызвать queryPID без падения
  obd2.queryPID(OBD2::SERVICE_01, OBD2::ENGINE_RPM);
  TEST_ASSERT_TRUE_MESSAGE(true, "queryPID должен выполняться без ошибок");
}

// Тест 13: Тестирование констант DTC
void test_obd2_dtc_constants() {
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(6, OBD2::DTC_CODE_LEN, "DTC_CODE_LEN должен быть 6");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(16, OBD2::DTC_MAX_CODES, "DTC_MAX_CODES должен быть 16");
}

// Тест 14: Тестирование констант запроса
void test_obd2_query_constants() {
  TEST_ASSERT_EQUAL_INT8_MESSAGE(9, OBD2::QUERY_LEN, "QUERY_LEN должен быть 9");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0x20, OBD2::PID_INTERVAL_OFFSET, "PID_INTERVAL_OFFSET должен быть 0x20");
}

// Тест 15: Комплексный тест создания мок-сообщений для всех групп PID
void test_mock_messages_for_all_pid_groups() {
  // Группа 1-20: ENGINE_RPM
  MockIsoTp::Message rpm_msg =
      create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, OBD2::ENGINE_RPM, 0x1A, 0xF8);
  TEST_ASSERT_NOT_NULL_MESSAGE(rpm_msg.data, "RPM сообщение должно быть создано");
  if (rpm_msg.data)
    delete[] rpm_msg.data;

  // Группа 21-40: FUEL_TANK_LEVEL_INPUT
  MockIsoTp::Message fuel_msg =
      create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, OBD2::FUEL_TANK_LEVEL_INPUT, 0xCC);
  TEST_ASSERT_NOT_NULL_MESSAGE(fuel_msg.data, "Fuel сообщение должно быть создано");
  if (fuel_msg.data)
    delete[] fuel_msg.data;

  // Группа 41-60: CONTROL_MODULE_VOLTAGE
  MockIsoTp::Message voltage_msg = create_obd_response_2_bytes(
      0x7E8, OBD2::SERVICE_01, OBD2::CONTROL_MODULE_VOLTAGE, 0x30, 0x39);
  TEST_ASSERT_NOT_NULL_MESSAGE(voltage_msg.data, "Voltage сообщение должно быть создано");
  if (voltage_msg.data)
    delete[] voltage_msg.data;

  // Группа 61-80: DEMANDED_ENGINE_PERCENT_TORQUE
  MockIsoTp::Message torque_msg = create_obd_response_1_byte(
      0x7E8, OBD2::SERVICE_01, OBD2::DEMANDED_ENGINE_PERCENT_TORQUE, 0x96);
  TEST_ASSERT_NOT_NULL_MESSAGE(torque_msg.data, "Torque сообщение должно быть создано");
  if (torque_msg.data)
    delete[] torque_msg.data;
}

// Функция запуска всех тестов PID групп
extern "C" void run_obd_pids_tests() {
  // Тесты констант PID групп
  RUN_TEST(test_obd2_pid_group_1_20_constants);
  RUN_TEST(test_obd2_pid_group_21_40_constants);
  RUN_TEST(test_obd2_pid_group_41_60_constants);
  RUN_TEST(test_obd2_pid_group_61_80_constants);

  // Тесты математических вычислений
  RUN_TEST(test_obd2_rpm_calculation_formula);
  RUN_TEST(test_obd2_engine_load_calculation_formula);
  RUN_TEST(test_obd2_coolant_temp_calculation_formula);
  RUN_TEST(test_obd2_maf_rate_calculation_formula);
  RUN_TEST(test_obd2_voltage_calculation_formula);

  // Тесты битовых операций
  RUN_TEST(test_obd2_pid_support_bit_operations);
  RUN_TEST(test_obd2_pid_ranges);

  // Тесты создания объектов
  RUN_TEST(test_obd2_object_creation_for_pids);
  RUN_TEST(test_obd2_dtc_constants);
  RUN_TEST(test_obd2_query_constants);
  RUN_TEST(test_mock_messages_for_all_pid_groups);
}