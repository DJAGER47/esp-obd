#include <cstdio>
#include <cstring>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// ============================================================================
// ТЕСТЫ OBD2 PID ГРУППЫ 21-40 (ВЫСОКИЙ ПРИОРИТЕТ)
// ============================================================================

/*
 * ПОЛНОЕ ПОКРЫТИЕ ТЕСТАМИ PID ГРУППЫ 21-40:
 *
 * ✅ PID 21 - distTravelWithMIL() - расстояние с включенной лампой MIL
 * ✅ PID 22 - fuelRailPressure() - давление топливной рампы (относительно вакуума)
 * ✅ PID 23 - fuelRailGuagePressure() - давление топливной рампы (абсолютное)
 * ✅ PID 2C - commandedEGR() - командованная EGR
 * ✅ PID 2D - egrError() - ошибка EGR
 * ✅ PID 2E - commandedEvapPurge() - командованная продувка адсорбера
 * ✅ PID 2F - fuelLevel() - уровень топлива в баке
 * ✅ PID 30 - warmUpsSinceCodesCleared() - количество прогревов с момента очистки кодов
 * ✅ PID 31 - distSinceCodesCleared() - расстояние с момента очистки кодов
 * ✅ PID 32 - evapSysVapPressure() - давление паров системы улавливания паров
 * ✅ PID 33 - absBaroPressure() - абсолютное барометрическое давление
 * ✅ PID 3C - catTempB1S1() - температура катализатора банк 1 датчик 1
 * ✅ PID 3D - catTempB2S1() - температура катализатора банк 2 датчик 1
 * ✅ PID 3E - catTempB1S2() - температура катализатора банк 1 датчик 2
 * ✅ PID 3F - catTempB2S2() - температура катализатора банк 2 датчик 2
 *
 * ВСЕГО ТЕСТОВ: 30 (по 2 теста на каждый PID)
 */

// Глобальные объекты для тестов
static MockIsoTp g_mock_iso_tp;

// ============================================================================
// PID 21 - DISTANCE TRAVELED WITH MIL ON ТЕСТЫ
// ============================================================================

// Тест 1: distTravelWithMIL - валидные данные
void test_pid_21_dist_travel_with_mil_valid_data() {
  g_mock_iso_tp.reset();

  // Расстояние с MIL: 0x1234 = 4660 км
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x21, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint16_t distance = obd2.distTravelWithMIL();

  // Формула: (A*256 + B) км
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      4660, distance, "distTravelWithMIL должен вернуть правильное расстояние");

  if (response.data)
    delete[] response.data;
}

// Тест 2: distTravelWithMIL - граничные значения
void test_pid_21_dist_travel_with_mil_boundary() {
  g_mock_iso_tp.reset();

  // Максимальное значение: 0xFFFF = 65535 км
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x21, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint16_t distance = obd2.distTravelWithMIL();

  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      65535, distance, "distTravelWithMIL должен обрабатывать максимальное значение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 22 - FUEL RAIL PRESSURE ТЕСТЫ
// ============================================================================

// Тест 3: fuelRailPressure - валидные данные
void test_pid_22_fuel_rail_pressure_valid_data() {
  g_mock_iso_tp.reset();

  // Давление: 0x1234 = 4660, формула: (A*256 + B) * 0.079 кПа
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x22, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double pressure = obd2.fuelRailPressure();

  // Ожидаемое значение: 4660 * 0.079 = 368.14 кПа
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 368.14, pressure, "fuelRailPressure должен вернуть правильное давление");

  if (response.data)
    delete[] response.data;
}

// Тест 4: fuelRailPressure - нулевое значение
void test_pid_22_fuel_rail_pressure_zero() {
  g_mock_iso_tp.reset();

  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x22, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double pressure = obd2.fuelRailPressure();

  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 0.0, pressure, "fuelRailPressure должен обрабатывать нулевое значение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 23 - FUEL RAIL GAUGE PRESSURE ТЕСТЫ
// ============================================================================

// Тест 5: fuelRailGuagePressure - валидные данные
void test_pid_23_fuel_rail_gauge_pressure_valid_data() {
  g_mock_iso_tp.reset();

  // Давление: 0x1234 = 4660, формула: (A*256 + B) * 10.0 кПа
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x23, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double pressure = obd2.fuelRailGuagePressure();

  // Ожидаемое значение: 4660 * 10.0 = 46600.0 кПа
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 46600.0, pressure, "fuelRailGuagePressure должен вернуть правильное давление");

  if (response.data)
    delete[] response.data;
}

// Тест 6: fuelRailGuagePressure - граничные значения
void test_pid_23_fuel_rail_gauge_pressure_boundary() {
  g_mock_iso_tp.reset();

  // Максимальное значение: 0xFFFF = 65535
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x23, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double pressure = obd2.fuelRailGuagePressure();

  // Ожидаемое значение: 65535 * 10.0 = 655350.0 кПа
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      1.0, 655350.0, pressure, "fuelRailGuagePressure должен обрабатывать максимальное значение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 2C - COMMANDED EGR ТЕСТЫ
// ============================================================================

// Тест 7: commandedEGR - валидные данные
void test_pid_2c_commanded_egr_valid_data() {
  g_mock_iso_tp.reset();

  // EGR: 0x80 = 128, формула: A * 100/255 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x2C, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double egr = obd2.commandedEGR();

  // Ожидаемое значение: 128 * 100/255 = 50.196%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 50.196, egr, "commandedEGR должен вернуть правильный процент");

  if (response.data)
    delete[] response.data;
}

// Тест 8: commandedEGR - граничные значения
void test_pid_2c_commanded_egr_boundary() {
  g_mock_iso_tp.reset();

  // Максимальное значение: 0xFF = 255
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x2C, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double egr = obd2.commandedEGR();

  // Ожидаемое значение: 255 * 100/255 = 100.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 100.0, egr, "commandedEGR должен обрабатывать максимальное значение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 2D - EGR ERROR ТЕСТЫ
// ============================================================================

// Тест 9: egrError - валидные данные
void test_pid_2d_egr_error_valid_data() {
  g_mock_iso_tp.reset();

  // EGR Error: 0x80 = 128, формула: (A * 100/128) - 100 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x2D, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double error = obd2.egrError();

  // Ожидаемое значение: (128 * 100/128) - 100 = 0.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01, 0.0, error, "egrError должен вернуть правильную ошибку");

  if (response.data)
    delete[] response.data;
}

// Тест 10: egrError - отрицательная ошибка
void test_pid_2d_egr_error_negative() {
  g_mock_iso_tp.reset();

  // EGR Error: 0x40 = 64, формула: (A * 100/128) - 100 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x2D, 0x40);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double error = obd2.egrError();

  // Ожидаемое значение: (64 * 100/128) - 100 = -50.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, -50.0, error, "egrError должен обрабатывать отрицательные значения");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 2E - COMMANDED EVAPORATIVE PURGE ТЕСТЫ
// ============================================================================

// Тест 11: commandedEvapPurge - валидные данные
void test_pid_2e_commanded_evap_purge_valid_data() {
  g_mock_iso_tp.reset();

  // Evap Purge: 0x80 = 128, формула: A * 100/255 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x2E, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double purge = obd2.commandedEvapPurge();

  // Ожидаемое значение: 128 * 100/255 = 50.196%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 50.196, purge, "commandedEvapPurge должен вернуть правильный процент");

  if (response.data)
    delete[] response.data;
}

// Тест 12: commandedEvapPurge - нулевое значение
void test_pid_2e_commanded_evap_purge_zero() {
  g_mock_iso_tp.reset();

  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x2E, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double purge = obd2.commandedEvapPurge();

  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 0.0, purge, "commandedEvapPurge должен обрабатывать нулевое значение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 2F - FUEL TANK LEVEL INPUT ТЕСТЫ
// ============================================================================

// Тест 13: fuelLevel - валидные данные
void test_pid_2f_fuel_level_valid_data() {
  g_mock_iso_tp.reset();

  // Fuel Level: 0x80 = 128, формула: A * 100/255 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x2F, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double level = obd2.fuelLevel();

  // Ожидаемое значение: 128 * 100/255 = 50.196%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.01, 50.196, level, "fuelLevel должен вернуть правильный уровень топлива");

  if (response.data)
    delete[] response.data;
}

// Тест 14: fuelLevel - полный бак
void test_pid_2f_fuel_level_full_tank() {
  g_mock_iso_tp.reset();

  // Полный бак: 0xFF = 255
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x2F, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double level = obd2.fuelLevel();

  // Ожидаемое значение: 255 * 100/255 = 100.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01, 100.0, level, "fuelLevel должен обрабатывать полный бак");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 30 - WARM-UPS SINCE CODES CLEARED ТЕСТЫ
// ============================================================================

// Тест 15: warmUpsSinceCodesCleared - валидные данные
void test_pid_30_warm_ups_since_codes_cleared_valid_data() {
  g_mock_iso_tp.reset();

  // Количество прогревов: 0x15 = 21
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x30, 0x15);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint8_t warmups = obd2.warmUpsSinceCodesCleared();

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      21, warmups, "warmUpsSinceCodesCleared должен вернуть правильное количество прогревов");

  if (response.data)
    delete[] response.data;
}

// Тест 16: warmUpsSinceCodesCleared - максимальное значение
void test_pid_30_warm_ups_since_codes_cleared_max() {
  g_mock_iso_tp.reset();

  // Максимальное значение: 0xFF = 255
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x30, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint8_t warmups = obd2.warmUpsSinceCodesCleared();

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      255, warmups, "warmUpsSinceCodesCleared должен обрабатывать максимальное значение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 31 - DISTANCE TRAVELED SINCE CODES CLEARED ТЕСТЫ
// ============================================================================

// Тест 17: distSinceCodesCleared - валидные данные
void test_pid_31_dist_since_codes_cleared_valid_data() {
  g_mock_iso_tp.reset();

  // Расстояние: 0x1234 = 4660 км
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x31, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint16_t distance = obd2.distSinceCodesCleared();

  // Формула: (A*256 + B) км
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      4660, distance, "distSinceCodesCleared должен вернуть правильное расстояние");

  if (response.data)
    delete[] response.data;
}

// Тест 18: distSinceCodesCleared - нулевое значение
void test_pid_31_dist_since_codes_cleared_zero() {
  g_mock_iso_tp.reset();

  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x31, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint16_t distance = obd2.distSinceCodesCleared();

  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      0, distance, "distSinceCodesCleared должен обрабатывать нулевое значение");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 32 - EVAP SYSTEM VAPOR PRESSURE ТЕСТЫ
// ============================================================================

// Тест 19: evapSysVapPressure - валидные данные
void test_pid_32_evap_sys_vap_pressure_valid_data() {
  g_mock_iso_tp.reset();

  // Давление: 0x1234 = 4660, формула: (A*256 + B) / 4 Па
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x32, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double pressure = obd2.evapSysVapPressure();

  // Ожидаемое значение: 4660 / 4 = 1165.0 Па
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 1165.0, pressure, "evapSysVapPressure должен вернуть правильное давление");

  if (response.data)
    delete[] response.data;
}

// Тест 20: evapSysVapPressure - отрицательное давление
void test_pid_32_evap_sys_vap_pressure_negative() {
  g_mock_iso_tp.reset();

  // Отрицательное давление: 0x8000 = 32768 (знаковое)
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x32, 0x80, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double pressure = obd2.evapSysVapPressure();

  // Ожидаемое значение: 32768 / 4 = 8192.0 Па
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 8192.0, pressure, "evapSysVapPressure должен обрабатывать большие значения");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 33 - ABSOLUTE BAROMETRIC PRESSURE ТЕСТЫ
// ============================================================================

// Тест 21: absBaroPressure - валидные данные
void test_pid_33_abs_baro_pressure_valid_data() {
  g_mock_iso_tp.reset();

  // Барометрическое давление: 0x65 = 101 кПа (нормальное атмосферное давление)
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x33, 0x65);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint8_t pressure = obd2.absBaroPressure();

  // Формула: A кПа
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      101, pressure, "absBaroPressure должен вернуть правильное давление");

  if (response.data)
    delete[] response.data;
}

// Тест 22: absBaroPressure - высокое давление
void test_pid_33_abs_baro_pressure_high() {
  g_mock_iso_tp.reset();

  // Высокое давление: 0xFF = 255 кПа
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, OBD2::SERVICE_01, 0x33, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  uint8_t pressure = obd2.absBaroPressure();

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      255, pressure, "absBaroPressure должен обрабатывать высокое давление");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 3C - CATALYST TEMPERATURE BANK 1 SENSOR 1 ТЕСТЫ
// ============================================================================

// Тест 23: catTempB1S1 - валидные данные
void test_pid_3c_cat_temp_b1s1_valid_data() {
  g_mock_iso_tp.reset();

  // Температура: 0x1234 = 4660, формула: ((A*256 + B) / 10) - 40 °C
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x3C, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.catTempB1S1();

  // Ожидаемое значение: (4660 / 10) - 40 = 426.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 426.0, temp, "catTempB1S1 должен вернуть правильную температуру");

  if (response.data)
    delete[] response.data;
}

// Тест 24: catTempB1S1 - низкая температура
void test_pid_3c_cat_temp_b1s1_low_temp() {
  g_mock_iso_tp.reset();

  // Низкая температура: 0x0190 = 400, формула: (400 / 10) - 40 = 0 °C
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x3C, 0x01, 0x90);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.catTempB1S1();

  // Ожидаемое значение: (400 / 10) - 40 = 0.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 0.0, temp, "catTempB1S1 должен обрабатывать низкую температуру");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 3D - CATALYST TEMPERATURE BANK 2 SENSOR 1 ТЕСТЫ
// ============================================================================

// Тест 25: catTempB2S1 - валидные данные
void test_pid_3d_cat_temp_b2s1_valid_data() {
  g_mock_iso_tp.reset();

  // Температура: 0x1234 = 4660, формула: ((A*256 + B) / 10) - 40 °C
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x3D, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.catTempB2S1();

  // Ожидаемое значение: (4660 / 10) - 40 = 426.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 426.0, temp, "catTempB2S1 должен вернуть правильную температуру");

  if (response.data)
    delete[] response.data;
}

// Тест 26: catTempB2S1 - высокая температура
void test_pid_3d_cat_temp_b2s1_high_temp() {
  g_mock_iso_tp.reset();

  // Высокая температура: 0x2710 = 10000, формула: (10000 / 10) - 40 = 960 °C
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x3D, 0x27, 0x10);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.catTempB2S1();

  // Ожидаемое значение: (10000 / 10) - 40 = 960.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 960.0, temp, "catTempB2S1 должен обрабатывать высокую температуру");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 3E - CATALYST TEMPERATURE BANK 1 SENSOR 2 ТЕСТЫ
// ============================================================================

// Тест 27: catTempB1S2 - валидные данные
void test_pid_3e_cat_temp_b1s2_valid_data() {
  g_mock_iso_tp.reset();

  // Температура: 0x1388 = 5000, формула: ((A*256 + B) / 10) - 40 °C
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x3E, 0x13, 0x88);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.catTempB1S2();

  // Ожидаемое значение: (5000 / 10) - 40 = 460.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 460.0, temp, "catTempB1S2 должен вернуть правильную температуру");

  if (response.data)
    delete[] response.data;
}

// Тест 28: catTempB1S2 - минимальная температура
void test_pid_3e_cat_temp_b1s2_min_temp() {
  g_mock_iso_tp.reset();

  // Минимальная температура: 0x0000 = 0, формула: (0 / 10) - 40 = -40 °C
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x3E, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.catTempB1S2();

  // Ожидаемое значение: (0 / 10) - 40 = -40.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, -40.0, temp, "catTempB1S2 должен обрабатывать минимальную температуру");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 3F - CATALYST TEMPERATURE BANK 2 SENSOR 2 ТЕСТЫ
// ============================================================================

// Тест 29: catTempB2S2 - валидные данные
void test_pid_3f_cat_temp_b2s2_valid_data() {
  g_mock_iso_tp.reset();

  // Температура: 0x1770 = 6000, формула: ((A*256 + B) / 10) - 40 °C
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x3F, 0x17, 0x70);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.catTempB2S2();

  // Ожидаемое значение: (6000 / 10) - 40 = 560.0 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 560.0, temp, "catTempB2S2 должен вернуть правильную температуру");

  if (response.data)
    delete[] response.data;
}

// Тест 30: catTempB2S2 - максимальная температура
void test_pid_3f_cat_temp_b2s2_max_temp() {
  g_mock_iso_tp.reset();

  // Максимальная температура: 0xFFFF = 65535, формула: (65535 / 10) - 40 = 6513.5 °C
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, OBD2::SERVICE_01, 0x3F, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  double temp = obd2.catTempB2S2();

  // Ожидаемое значение: (65535 / 10) - 40 = 6513.5 °C
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 6513.5, temp, "catTempB2S2 должен обрабатывать максимальную температуру");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// ФУНКЦИЯ ЗАПУСКА ВСЕХ ТЕСТОВ PID ГРУППЫ 21-40
// ============================================================================

extern "C" void run_obd_pid_group_21_40_tests() {
  printf("\n=== Запуск тестов PID группы 21-40 ===\n");

  // PID 21 - Distance traveled with MIL on
  RUN_TEST(test_pid_21_dist_travel_with_mil_valid_data);
  RUN_TEST(test_pid_21_dist_travel_with_mil_boundary);

  // PID 22 - Fuel rail pressure
  RUN_TEST(test_pid_22_fuel_rail_pressure_valid_data);
  RUN_TEST(test_pid_22_fuel_rail_pressure_zero);

  // PID 23 - Fuel rail gauge pressure
  RUN_TEST(test_pid_23_fuel_rail_gauge_pressure_valid_data);
  RUN_TEST(test_pid_23_fuel_rail_gauge_pressure_boundary);

  // PID 2C - Commanded EGR
  RUN_TEST(test_pid_2c_commanded_egr_valid_data);
  RUN_TEST(test_pid_2c_commanded_egr_boundary);

  // PID 2D - EGR Error
  RUN_TEST(test_pid_2d_egr_error_valid_data);
  RUN_TEST(test_pid_2d_egr_error_negative);

  // PID 2E - Commanded evaporative purge
  RUN_TEST(test_pid_2e_commanded_evap_purge_valid_data);
  RUN_TEST(test_pid_2e_commanded_evap_purge_zero);

  // PID 2F - Fuel tank level input
  RUN_TEST(test_pid_2f_fuel_level_valid_data);
  RUN_TEST(test_pid_2f_fuel_level_full_tank);

  // PID 30 - Warm-ups since codes cleared
  RUN_TEST(test_pid_30_warm_ups_since_codes_cleared_valid_data);
  RUN_TEST(test_pid_30_warm_ups_since_codes_cleared_max);

  // PID 31 - Distance traveled since codes cleared
  RUN_TEST(test_pid_31_dist_since_codes_cleared_valid_data);
  RUN_TEST(test_pid_31_dist_since_codes_cleared_zero);

  // PID 32 - Evap. system vapor pressure
  RUN_TEST(test_pid_32_evap_sys_vap_pressure_valid_data);
  RUN_TEST(test_pid_32_evap_sys_vap_pressure_negative);

  // PID 33 - Absolute barometric pressure
  RUN_TEST(test_pid_33_abs_baro_pressure_valid_data);
  RUN_TEST(test_pid_33_abs_baro_pressure_high);

  // PID 3C - Catalyst temperature bank 1 sensor 1
  RUN_TEST(test_pid_3c_cat_temp_b1s1_valid_data);
  RUN_TEST(test_pid_3c_cat_temp_b1s1_low_temp);

  // PID 3D - Catalyst temperature bank 2 sensor 1
  RUN_TEST(test_pid_3d_cat_temp_b2s1_valid_data);
  RUN_TEST(test_pid_3d_cat_temp_b2s1_high_temp);

  // PID 3E - Catalyst temperature bank 1 sensor 2
  RUN_TEST(test_pid_3e_cat_temp_b1s2_valid_data);
  RUN_TEST(test_pid_3e_cat_temp_b1s2_min_temp);

  // PID 3F - Catalyst temperature bank 2 sensor 2
  RUN_TEST(test_pid_3f_cat_temp_b2s2_valid_data);
  RUN_TEST(test_pid_3f_cat_temp_b2s2_max_temp);

  printf("=== Тесты PID группы 21-40 завершены ===\n");
}