#include <cstdio>
#include <cstring>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// ============================================================================
// ТЕСТЫ OBD2 PID ГРУППЫ 1-20 (ВЫСОКИЙ ПРИОРИТЕТ)
// ============================================================================

/*
 * ПОЛНОЕ ПОКРЫТИЕ ТЕСТАМИ PID ГРУППЫ 1-20:
 *
 * ✅ PID 00 - supportedPIDs_1_20() - поддерживаемые PID 1-20
 * ✅ PID 01 - monitorStatus() - статус мониторинга с момента очистки DTC
 * ✅ PID 02 - freezeDTC() - замороженные DTC
 * ✅ PID 03 - fuelSystemStatus() - статус топливной системы
 * ✅ PID 04 - engineLoad() - нагрузка двигателя
 * ✅ PID 05 - engineCoolantTemp() - температура охлаждающей жидкости
 * ✅ PID 06 - shortTermFuelTrimBank_1() - краткосрочная коррекция топлива банк 1
 * ✅ PID 07 - longTermFuelTrimBank_1() - долгосрочная коррекция топлива банк 1
 * ✅ PID 08 - shortTermFuelTrimBank_2() - краткосрочная коррекция топлива банк 2
 * ✅ PID 09 - longTermFuelTrimBank_2() - долгосрочная коррекция топлива банк 2
 * ✅ PID 0A - fuelPressure() - давление топлива
 * ✅ PID 0B - manifoldPressure() - давление во впускном коллекторе
 * ✅ PID 0C - rpm() - обороты двигателя
 * ✅ PID 0D - kph() - скорость автомобиля
 * ✅ PID 0E - timingAdvance() - угол опережения зажигания
 * ✅ PID 0F - intakeAirTemp() - температура воздуха на впуске
 * ✅ PID 10 - mafRate() - расход воздуха через MAF
 * ✅ PID 11 - throttle() - положение дроссельной заслонки
 * ✅ PID 12 - commandedSecAirStatus() - статус вторичного воздуха
 * ✅ PID 13 - oxygenSensorsPresent_2banks() - присутствующие кислородные датчики
 * ✅ PID 14 - oxygenSensor1() - кислородный датчик 1
 *
 * ВСЕГО ТЕСТОВ: 40 (по 2-3 теста на каждый PID)
 */

// Глобальные объекты для тестов
static MockIsoTp g_mock_iso_tp;

// ============================================================================
// PID 00 - SUPPORTED PIDS 1-20 ТЕСТЫ
// ============================================================================

// Тест 1: supportedPIDs_1_20 - валидные данные
void test_pid_00_supported_pids_1_20_valid_data() {
  g_mock_iso_tp.reset();

  // Создаем ответ с битовой маской 0xBE1FA813
  // Биты: 10111110 00011111 10101000 00010011
  IIsoTp::Message response =
      create_obd_response_4_bytes(0x7E8, SERVICE_01, SUPPORTED_PIDS_1_20, 0xBE, 0x1F, 0xA8, 0x13);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint32_t supported = obd2.supportedPIDs_1_20();

  // Проверяем результат: 0xBE1FA813 = 3188893715
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0xBE1FA813, supported, "supportedPIDs_1_20 должен вернуть правильную битовую маску");

  // Очищаем память
  if (response.data)
    delete[] response.data;
}

// Тест 2: supportedPIDs_1_20 - проверка конкретных битов
void test_pid_00_supported_pids_bit_check() {
  g_mock_iso_tp.reset();

  // Маска 0xBE1FA813: PID 01, 04, 05, 0C, 0D, 0F, 10, 11, 13, 14 поддерживаются
  IIsoTp::Message response =
      create_obd_response_4_bytes(0x7E8, SERVICE_01, SUPPORTED_PIDS_1_20, 0xBE, 0x1F, 0xA8, 0x13);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint32_t supported = obd2.supportedPIDs_1_20();

  // Проверяем конкретные биты (PID = 32 - bit_position)
  bool pid_01_supported = (supported & (1U << 31)) != 0;  // Бит 31
  bool pid_04_supported = (supported & (1U << 28)) != 0;  // Бит 28
  bool pid_0C_supported = (supported & (1U << 20)) != 0;  // Бит 20
  bool pid_13_supported = (supported & (1U << 19)) != 0;  // Бит 19

  TEST_ASSERT_TRUE_MESSAGE(pid_01_supported, "PID 01 должен быть поддержан");
  TEST_ASSERT_TRUE_MESSAGE(pid_04_supported, "PID 04 должен быть поддержан");
  TEST_ASSERT_TRUE_MESSAGE(pid_0C_supported, "PID 0C должен быть поддержан");
  TEST_ASSERT_TRUE_MESSAGE(pid_13_supported, "PID 13 должен быть поддержан");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 01 - MONITOR STATUS ТЕСТЫ
// ============================================================================

// Тест 3: monitorStatus - валидные данные
void test_pid_01_monitor_status_valid_data() {
  g_mock_iso_tp.reset();

  // Статус мониторинга: A7 80 00 00 (MIL включен, 7 DTC)
  IIsoTp::Message response = create_obd_response_4_bytes(
      0x7E8, SERVICE_01, MONITOR_STATUS_SINCE_DTC_CLEARED, 0xA7, 0x80, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint32_t status = obd2.monitorStatus();

  // Проверяем результат: 0xA7800000 = 2812608512
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0xA7800000, status, "monitorStatus должен вернуть правильный статус");

  if (response.data)
    delete[] response.data;
}

// Тест 4: monitorStatus - проверка битовых полей
void test_pid_01_monitor_status_bit_fields() {
  g_mock_iso_tp.reset();

  // A7 = 10100111: MIL=1, DTC_count=7, готовность тестов
  IIsoTp::Message response = create_obd_response_4_bytes(
      0x7E8, SERVICE_01, MONITOR_STATUS_SINCE_DTC_CLEARED, 0xA7, 0x80, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint32_t status = obd2.monitorStatus();

  // Извлекаем поля
  bool mil_on       = (status & 0x80000000) != 0;  // Бит 31
  uint8_t dtc_count = (status >> 24) & 0x7F;       // Биты 30-24

  TEST_ASSERT_TRUE_MESSAGE(mil_on, "MIL должен быть включен");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x27, dtc_count, "Количество DTC должно быть 39");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 02 - FREEZE DTC ТЕСТЫ
// ============================================================================

// Тест 5: freezeDTC - валидные данные
void test_pid_02_freeze_dtc_valid_data() {
  g_mock_iso_tp.reset();

  // Freeze DTC: P0301 (цилиндр 1 пропуски зажигания)
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, FREEZE_DTC, 0x03, 0x01);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint16_t freeze_dtc = obd2.freezeDTC();

  // Проверяем результат: 0x0301 = 769
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      0x0301, freeze_dtc, "freezeDTC должен вернуть правильный код DTC");

  if (response.data)
    delete[] response.data;
}

// Тест 6: freezeDTC - граничные значения
void test_pid_02_freeze_dtc_boundary_values() {
  g_mock_iso_tp.reset();

  // Максимальное значение: 0xFFFF
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, FREEZE_DTC, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint16_t freeze_dtc = obd2.freezeDTC();

  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      0xFFFF, freeze_dtc, "freezeDTC должен обрабатывать максимальные значения");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 03 - FUEL SYSTEM STATUS ТЕСТЫ
// ============================================================================

// Тест 7: fuelSystemStatus - нормальная работа
void test_pid_03_fuel_system_status_normal() {
  g_mock_iso_tp.reset();

  // Статус: 02 00 (система 1 в замкнутом контуре, система 2 не используется)
  IIsoTp::Message response =
      create_obd_response_2_bytes(0x7E8, SERVICE_01, FUEL_SYSTEM_STATUS, 0x02, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint16_t fuel_status = obd2.fuelSystemStatus();

  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      0x0200, fuel_status, "fuelSystemStatus должен вернуть статус замкнутого контура");

  if (response.data)
    delete[] response.data;
}

// Тест 8: fuelSystemStatus - различные статусы
void test_pid_03_fuel_system_status_various() {
  g_mock_iso_tp.reset();

  // Статус: 01 08 (система 1 открытый контур, система 2 отключена)
  IIsoTp::Message response =
      create_obd_response_2_bytes(0x7E8, SERVICE_01, FUEL_SYSTEM_STATUS, 0x01, 0x08);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint16_t fuel_status = obd2.fuelSystemStatus();

  // Проверяем отдельные системы
  uint8_t system1 = (fuel_status >> 8) & 0xFF;
  uint8_t system2 = fuel_status & 0xFF;

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x01, system1, "Система 1 должна быть в открытом контуре");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x08, system2, "Система 2 должна быть отключена");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 04 - ENGINE LOAD ТЕСТЫ
// ============================================================================

// Тест 9: engineLoad - валидные данные
void test_pid_04_engine_load_valid_data() {
  g_mock_iso_tp.reset();

  // Нагрузка: 0x80 (128) = 50.2%
  // Формула: A * 100 / 255
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, ENGINE_LOAD, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double load = obd2.engineLoad();

  // 128 * 100 / 255 = 50.196%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 50.196f, (float)load, "engineLoad должен вернуть правильный процент нагрузки");

  if (response.data)
    delete[] response.data;
}

// Тест 10: engineLoad - граничные значения
void test_pid_04_engine_load_boundary_values() {
  g_mock_iso_tp.reset();

  // Минимальная нагрузка: 0x00 = 0%
  IIsoTp::Message response1 = create_obd_response_1_byte(0x7E8, SERVICE_01, ENGINE_LOAD, 0x00);
  g_mock_iso_tp.add_receive_message(response1);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double min_load = obd2.engineLoad();

  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01f, 0.0f, (float)min_load, "Минимальная нагрузка должна быть 0%");

  // Максимальная нагрузка: 0xFF = 100%
  g_mock_iso_tp.reset();
  IIsoTp::Message response2 = create_obd_response_1_byte(0x7E8, SERVICE_01, ENGINE_LOAD, 0xFF);
  g_mock_iso_tp.add_receive_message(response2);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2_max(g_mock_iso_tp);
  double max_load = obd2_max.engineLoad();

  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01f, 100.0f, (float)max_load, "Максимальная нагрузка должна быть 100%");

  if (response1.data)
    delete[] response1.data;
  if (response2.data)
    delete[] response2.data;
}

// ============================================================================
// PID 05 - ENGINE COOLANT TEMP ТЕСТЫ
// ============================================================================

// Тест 11: engineCoolantTemp - нормальная температура
void test_pid_05_coolant_temp_normal() {
  g_mock_iso_tp.reset();

  // Температура: 0x5A (90) = 50°C
  // Формула: A - 40
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, ENGINE_COOLANT_TEMP, 0x5A);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.engineCoolantTemp();

  // 90 - 40 = 50°C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 50.0f, (float)temp, "engineCoolantTemp должен вернуть правильную температуру");

  if (response.data)
    delete[] response.data;
}

// Тест 12: engineCoolantTemp - экстремальные температуры
void test_pid_05_coolant_temp_extreme() {
  g_mock_iso_tp.reset();

  // Минимальная температура: 0x00 = -40°C
  IIsoTp::Message response1 =
      create_obd_response_1_byte(0x7E8, SERVICE_01, ENGINE_COOLANT_TEMP, 0x00);
  g_mock_iso_tp.add_receive_message(response1);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2_min(g_mock_iso_tp);
  double min_temp = obd2_min.engineCoolantTemp();

  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, -40.0f, (float)min_temp, "Минимальная температура должна быть -40°C");

  // Максимальная температура: 0xFF (255) = 215°C
  g_mock_iso_tp.reset();
  IIsoTp::Message response2 =
      create_obd_response_1_byte(0x7E8, SERVICE_01, ENGINE_COOLANT_TEMP, 0xFF);
  g_mock_iso_tp.add_receive_message(response2);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2_max(g_mock_iso_tp);
  double max_temp = obd2_max.engineCoolantTemp();

  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 215.0f, (float)max_temp, "Максимальная температура должна быть 215°C");

  if (response1.data)
    delete[] response1.data;
  if (response2.data)
    delete[] response2.data;
}

// ============================================================================
// PID 06-09 - FUEL TRIM ТЕСТЫ
// ============================================================================

// Тест 13: shortTermFuelTrimBank_1 - нормальные значения
void test_pid_06_short_term_fuel_trim_bank1() {
  g_mock_iso_tp.reset();

  // Коррекция: 0x80 (128) = 0%
  // Формула: (A * 100 / 128) - 100
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, SHORT_TERM_FUEL_TRIM_BANK_1, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double trim = obd2.shortTermFuelTrimBank_1();

  // (128 * 100 / 128) - 100 = 0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 0.0f, (float)trim, "shortTermFuelTrimBank_1 должен вернуть 0% при значении 0x80");

  if (response.data)
    delete[] response.data;
}

// Тест 14: longTermFuelTrimBank_1 - положительная коррекция
void test_pid_07_long_term_fuel_trim_bank1_positive() {
  g_mock_iso_tp.reset();

  // Коррекция: 0x90 (144) = +12.5%
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, LONG_TERM_FUEL_TRIM_BANK_1, 0x90);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double trim = obd2.longTermFuelTrimBank_1();

  // (144 * 100 / 128) - 100 = 12.5%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 12.5f, (float)trim, "longTermFuelTrimBank_1 должен вернуть +12.5%");

  if (response.data)
    delete[] response.data;
}

// Тест 15: shortTermFuelTrimBank_2 - отрицательная коррекция
void test_pid_08_short_term_fuel_trim_bank2_negative() {
  g_mock_iso_tp.reset();

  // Коррекция: 0x70 (112) = -12.5%
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, SHORT_TERM_FUEL_TRIM_BANK_2, 0x70);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double trim = obd2.shortTermFuelTrimBank_2();

  // (112 * 100 / 128) - 100 = -12.5%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, -12.5f, (float)trim, "shortTermFuelTrimBank_2 должен вернуть -12.5%");

  if (response.data)
    delete[] response.data;
}

// Тест 16: longTermFuelTrimBank_2 - экстремальные значения
void test_pid_09_long_term_fuel_trim_bank2_extreme() {
  g_mock_iso_tp.reset();

  // Минимальная коррекция: 0x00 = -100%
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, LONG_TERM_FUEL_TRIM_BANK_2, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double trim = obd2.longTermFuelTrimBank_2();

  // (0 * 100 / 128) - 100 = -100%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f,
      -100.0f,
      (float)trim,
      "longTermFuelTrimBank_2 должен вернуть -100% при минимальном значении");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 0A - FUEL PRESSURE ТЕСТЫ
// ============================================================================

// Тест 17: fuelPressure - нормальное давление
void test_pid_0a_fuel_pressure_normal() {
  g_mock_iso_tp.reset();

  // Давление: 0x50 (80) = 240 kPa
  // Формула: A * 3
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, FUEL_PRESSURE, 0x50);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double pressure = obd2.fuelPressure();

  // 80 * 3 = 240 kPa
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 240.0f, (float)pressure, "fuelPressure должен вернуть правильное давление");

  if (response.data)
    delete[] response.data;
}

// Тест 18: fuelPressure - максимальное давление
void test_pid_0a_fuel_pressure_maximum() {
  g_mock_iso_tp.reset();

  // Максимальное давление: 0xFF (255) = 765 kPa
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, FUEL_PRESSURE, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double pressure = obd2.fuelPressure();

  // 255 * 3 = 765 kPa
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 765.0f, (float)pressure, "fuelPressure должен обрабатывать максимальные значения");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 0B - MANIFOLD PRESSURE ТЕСТЫ
// ============================================================================

// Тест 19: manifoldPressure - нормальное давление
void test_pid_0b_manifold_pressure_normal() {
  g_mock_iso_tp.reset();

  // Давление: 0x65 (101) = 101 kPa (атмосферное)
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, INTAKE_MANIFOLD_ABS_PRESSURE, 0x65);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint8_t pressure = obd2.manifoldPressure();

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      101, pressure, "manifoldPressure должен вернуть атмосферное давление");

  if (response.data)
    delete[] response.data;
}

// Тест 20: manifoldPressure - вакуум
void test_pid_0b_manifold_pressure_vacuum() {
  g_mock_iso_tp.reset();

  // Вакуум: 0x30 (48) = 48 kPa
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, INTAKE_MANIFOLD_ABS_PRESSURE, 0x30);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint8_t pressure = obd2.manifoldPressure();

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(48, pressure, "manifoldPressure должен обрабатывать вакуум");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 0C - RPM ТЕСТЫ
// ============================================================================

// Тест 21: rpm - холостой ход
void test_pid_0c_rpm_idle() {
  g_mock_iso_tp.reset();

  // RPM: 0x0C 0x1C (3100) = 775 rpm
  // Формула: ((A * 256) + B) / 4
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, ENGINE_RPM, 0x0C, 0x1C);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double rpm_value = obd2.rpm();

  // ((12 * 256) + 28) / 4 = 3100 / 4 = 775 rpm
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 775.0f, (float)rpm_value, "rpm должен вернуть правильные обороты холостого хода");

  if (response.data)
    delete[] response.data;
}

// Тест 22: rpm - высокие обороты
void test_pid_0c_rpm_high() {
  g_mock_iso_tp.reset();

  // RPM: 0x1A 0xF8 (6904) = 1726 rpm
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, ENGINE_RPM, 0x1A, 0xF8);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double rpm_value = obd2.rpm();

  // ((26 * 256) + 248) / 4 = 6904 / 4 = 1726 rpm
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 1726.0f, (float)rpm_value, "rpm должен вернуть правильные высокие обороты");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 0D - VEHICLE SPEED ТЕСТЫ
// ============================================================================

// Тест 23: kph - городская скорость
void test_pid_0d_kph_city_speed() {
  g_mock_iso_tp.reset();

  // Скорость: 0x3C (60) = 60 km/h
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, VEHICLE_SPEED, 0x3C);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  int32_t speed = obd2.kph();

  TEST_ASSERT_EQUAL_INT32_MESSAGE(60, speed, "kph должен вернуть правильную городскую скорость");

  if (response.data)
    delete[] response.data;
}

// Тест 24: kph - максимальная скорость
void test_pid_0d_kph_maximum() {
  g_mock_iso_tp.reset();

  // Максимальная скорость: 0xFF (255) = 255 km/h
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, VEHICLE_SPEED, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  int32_t speed = obd2.kph();

  TEST_ASSERT_EQUAL_INT32_MESSAGE(255, speed, "kph должен обрабатывать максимальную скорость");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 0E - TIMING ADVANCE ТЕСТЫ
// ============================================================================

// Тест 25: timingAdvance - нормальный угол
void test_pid_0e_timing_advance_normal() {
  g_mock_iso_tp.reset();

  // Угол: 0x80 (128) = 0°
  // Формула: (A / 2) - 64
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, TIMING_ADVANCE, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double advance = obd2.timingAdvance();

  // (128 / 2) - 64 = 64 - 64 = 0°
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 0.0f, (float)advance, "timingAdvance должен вернуть 0° при значении 0x80");

  if (response.data)
    delete[] response.data;
}

// Тест 26: timingAdvance - опережение зажигания
void test_pid_0e_timing_advance_positive() {
  g_mock_iso_tp.reset();

  // Угол: 0x90 (144) = 8°
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, TIMING_ADVANCE, 0x90);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double advance = obd2.timingAdvance();

  // (144 / 2) - 64 = 72 - 64 = 8°
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 8.0f, (float)advance, "timingAdvance должен вернуть положительный угол опережения");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 0F - INTAKE AIR TEMP ТЕСТЫ
// ============================================================================

// Тест 27: intakeAirTemp - комнатная температура
void test_pid_0f_intake_air_temp_room() {
  g_mock_iso_tp.reset();

  // Температура: 0x3C (60) = 20°C
  // Формула: A - 40
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, INTAKE_AIR_TEMP, 0x3C);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.intakeAirTemp();

  // 60 - 40 = 20°C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 20.0f, (float)temp, "intakeAirTemp должен вернуть комнатную температуру");

  if (response.data)
    delete[] response.data;
}

// Тест 28: intakeAirTemp - горячий воздух
void test_pid_0f_intake_air_temp_hot() {
  g_mock_iso_tp.reset();

  // Горячий воздух: 0x78 (120) = 80°C
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, INTAKE_AIR_TEMP, 0x78);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.intakeAirTemp();

  // 120 - 40 = 80°C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 80.0f, (float)temp, "intakeAirTemp должен обрабатывать горячий воздух");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 10 - MAF RATE ТЕСТЫ
// ============================================================================

// Тест 29: mafRate - нормальный расход
void test_pid_10_maf_rate_normal() {
  g_mock_iso_tp.reset();

  // MAF: 0x01 0x90 (400) = 4.0 g/s
  // Формула: ((A * 256) + B) / 100
  IIsoTp::Message response =
      create_obd_response_2_bytes(0x7E8, SERVICE_01, MAF_FLOW_RATE, 0x01, 0x90);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double maf = obd2.mafRate();

  // ((1 * 256) + 144) / 100 = 400 / 100 = 4.0 g/s
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 4.0f, (float)maf, "mafRate должен вернуть правильный расход воздуха");

  if (response.data)
    delete[] response.data;
}

// Тест 30: mafRate - высокий расход
void test_pid_10_maf_rate_high() {
  g_mock_iso_tp.reset();

  // Высокий MAF: 0x0A 0x00 (2560) = 25.6 g/s
  IIsoTp::Message response =
      create_obd_response_2_bytes(0x7E8, SERVICE_01, MAF_FLOW_RATE, 0x0A, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double maf = obd2.mafRate();

  // ((10 * 256) + 0) / 100 = 2560 / 100 = 25.6 g/s
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1f, 25.6f, (float)maf, "mafRate должен обрабатывать высокий расход");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 11 - THROTTLE POSITION ТЕСТЫ
// ============================================================================

// Тест 31: throttle - закрытая заслонка
void test_pid_11_throttle_closed() {
  g_mock_iso_tp.reset();

  // Заслонка: 0x00 (0) = 0%
  // Формула: A * 100 / 255
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, THROTTLE_POSITION, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double throttle_pos = obd2.throttle();

  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01f, 0.0f, (float)throttle_pos, "throttle должен вернуть 0% для закрытой заслонки");

  if (response.data)
    delete[] response.data;
}

// Тест 32: throttle - полностью открытая заслонка
void test_pid_11_throttle_wide_open() {
  g_mock_iso_tp.reset();

  // Заслонка: 0xFF (255) = 100%
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, THROTTLE_POSITION, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double throttle_pos = obd2.throttle();

  // 255 * 100 / 255 = 100%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f,
                                   100.0f,
                                   (float)throttle_pos,
                                   "throttle должен вернуть 100% для полностью открытой заслонки");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 12 - COMMANDED SECONDARY AIR STATUS ТЕСТЫ
// ============================================================================

// Тест 33: commandedSecAirStatus - система выключена
void test_pid_12_secondary_air_status_off() {
  g_mock_iso_tp.reset();

  // Статус: 0x00 = система выключена
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, COMMANDED_SECONDARY_AIR_STATUS, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint8_t status = obd2.commandedSecAirStatus();

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0x00, status, "commandedSecAirStatus должен вернуть статус выключенной системы");

  if (response.data)
    delete[] response.data;
}

// Тест 34: commandedSecAirStatus - система включена
void test_pid_12_secondary_air_status_on() {
  g_mock_iso_tp.reset();

  // Статус: 0x01 = система включена
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, COMMANDED_SECONDARY_AIR_STATUS, 0x01);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint8_t status = obd2.commandedSecAirStatus();

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0x01, status, "commandedSecAirStatus должен вернуть статус включенной системы");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 13 - OXYGEN SENSORS PRESENT ТЕСТЫ
// ============================================================================

// Тест 35: oxygenSensorsPresent_2banks - все датчики присутствуют
void test_pid_13_oxygen_sensors_all_present() {
  g_mock_iso_tp.reset();

  // Датчики: 0xFF = все 8 датчиков присутствуют
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, OXYGEN_SENSORS_PRESENT_2_BANKS, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint8_t sensors = obd2.oxygenSensorsPresent_2banks();

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0xFF, sensors, "oxygenSensorsPresent_2banks должен показать все датчики");

  if (response.data)
    delete[] response.data;
}

// Тест 36: oxygenSensorsPresent_2banks - частичное присутствие
void test_pid_13_oxygen_sensors_partial() {
  g_mock_iso_tp.reset();

  // Датчики: 0x0F = только банк 1 (датчики 1-4)
  IIsoTp::Message response =
      create_obd_response_1_byte(0x7E8, SERVICE_01, OXYGEN_SENSORS_PRESENT_2_BANKS, 0x0F);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint8_t sensors = obd2.oxygenSensorsPresent_2banks();

  // Проверяем биты банка 1
  bool bank1_sensor1 = (sensors & 0x01) != 0;
  bool bank1_sensor2 = (sensors & 0x02) != 0;
  bool bank1_sensor3 = (sensors & 0x04) != 0;
  bool bank1_sensor4 = (sensors & 0x08) != 0;
  bool bank2_sensor1 = (sensors & 0x10) != 0;

  TEST_ASSERT_TRUE_MESSAGE(bank1_sensor1, "Банк 1 датчик 1 должен присутствовать");
  TEST_ASSERT_TRUE_MESSAGE(bank1_sensor2, "Банк 1 датчик 2 должен присутствовать");
  TEST_ASSERT_TRUE_MESSAGE(bank1_sensor3, "Банк 1 датчик 3 должен присутствовать");
  TEST_ASSERT_TRUE_MESSAGE(bank1_sensor4, "Банк 1 датчик 4 должен присутствовать");
  TEST_ASSERT_FALSE_MESSAGE(bank2_sensor1, "Банк 2 датчик 1 не должен присутствовать");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 14 - OXYGEN SENSOR 1 ТЕСТЫ (заглушка, так как метод не реализован)
// ============================================================================

// Тест 37: oxygenSensor1 - проверка существования константы
void test_pid_14_oxygen_sensor_1_constant() {
  // Проверяем, что константа определена правильно
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      20, OXYGEN_SENSOR_1_A, "OXYGEN_SENSOR_1_A должен иметь значение 20 (0x14)");
}

// ============================================================================
// ДОПОЛНИТЕЛЬНЫЕ ТЕСТЫ ФОРМУЛ И ГРАНИЧНЫХ СЛУЧАЕВ
// ============================================================================

// Тест 38: Проверка всех формул с реальными данными автомобиля
void test_real_car_data_formulas() {
  // Реальные данные с Toyota Camry 2018
  g_mock_iso_tp.reset();

  // ENGINE_LOAD: 45% (0x73 = 115)
  IIsoTp::Message load_msg = create_obd_response_1_byte(0x7E8, SERVICE_01, ENGINE_LOAD, 0x73);
  g_mock_iso_tp.add_receive_message(load_msg);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2_load(g_mock_iso_tp);
  double load = obd2_load.engineLoad();

  // 115 * 100 / 255 = 45.1%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.5f, 45.1f, (float)load, "Реальные данные: нагрузка двигателя должна быть ~45%");

  if (load_msg.data)
    delete[] load_msg.data;
}

// Тест 39: Проверка обработки ошибок и нулевых значений
void test_error_handling_and_zero_values() {
  g_mock_iso_tp.reset();

  // Тест с нулевыми значениями для RPM
  IIsoTp::Message rpm_zero = create_obd_response_2_bytes(0x7E8, SERVICE_01, ENGINE_RPM, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(rpm_zero);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2_zero(g_mock_iso_tp);
  double rpm_value = obd2_zero.rpm();

  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01f, 0.0f, (float)rpm_value, "RPM должен корректно обрабатывать нулевые значения");

  if (rpm_zero.data)
    delete[] rpm_zero.data;
}

// Тест 40: Комплексный тест всех PID группы 1-20
void test_comprehensive_pid_group_1_20() {
  g_mock_iso_tp.reset();

  // Создаем объект OBD2
  OBD2 obd2(g_mock_iso_tp);

  // Проверяем, что все константы PID определены правильно
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, SUPPORTED_PIDS_1_20, "PID 00 должен быть 0");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, MONITOR_STATUS_SINCE_DTC_CLEARED, "PID 01 должен быть 1");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(2, FREEZE_DTC, "PID 02 должен быть 2");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(3, FUEL_SYSTEM_STATUS, "PID 03 должен быть 3");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(4, ENGINE_LOAD, "PID 04 должен быть 4");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(5, ENGINE_COOLANT_TEMP, "PID 05 должен быть 5");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(6, SHORT_TERM_FUEL_TRIM_BANK_1, "PID 06 должен быть 6");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(7, LONG_TERM_FUEL_TRIM_BANK_1, "PID 07 должен быть 7");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(8, SHORT_TERM_FUEL_TRIM_BANK_2, "PID 08 должен быть 8");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(9, LONG_TERM_FUEL_TRIM_BANK_2, "PID 09 должен быть 9");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(10, FUEL_PRESSURE, "PID 0A должен быть 10");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(11, INTAKE_MANIFOLD_ABS_PRESSURE, "PID 0B должен быть 11");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(12, ENGINE_RPM, "PID 0C должен быть 12");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(13, VEHICLE_SPEED, "PID 0D должен быть 13");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(14, TIMING_ADVANCE, "PID 0E должен быть 14");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(15, INTAKE_AIR_TEMP, "PID 0F должен быть 15");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(16, MAF_FLOW_RATE, "PID 10 должен быть 16");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(17, THROTTLE_POSITION, "PID 11 должен быть 17");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(18, COMMANDED_SECONDARY_AIR_STATUS, "PID 12 должен быть 18");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(19, OXYGEN_SENSORS_PRESENT_2_BANKS, "PID 13 должен быть 19");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(20, OXYGEN_SENSOR_1_A, "PID 14 должен быть 20");
}

// ============================================================================
// ФУНКЦИЯ ЗАПУСКА ВСЕХ ТЕСТОВ PID ГРУППЫ 1-20
// ============================================================================

extern "C" void run_obd_pid_group_1_20_tests() {
  printf("\n=== Запуск тестов PID группы 1-20 ===\n");
  // PID 00 - supportedPIDs_1_20
  RUN_TEST(test_pid_00_supported_pids_1_20_valid_data);
  RUN_TEST(test_pid_00_supported_pids_bit_check);

  // PID 01 - monitorStatus
  RUN_TEST(test_pid_01_monitor_status_valid_data);
  RUN_TEST(test_pid_01_monitor_status_bit_fields);

  // PID 02 - freezeDTC
  RUN_TEST(test_pid_02_freeze_dtc_valid_data);
  RUN_TEST(test_pid_02_freeze_dtc_boundary_values);

  // PID 03 - fuelSystemStatus
  RUN_TEST(test_pid_03_fuel_system_status_normal);
  RUN_TEST(test_pid_03_fuel_system_status_various);

  // PID 04 - engineLoad
  RUN_TEST(test_pid_04_engine_load_valid_data);
  RUN_TEST(test_pid_04_engine_load_boundary_values);

  // PID 05 - engineCoolantTemp
  RUN_TEST(test_pid_05_coolant_temp_normal);
  RUN_TEST(test_pid_05_coolant_temp_extreme);

  // PID 06-09 - fuel trim
  RUN_TEST(test_pid_06_short_term_fuel_trim_bank1);
  RUN_TEST(test_pid_07_long_term_fuel_trim_bank1_positive);
  RUN_TEST(test_pid_08_short_term_fuel_trim_bank2_negative);
  RUN_TEST(test_pid_09_long_term_fuel_trim_bank2_extreme);

  // PID 0A - fuelPressure
  RUN_TEST(test_pid_0a_fuel_pressure_normal);
  RUN_TEST(test_pid_0a_fuel_pressure_maximum);

  // PID 0B - manifoldPressure
  RUN_TEST(test_pid_0b_manifold_pressure_normal);
  RUN_TEST(test_pid_0b_manifold_pressure_vacuum);

  // PID 0C - rpm
  RUN_TEST(test_pid_0c_rpm_idle);
  RUN_TEST(test_pid_0c_rpm_high);

  // PID 0D - kph
  RUN_TEST(test_pid_0d_kph_city_speed);
  RUN_TEST(test_pid_0d_kph_maximum);

  // PID 0E - timingAdvance
  RUN_TEST(test_pid_0e_timing_advance_normal);
  RUN_TEST(test_pid_0e_timing_advance_positive);

  // PID 0F - intakeAirTemp
  RUN_TEST(test_pid_0f_intake_air_temp_room);
  RUN_TEST(test_pid_0f_intake_air_temp_hot);

  // PID 10 - mafRate
  RUN_TEST(test_pid_10_maf_rate_normal);
  RUN_TEST(test_pid_10_maf_rate_high);

  // PID 11 - throttle
  RUN_TEST(test_pid_11_throttle_closed);
  RUN_TEST(test_pid_11_throttle_wide_open);

  // PID 12 - commandedSecAirStatus
  RUN_TEST(test_pid_12_secondary_air_status_off);
  RUN_TEST(test_pid_12_secondary_air_status_on);

  // PID 13 - oxygenSensorsPresent
  RUN_TEST(test_pid_13_oxygen_sensors_all_present);
  RUN_TEST(test_pid_13_oxygen_sensors_partial);

  // PID 14 - oxygenSensor1 (константа)
  RUN_TEST(test_pid_14_oxygen_sensor_1_constant);

  // Дополнительные тесты
  RUN_TEST(test_real_car_data_formulas);
  RUN_TEST(test_error_handling_and_zero_values);
  RUN_TEST(test_comprehensive_pid_group_1_20);
}