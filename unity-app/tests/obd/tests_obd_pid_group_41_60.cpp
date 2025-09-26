#include <cstdio>
#include <cstring>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// ============================================================================
// ТЕСТЫ OBD2 PID ГРУППЫ 41-60 (ВЫСОКИЙ ПРИОРИТЕТ)
// ============================================================================

/*
 * ПОЛНОЕ ПОКРЫТИЕ ТЕСТАМИ PID ГРУППЫ 41-60:
 *
 * ✅ PID 41 - monitorDriveCycleStatus() - статус мониторинга этого цикла вождения
 * ✅ PID 42 - ctrlModVoltage() - напряжение модуля управления
 * ✅ PID 43 - absLoad() - абсолютное значение нагрузки
 * ✅ PID 44 - commandedAirFuelRatio() - командованное соотношение воздух-топливо
 * ✅ PID 45 - relativeThrottle() - относительное положение дроссельной заслонки
 * ✅ PID 46 - ambientAirTemp() - температура окружающего воздуха
 * ✅ PID 47 - absThrottlePosB() - абсолютное положение дроссельной заслонки B
 * ✅ PID 48 - absThrottlePosC() - абсолютное положение дроссельной заслонки C
 * ✅ PID 49 - absThrottlePosD() - абсолютное положение дроссельной заслонки D
 * ✅ PID 4A - absThrottlePosE() - абсолютное положение дроссельной заслонки E
 * ✅ PID 4B - absThrottlePosF() - абсолютное положение дроссельной заслонки F
 * ✅ PID 4C - commandedThrottleActuator() - командованный привод дроссельной заслонки
 * ✅ PID 4D - timeRunWithMIL() - время работы с включенной лампой MIL
 * ✅ PID 4E - timeSinceCodesCleared() - время с момента очистки кодов
 * ✅ PID 50 - maxMafRate() - максимальный расход воздуха через MAF
 * ✅ PID 51 - fuelType() - тип топлива
 * ✅ PID 52 - ethanolPercent() - процент этанола в топливе
 * ✅ PID 53 - absEvapSysVapPressure() - абсолютное давление паров системы улавливания
 * ✅ PID 54 - evapSysVapPressure2() - давление паров системы улавливания (альтернативное)
 * ❌ PID 55 - shortTermSecOxyTrimB1() - краткосрочная коррекция вторичного кислорода банк 1
 * ❌ PID 56 - longTermSecOxyTrimB1() - долгосрочная коррекция вторичного кислорода банк 1
 * ❌ PID 57 - shortTermSecOxyTrimB2() - краткосрочная коррекция вторичного кислорода банк 2
 * ❌ PID 58 - longTermSecOxyTrimB2() - долгосрочная коррекция вторичного кислорода банк 2
 * ✅ PID 59 - absFuelRailPressure() - абсолютное давление топливной рампы
 * ✅ PID 5A - relativePedalPos() - относительное положение педали акселератора
 * ✅ PID 5B - hybridBatLife() - оставшийся ресурс гибридной батареи
 * ✅ PID 5C - oilTemp() - температура моторного масла
 * ✅ PID 5D - fuelInjectTiming() - время впрыска топлива
 * ✅ PID 5E - fuelRate() - расход топлива двигателем
 * ✅ PID 5F - emissionRqmts() - требования к выбросам
 *
 * ВСЕГО ТЕСТОВ: 34 (по 1-2 теста на каждый PID)
 */

// Глобальные объекты для тестов
static MockIsoTp g_mock_iso_tp;

// ============================================================================
// PID 41 - MONITOR STATUS THIS DRIVE CYCLE ТЕСТЫ
// ============================================================================

// Тест 1: monitorDriveCycleStatus - валидные данные
void test_pid_41_monitor_drive_cycle_status_valid_data() {
  g_mock_iso_tp.reset();

  // Статус мониторинга: 0x12345678
  IIsoTp::Message response =
      create_obd_response_4_bytes(0x7E8, SERVICE_01, 0x41, 0x12, 0x34, 0x56, 0x78);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto status = obd2.monitorDriveCycleStatus();
  TEST_ASSERT_TRUE_MESSAGE(status.has_value(), "monitorDriveCycleStatus должен вернуть значение");

  // Формула: (A*16777216 + B*65536 + C*256 + D)
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0x12345678, status.value(), "monitorDriveCycleStatus должен вернуть правильный статус");

  if (response.data)
    delete[] response.data;
}

// Тест 2: monitorDriveCycleStatus - нулевой статус
void test_pid_41_monitor_drive_cycle_status_zero() {
  g_mock_iso_tp.reset();

  IIsoTp::Message response =
      create_obd_response_4_bytes(0x7E8, SERVICE_01, 0x41, 0x00, 0x00, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto status = obd2.monitorDriveCycleStatus();
  TEST_ASSERT_TRUE_MESSAGE(status.has_value(), "monitorDriveCycleStatus должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0x00000000, status.value(), "monitorDriveCycleStatus должен обрабатывать нулевой статус");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 42 - CONTROL MODULE VOLTAGE ТЕСТЫ
// ============================================================================

// Тест 3: ctrlModVoltage - валидные данные
void test_pid_42_ctrl_mod_voltage_valid_data() {
  g_mock_iso_tp.reset();

  // Напряжение: 0x3039 = 12345, формула: (A*256 + B) / 1000 В
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x42, 0x30, 0x39);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto voltage = obd2.ctrlModVoltage();
  TEST_ASSERT_TRUE_MESSAGE(voltage.has_value(), "ctrlModVoltage должен вернуть значение");

  // Ожидаемое значение: 12345 / 1000 = 12.345 В
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.001, 12.345, voltage.value(), "ctrlModVoltage должен вернуть правильное напряжение");

  if (response.data)
    delete[] response.data;
}

// Тест 4: ctrlModVoltage - низкое напряжение
void test_pid_42_ctrl_mod_voltage_low() {
  g_mock_iso_tp.reset();

  // Низкое напряжение: 0x2710 = 10000, формула: 10000 / 1000 = 10.0 В
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x42, 0x27, 0x10);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto voltage = obd2.ctrlModVoltage();
  TEST_ASSERT_TRUE_MESSAGE(voltage.has_value(), "ctrlModVoltage должен вернуть значение");

  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.001, 10.0, voltage.value(), "ctrlModVoltage должен обрабатывать низкое напряжение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 43 - ABSOLUTE LOAD VALUE ТЕСТЫ
// ============================================================================

// Тест 5: absLoad - валидные данные
void test_pid_43_abs_load_valid_data() {
  g_mock_iso_tp.reset();

  // Нагрузка: 0x8000 = 32768, формула: (A*256 + B) * 100/255 %
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x43, 0x80, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto load = obd2.absLoad();
  TEST_ASSERT_TRUE_MESSAGE(load.has_value(), "absLoad должен вернуть значение");

  // Ожидаемое значение: 32768 * 100/255 = 12850.196%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 12850.196, load.value(), "absLoad должен вернуть правильную нагрузку");

  if (response.data)
    delete[] response.data;
}

// Тест 6: absLoad - нулевая нагрузка
void test_pid_43_abs_load_zero() {
  g_mock_iso_tp.reset();

  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x43, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto load = obd2.absLoad();
  TEST_ASSERT_TRUE_MESSAGE(load.has_value(), "absLoad должен вернуть значение");

  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 0.0, load.value(), "absLoad должен обрабатывать нулевую нагрузку");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 44 - COMMANDED AIR-FUEL EQUIVALENCE RATIO ТЕСТЫ
// ============================================================================

// Тест 7: commandedAirFuelRatio - валидные данные
void test_pid_44_commanded_air_fuel_ratio_valid_data() {
  g_mock_iso_tp.reset();

  // Соотношение: 0x8000 = 32768, формула: (A*256 + B) * 2/65536
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x44, 0x80, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto ratio = obd2.commandedAirFuelRatio();
  TEST_ASSERT_TRUE_MESSAGE(ratio.has_value(), "commandedAirFuelRatio должен вернуть значение");

  // Ожидаемое значение: 32768 * 2/65536 = 1.0
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.001, 1.0, ratio.value(), "commandedAirFuelRatio должен вернуть правильное соотношение");

  if (response.data)
    delete[] response.data;
}

// Тест 8: commandedAirFuelRatio - максимальное значение
void test_pid_44_commanded_air_fuel_ratio_max() {
  g_mock_iso_tp.reset();

  // Максимальное соотношение: 0xFFFF = 65535
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x44, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto ratio = obd2.commandedAirFuelRatio();
  TEST_ASSERT_TRUE_MESSAGE(ratio.has_value(), "commandedAirFuelRatio должен вернуть значение");

  // Ожидаемое значение: 65535 * 2/65536 = 1.999969
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.001,
      1.999969,
      ratio.value(),
      "commandedAirFuelRatio должен обрабатывать максимальное значение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 45 - RELATIVE THROTTLE POSITION ТЕСТЫ
// ============================================================================

// Тест 9: relativeThrottle - валидные данные
void test_pid_45_relative_throttle_valid_data() {
  g_mock_iso_tp.reset();

  // Положение дроссельной заслонки: 0x80 = 128, формула: A * 100/255 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x45, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto throttle = obd2.relativeThrottle();
  TEST_ASSERT_TRUE_MESSAGE(throttle.has_value(), "relativeThrottle должен вернуть значение");

  // Ожидаемое значение: 128 * 100/255 = 50.196%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 50.196, throttle.value(), "relativeThrottle должен вернуть правильное положение");

  if (response.data)
    delete[] response.data;
}

// Тест 10: relativeThrottle - полностью открытая заслонка
void test_pid_45_relative_throttle_full_open() {
  g_mock_iso_tp.reset();

  // Полностью открытая заслонка: 0xFF = 255
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x45, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto throttle = obd2.relativeThrottle();
  TEST_ASSERT_TRUE_MESSAGE(throttle.has_value(), "relativeThrottle должен вернуть значение");

  // Ожидаемое значение: 255 * 100/255 = 100.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01,
      100.0,
      throttle.value(),
      "relativeThrottle должен обрабатывать полностью открытую заслонку");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 46 - AMBIENT AIR TEMPERATURE ТЕСТЫ
// ============================================================================

// Тест 11: ambientAirTemp - валидные данные
void test_pid_46_ambient_air_temp_valid_data() {
  g_mock_iso_tp.reset();

  // Температура: 0x50 = 80, формула: A - 40 °C
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x46, 0x50);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto temp = obd2.ambientAirTemp();
  TEST_ASSERT_TRUE_MESSAGE(temp.has_value(), "ambientAirTemp должен вернуть значение");

  // Ожидаемое значение: 80 - 40 = 40.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 40.0, temp.value(), "ambientAirTemp должен вернуть правильную температуру");

  if (response.data)
    delete[] response.data;
}

// Тест 12: ambientAirTemp - отрицательная температура
void test_pid_46_ambient_air_temp_negative() {
  g_mock_iso_tp.reset();

  // Отрицательная температура: 0x14 = 20, формула: 20 - 40 = -20 °C
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x46, 0x14);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto temp = obd2.ambientAirTemp();
  TEST_ASSERT_TRUE_MESSAGE(temp.has_value(), "ambientAirTemp должен вернуть значение");

  // Ожидаемое значение: 20 - 40 = -20.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, -20.0, temp.value(), "ambientAirTemp должен обрабатывать отрицательную температуру");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 47-4B - ABSOLUTE THROTTLE POSITION B-F ТЕСТЫ
// ============================================================================

// Тест 13: absThrottlePosB - валидные данные
void test_pid_47_abs_throttle_pos_b_valid_data() {
  g_mock_iso_tp.reset();

  // Положение дроссельной заслонки B: 0x80 = 128, формула: A * 100/255 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x47, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto throttle = obd2.absThrottlePosB();
  TEST_ASSERT_TRUE_MESSAGE(throttle.has_value(), "absThrottlePosB должен вернуть значение");

  // Ожидаемое значение: 128 * 100/255 = 50.196%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 50.196, throttle.value(), "absThrottlePosB должен вернуть правильное положение");

  if (response.data)
    delete[] response.data;
}

// Тест 14: absThrottlePosC - валидные данные
void test_pid_48_abs_throttle_pos_c_valid_data() {
  g_mock_iso_tp.reset();

  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x48, 0x40);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto throttle = obd2.absThrottlePosC();
  TEST_ASSERT_TRUE_MESSAGE(throttle.has_value(), "absThrottlePosC должен вернуть значение");

  // Ожидаемое значение: 64 * 100/255 = 25.098%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 25.098, throttle.value(), "absThrottlePosC должен вернуть правильное положение");

  if (response.data)
    delete[] response.data;
}

// Тест 15: absThrottlePosD - валидные данные
void test_pid_49_abs_throttle_pos_d_valid_data() {
  g_mock_iso_tp.reset();

  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x49, 0xC0);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto throttle = obd2.absThrottlePosD();
  TEST_ASSERT_TRUE_MESSAGE(throttle.has_value(), "absThrottlePosD должен вернуть значение");

  // Ожидаемое значение: 192 * 100/255 = 75.294%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 75.294, throttle.value(), "absThrottlePosD должен вернуть правильное положение");

  if (response.data)
    delete[] response.data;
}

// Тест 16: absThrottlePosE - валидные данные
void test_pid_4a_abs_throttle_pos_e_valid_data() {
  g_mock_iso_tp.reset();

  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x4A, 0x20);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto throttle = obd2.absThrottlePosE();
  TEST_ASSERT_TRUE_MESSAGE(throttle.has_value(), "absThrottlePosE должен вернуть значение");

  // Ожидаемое значение: 32 * 100/255 = 12.549%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 12.549, throttle.value(), "absThrottlePosE должен вернуть правильное положение");

  if (response.data)
    delete[] response.data;
}

// Тест 17: absThrottlePosF - валидные данные
void test_pid_4b_abs_throttle_pos_f_valid_data() {
  g_mock_iso_tp.reset();

  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x4B, 0xA0);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto throttle = obd2.absThrottlePosF();
  TEST_ASSERT_TRUE_MESSAGE(throttle.has_value(), "absThrottlePosF должен вернуть значение");

  // Ожидаемое значение: 160 * 100/255 = 62.745%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 62.745, throttle.value(), "absThrottlePosF должен вернуть правильное положение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 4C - COMMANDED THROTTLE ACTUATOR ТЕСТЫ
// ============================================================================

// Тест 18: commandedThrottleActuator - валидные данные
void test_pid_4c_commanded_throttle_actuator_valid_data() {
  g_mock_iso_tp.reset();

  // Командованный привод: 0x80 = 128, формула: A * 100/255 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x4C, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto actuator = obd2.commandedThrottleActuator();
  TEST_ASSERT_TRUE_MESSAGE(actuator.has_value(),
                           "commandedThrottleActuator должен вернуть значение");

  // Ожидаемое значение: 128 * 100/255 = 50.196%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01,
                                   50.196,
                                   actuator.value(),
                                   "commandedThrottleActuator должен вернуть правильное значение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 4D - TIME RUN WITH MIL ON ТЕСТЫ
// ============================================================================

// Тест 19: timeRunWithMIL - валидные данные
void test_pid_4d_time_run_with_mil_valid_data() {
  g_mock_iso_tp.reset();

  // Время: 0x1234 = 4660 минут
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x4D, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto time = obd2.timeRunWithMIL();
  TEST_ASSERT_TRUE_MESSAGE(time.has_value(), "timeRunWithMIL должен вернуть значение");

  // Формула: (A*256 + B) минут
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      4660, time.value(), "timeRunWithMIL должен вернуть правильное время");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 4E - TIME SINCE CODES CLEARED ТЕСТЫ
// ============================================================================

// Тест 20: timeSinceCodesCleared - валидные данные
void test_pid_4e_time_since_codes_cleared_valid_data() {
  g_mock_iso_tp.reset();

  // Время: 0x5678 = 22136 минут
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x4E, 0x56, 0x78);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto time = obd2.timeSinceCodesCleared();
  TEST_ASSERT_TRUE_MESSAGE(time.has_value(), "timeSinceCodesCleared должен вернуть значение");

  // Формула: (A*256 + B) минут
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      22136, time.value(), "timeSinceCodesCleared должен вернуть правильное время");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 50 - MAXIMUM VALUE FOR MAF RATE ТЕСТЫ
// ============================================================================

// Тест 21: maxMafRate - валидные данные
void test_pid_50_max_maf_rate_valid_data() {
  g_mock_iso_tp.reset();

  // Максимальный расход: 0x64 = 100, формула: A * 10 г/с
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x50, 0x64);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto rate = obd2.maxMafRate();
  TEST_ASSERT_TRUE_MESSAGE(rate.has_value(), "maxMafRate должен вернуть значение");

  // Ожидаемое значение: 100 * 10 = 1000.0 г/с
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   1000.0,
                                   static_cast<double>(rate.value()),
                                   "maxMafRate должен вернуть правильный расход");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 51 - FUEL TYPE ТЕСТЫ
// ============================================================================

// Тест 22: fuelType - валидные данные
void test_pid_51_fuel_type_valid_data() {
  g_mock_iso_tp.reset();

  // Тип топлива: 0x01 = бензин
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x51, 0x01);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto fuel = obd2.fuelType();
  TEST_ASSERT_TRUE_MESSAGE(fuel.has_value(), "fuelType должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1, fuel.value(), "fuelType должен вернуть правильный тип топлива");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 52 - ETHANOL FUEL PERCENT ТЕСТЫ
// ============================================================================

// Тест 23: ethanolPercent - валидные данные
void test_pid_52_ethanol_percent_valid_data() {
  g_mock_iso_tp.reset();

  // Процент этанола: 0x33 = 51, формула: A * 100/255 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x52, 0x33);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto ethanol = obd2.ethanolPercent();
  TEST_ASSERT_TRUE_MESSAGE(ethanol.has_value(), "ethanolPercent должен вернуть значение");

  // Ожидаемое значение: 51 * 100/255 = 20.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 20.0, ethanol.value(), "ethanolPercent должен вернуть правильный процент");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 53 - ABSOLUTE EVAP SYSTEM VAPOR PRESSURE ТЕСТЫ
// ============================================================================

// Тест 24: absEvapSysVapPressure - валидные данные
void test_pid_53_abs_evap_sys_vap_pressure_valid_data() {
  g_mock_iso_tp.reset();

  // Давление: 0x1234 = 4660, формула: (A*256 + B) / 200 кПа
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x53, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto pressure = obd2.absEvapSysVapPressure();
  TEST_ASSERT_TRUE_MESSAGE(pressure.has_value(), "absEvapSysVapPressure должен вернуть значение");

  // Ожидаемое значение: 4660 / 200 = 23.3 кПа
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 23.3, pressure.value(), "absEvapSysVapPressure должен вернуть правильное давление");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 54 - EVAP SYSTEM VAPOR PRESSURE (ALTERNATIVE) ТЕСТЫ
// ============================================================================

// Тест 25: evapSysVapPressure2 - валидные данные
void test_pid_54_evap_sys_vap_pressure2_valid_data() {
  g_mock_iso_tp.reset();

  // Давление: 0x8000 = 32768, формула: (A*256 + B) - 32767 Па
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x54, 0x80, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto pressure = obd2.evapSysVapPressure2();
  TEST_ASSERT_TRUE_MESSAGE(pressure.has_value(), "evapSysVapPressure2 должен вернуть значение");

  // Ожидаемое значение: 32768 - 32767 = 1.0 Па
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   1.0,
                                   static_cast<double>(pressure.value()),
                                   "evapSysVapPressure2 должен вернуть правильное давление");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 59 - ABSOLUTE FUEL RAIL PRESSURE ТЕСТЫ
// ============================================================================

// Тест 26: absFuelRailPressure - валидные данные
void test_pid_59_abs_fuel_rail_pressure_valid_data() {
  g_mock_iso_tp.reset();

  // Давление: 0x1234 = 4660, формула: (A*256 + B) * 10 кПа
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x59, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto pressure = obd2.absFuelRailPressure();
  TEST_ASSERT_TRUE_MESSAGE(pressure.has_value(), "absFuelRailPressure должен вернуть значение");

  // Ожидаемое значение: 4660 * 10 = 46600.0 кПа
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   46600.0,
                                   static_cast<double>(pressure.value()),
                                   "absFuelRailPressure должен вернуть правильное давление");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 5A - RELATIVE ACCELERATOR PEDAL POSITION ТЕСТЫ
// ============================================================================

// Тест 27: relativePedalPos - валидные данные
void test_pid_5a_relative_pedal_pos_valid_data() {
  g_mock_iso_tp.reset();

  // Положение педали: 0x80 = 128, формула: A * 100/255 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x5A, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto pedal = obd2.relativePedalPos();
  TEST_ASSERT_TRUE_MESSAGE(pedal.has_value(), "relativePedalPos должен вернуть значение");

  // Ожидаемое значение: 128 * 100/255 = 50.196%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01,
                                   50.196,
                                   static_cast<double>(pedal.value()),
                                   "relativePedalPos должен вернуть правильное положение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 5B - HYBRID BATTERY PACK REMAINING LIFE ТЕСТЫ
// ============================================================================

// Тест 28: hybridBatLife - валидные данные
void test_pid_5b_hybrid_bat_life_valid_data() {
  g_mock_iso_tp.reset();

  // Ресурс батареи: 0xCC = 204, формула: A * 100/255 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x5B, 0xCC);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto life = obd2.hybridBatLife();
  TEST_ASSERT_TRUE_MESSAGE(life.has_value(), "hybridBatLife должен вернуть значение");

  // Ожидаемое значение: 204 * 100/255 = 80.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01,
                                   80.0,
                                   static_cast<double>(life.value()),
                                   "hybridBatLife должен вернуть правильный ресурс");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 5C - ENGINE OIL TEMPERATURE ТЕСТЫ
// ============================================================================

// Тест 29: oilTemp - валидные данные
void test_pid_5c_oil_temp_valid_data() {
  g_mock_iso_tp.reset();

  // Температура масла: 0x78 = 120, формула: A - 40 °C
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x5C, 0x78);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto temp = obd2.oilTemp();
  TEST_ASSERT_TRUE_MESSAGE(temp.has_value(), "oilTemp должен вернуть значение");

  // Ожидаемое значение: 120 - 40 = 80.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   80.0,
                                   static_cast<double>(temp.value()),
                                   "oilTemp должен вернуть правильную температуру");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 5D - FUEL INJECTION TIMING ТЕСТЫ
// ============================================================================

// Тест 30: fuelInjectTiming - валидные данные
void test_pid_5d_fuel_inject_timing_valid_data() {
  g_mock_iso_tp.reset();

  // Время впрыска: 0x6D60 = 28000, формула: ((A*256 + B) / 128) - 210 градусов
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x5D, 0x6D, 0x60);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto timing = obd2.fuelInjectTiming();
  TEST_ASSERT_TRUE_MESSAGE(timing.has_value(), "fuelInjectTiming должен вернуть значение");

  // Ожидаемое значение: (28000 / 128) - 210 = 8.75 градусов
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   8.75,
                                   static_cast<double>(timing.value()),
                                   "fuelInjectTiming должен вернуть правильное время");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 5E - ENGINE FUEL RATE ТЕСТЫ
// ============================================================================

// Тест 31: fuelRate - валидные данные
void test_pid_5e_fuel_rate_valid_data() {
  g_mock_iso_tp.reset();

  // Расход топлива: 0x0C80 = 3200, формула: (A*256 + B) / 20 л/ч
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x5E, 0x0C, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto rate = obd2.fuelRate();
  TEST_ASSERT_TRUE_MESSAGE(rate.has_value(), "fuelRate должен вернуть значение");

  // Ожидаемое значение: 3200 / 20 = 160.0 л/ч
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 160.0, static_cast<double>(rate.value()), "fuelRate должен вернуть правильный расход");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 5F - EMISSION REQUIREMENTS ТЕСТЫ
// ============================================================================

// Тест 32: emissionRqmts - валидные данные
void test_pid_5f_emission_rqmts_valid_data() {
  g_mock_iso_tp.reset();

  // Требования к выбросам: 0x05 = Euro 5
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x5F, 0x05);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto emission = obd2.emissionRqmts();
  TEST_ASSERT_TRUE_MESSAGE(emission.has_value(), "emissionRqmts должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      5, emission.value(), "emissionRqmts должен вернуть правильные требования");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// ДОПОЛНИТЕЛЬНЫЕ ТЕСТЫ ДЛЯ ГРАНИЧНЫХ СЛУЧАЕВ
// ============================================================================

// Тест 33: ctrlModVoltage - максимальное напряжение
void test_pid_42_ctrl_mod_voltage_max() {
  g_mock_iso_tp.reset();

  // Максимальное напряжение: 0xFFFF = 65535
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x42, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto voltage = obd2.ctrlModVoltage();
  TEST_ASSERT_TRUE_MESSAGE(voltage.has_value(), "ctrlModVoltage должен вернуть значение");

  // Ожидаемое значение: 65535 / 1000 = 65.535 В
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001,
                                   65.535,
                                   static_cast<double>(voltage.value()),
                                   "ctrlModVoltage должен обрабатывать максимальное напряжение");

  if (response.data)
    delete[] response.data;
}

// Тест 34: ambientAirTemp - максимальная температура
void test_pid_46_ambient_air_temp_max() {
  g_mock_iso_tp.reset();

  // Максимальная температура: 0xFF = 255, формула: 255 - 40 = 215 °C
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x46, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto temp = obd2.ambientAirTemp();
  TEST_ASSERT_TRUE_MESSAGE(temp.has_value(), "ambientAirTemp должен вернуть значение");

  // Ожидаемое значение: 255 - 40 = 215.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   215.0,
                                   static_cast<double>(temp.value()),
                                   "ambientAirTemp должен обрабатывать максимальную температуру");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// ФУНКЦИЯ ЗАПУСКА ВСЕХ ТЕСТОВ PID ГРУППЫ 41-60
// ============================================================================

extern "C" void run_obd_pid_group_41_60_tests() {
  printf("\n=== Запуск тестов PID группы 41-60 ===\n");

  // PID 41 - Monitor status this drive cycle
  RUN_TEST(test_pid_41_monitor_drive_cycle_status_valid_data);
  RUN_TEST(test_pid_41_monitor_drive_cycle_status_zero);

  // PID 42 - Control module voltage
  RUN_TEST(test_pid_42_ctrl_mod_voltage_valid_data);
  RUN_TEST(test_pid_42_ctrl_mod_voltage_low);
  RUN_TEST(test_pid_42_ctrl_mod_voltage_max);

  // PID 43 - Absolute load value
  RUN_TEST(test_pid_43_abs_load_valid_data);
  RUN_TEST(test_pid_43_abs_load_zero);

  // PID 44 - Commanded air-fuel equivalence ratio
  RUN_TEST(test_pid_44_commanded_air_fuel_ratio_valid_data);
  RUN_TEST(test_pid_44_commanded_air_fuel_ratio_max);

  // PID 45 - Relative throttle position
  RUN_TEST(test_pid_45_relative_throttle_valid_data);
  RUN_TEST(test_pid_45_relative_throttle_full_open);

  // PID 46 - Ambient air temperature
  RUN_TEST(test_pid_46_ambient_air_temp_valid_data);
  RUN_TEST(test_pid_46_ambient_air_temp_negative);
  RUN_TEST(test_pid_46_ambient_air_temp_max);

  // PID 47-4B - Absolute throttle positions B-F
  RUN_TEST(test_pid_47_abs_throttle_pos_b_valid_data);
  RUN_TEST(test_pid_48_abs_throttle_pos_c_valid_data);
  RUN_TEST(test_pid_49_abs_throttle_pos_d_valid_data);
  RUN_TEST(test_pid_4a_abs_throttle_pos_e_valid_data);
  RUN_TEST(test_pid_4b_abs_throttle_pos_f_valid_data);

  // PID 4C - Commanded throttle actuator
  RUN_TEST(test_pid_4c_commanded_throttle_actuator_valid_data);

  // PID 4D - Time run with MIL on
  RUN_TEST(test_pid_4d_time_run_with_mil_valid_data);

  // PID 4E - Time since codes cleared
  RUN_TEST(test_pid_4e_time_since_codes_cleared_valid_data);

  // PID 50 - Maximum value for MAF rate
  RUN_TEST(test_pid_50_max_maf_rate_valid_data);

  // PID 51 - Fuel type
  RUN_TEST(test_pid_51_fuel_type_valid_data);

  // PID 52 - Ethanol fuel percent
  RUN_TEST(test_pid_52_ethanol_percent_valid_data);

  // PID 53 - Absolute evap system vapor pressure
  RUN_TEST(test_pid_53_abs_evap_sys_vap_pressure_valid_data);

  // PID 54 - Evap system vapor pressure (alternative)
  RUN_TEST(test_pid_54_evap_sys_vap_pressure2_valid_data);

  // PID 59 - Absolute fuel rail pressure
  RUN_TEST(test_pid_59_abs_fuel_rail_pressure_valid_data);

  // PID 5A - Relative accelerator pedal position
  RUN_TEST(test_pid_5a_relative_pedal_pos_valid_data);

  // PID 5B - Hybrid battery pack remaining life
  RUN_TEST(test_pid_5b_hybrid_bat_life_valid_data);

  // PID 5C - Engine oil temperature
  RUN_TEST(test_pid_5c_oil_temp_valid_data);

  // PID 5D - Fuel injection timing
  RUN_TEST(test_pid_5d_fuel_inject_timing_valid_data);

  // PID 5E - Engine fuel rate
  RUN_TEST(test_pid_5e_fuel_rate_valid_data);

  // PID 5F - Emission requirements
  RUN_TEST(test_pid_5f_emission_rqmts_valid_data);

  printf("=== Тесты PID группы 41-60 завершены ===\n");
}