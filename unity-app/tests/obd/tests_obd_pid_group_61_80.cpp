#include <cstdio>
#include <cstring>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// ============================================================================
// ТЕСТЫ OBD2 PID ГРУППЫ 61-80 (ВЫСОКИЙ ПРИОРИТЕТ)
// ============================================================================

/*
 * ПОЛНОЕ ПОКРЫТИЕ ТЕСТАМИ PID ГРУППЫ 61-80:
 *
 * ✅ PID 61 - demandedTorque() - требуемый крутящий момент двигателя в %
 * ✅ PID 62 - torque() - фактический крутящий момент двигателя в %
 * ✅ PID 63 - referenceTorque() - эталонный крутящий момент двигателя в Нм
 * ✅ PID 65 - auxSupported() - поддерживаемые вспомогательные входы/выходы
 *
 * ВСЕГО ТЕСТОВ: 15 (по 3-4 теста на каждый PID)
 */

// Глобальные объекты для тестов
static MockIsoTp g_mock_iso_tp;

// ============================================================================
// PID 61 - DRIVER'S DEMANDED ENGINE PERCENT TORQUE ТЕСТЫ
// ============================================================================

// Тест 1: demandedTorque - валидные данные
void test_pid_61_demanded_torque_valid_data() {
  g_mock_iso_tp.reset();

  // Требуемый крутящий момент: 0x80 = 128, формула: A - 125 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x61, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.demandedTorque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "demandedTorque должен вернуть значение");

  // Ожидаемое значение: 128 - 125 = 3.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   3.0,
                                   static_cast<double>(torque.value()),
                                   "demandedTorque должен вернуть правильный крутящий момент");

  if (response.data)
    delete[] response.data;
}

// Тест 2: demandedTorque - отрицательный момент
void test_pid_61_demanded_torque_negative() {
  g_mock_iso_tp.reset();

  // Отрицательный крутящий момент: 0x50 = 80, формула: 80 - 125 = -45%
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x61, 0x50);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.demandedTorque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "demandedTorque должен вернуть значение");

  // Ожидаемое значение: 80 - 125 = -45.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   -45.0,
                                   static_cast<double>(torque.value()),
                                   "demandedTorque должен обрабатывать отрицательный момент");

  if (response.data)
    delete[] response.data;
}

// Тест 3: demandedTorque - максимальный момент
void test_pid_61_demanded_torque_max() {
  g_mock_iso_tp.reset();

  // Максимальный крутящий момент: 0xFF = 255, формула: 255 - 125 = 130%
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x61, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.demandedTorque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "demandedTorque должен вернуть значение");

  // Ожидаемое значение: 255 - 125 = 130.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   130.0,
                                   static_cast<double>(torque.value()),
                                   "demandedTorque должен обрабатывать максимальный момент");

  if (response.data)
    delete[] response.data;
}

// Тест 4: demandedTorque - минимальный момент
void test_pid_61_demanded_torque_min() {
  g_mock_iso_tp.reset();

  // Минимальный крутящий момент: 0x00 = 0, формула: 0 - 125 = -125%
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x61, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.demandedTorque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "demandedTorque должен вернуть значение");

  // Ожидаемое значение: 0 - 125 = -125.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   -125.0,
                                   static_cast<double>(torque.value()),
                                   "demandedTorque должен обрабатывать минимальный момент");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 62 - ACTUAL ENGINE PERCENT TORQUE ТЕСТЫ
// ============================================================================

// Тест 5: torque - валидные данные
void test_pid_62_actual_torque_valid_data() {
  g_mock_iso_tp.reset();

  // Фактический крутящий момент: 0x90 = 144, формула: A - 125 %
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x62, 0x90);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.torque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "torque должен вернуть значение");

  // Ожидаемое значение: 144 - 125 = 19.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   19.0,
                                   static_cast<double>(torque.value()),
                                   "torque должен вернуть правильный фактический момент");

  if (response.data)
    delete[] response.data;
}

// Тест 6: torque - нулевой момент
void test_pid_62_actual_torque_zero() {
  g_mock_iso_tp.reset();

  // Нулевой крутящий момент: 0x7D = 125, формула: 125 - 125 = 0%
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x62, 0x7D);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.torque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "torque должен вернуть значение");

  // Ожидаемое значение: 125 - 125 = 0.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
      0.1, 0.0, static_cast<double>(torque.value()), "torque должен обрабатывать нулевой момент");

  if (response.data)
    delete[] response.data;
}

// Тест 7: torque - отрицательный момент (торможение двигателем)
void test_pid_62_actual_torque_negative() {
  g_mock_iso_tp.reset();

  // Отрицательный крутящий момент: 0x3C = 60, формула: 60 - 125 = -65%
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x62, 0x3C);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.torque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "torque должен вернуть значение");

  // Ожидаемое значение: 60 - 125 = -65.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   -65.0,
                                   static_cast<double>(torque.value()),
                                   "torque должен обрабатывать отрицательный момент (торможение)");

  if (response.data)
    delete[] response.data;
}

// Тест 8: torque - минимальный момент
void test_pid_62_actual_torque_min() {
  g_mock_iso_tp.reset();

  // Минимальный фактический крутящий момент: 0x00 = 0, формула: 0 - 125 = -125%
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, SERVICE_01, 0x62, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.torque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "torque должен вернуть значение");

  // Ожидаемое значение: 0 - 125 = -125.0%
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1,
                                   -125.0,
                                   static_cast<double>(torque.value()),
                                   "torque должен обрабатывать минимальный фактический момент");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 63 - ENGINE REFERENCE TORQUE ТЕСТЫ
// ============================================================================

// Тест 9: referenceTorque - валидные данные
void test_pid_63_reference_torque_valid_data() {
  g_mock_iso_tp.reset();

  // Эталонный крутящий момент: 0x012C = 300 Нм
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x63, 0x01, 0x2C);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.referenceTorque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "referenceTorque должен вернуть значение");

  // Формула: (A*256 + B) Нм
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      300, torque.value(), "referenceTorque должен вернуть правильный эталонный момент");

  if (response.data)
    delete[] response.data;
}

// Тест 10: referenceTorque - высокий момент
void test_pid_63_reference_torque_high() {
  g_mock_iso_tp.reset();

  // Высокий эталонный крутящий момент: 0x1388 = 5000 Нм
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x63, 0x13, 0x88);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.referenceTorque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "referenceTorque должен вернуть значение");

  // Формула: (A*256 + B) Нм
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      5000, torque.value(), "referenceTorque должен обрабатывать высокий момент");

  if (response.data)
    delete[] response.data;
}

// Тест 11: referenceTorque - максимальный момент
void test_pid_63_reference_torque_max() {
  g_mock_iso_tp.reset();

  // Максимальный эталонный крутящий момент: 0xFFFF = 65535 Нм
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x63, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.referenceTorque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "referenceTorque должен вернуть значение");

  // Формула: (A*256 + B) Нм
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      65535, torque.value(), "referenceTorque должен обрабатывать максимальный момент");

  if (response.data)
    delete[] response.data;
}

// Тест 12: referenceTorque - нулевой эталонный момент
void test_pid_63_reference_torque_zero() {
  g_mock_iso_tp.reset();

  // Нулевой эталонный крутящий момент: 0x0000 = 0 Нм
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x63, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto torque = obd2.referenceTorque();
  TEST_ASSERT_TRUE_MESSAGE(torque.has_value(), "referenceTorque должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      0, torque.value(), "referenceTorque должен обрабатывать нулевой эталонный момент");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// PID 65 - AUXILIARY INPUT/OUTPUT SUPPORTED ТЕСТЫ
// ============================================================================

// Тест 13: auxSupported - валидные данные
void test_pid_65_aux_supported_valid_data() {
  g_mock_iso_tp.reset();

  // Поддерживаемые вспомогательные входы/выходы: 0x1234
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x65, 0x12, 0x34);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto aux = obd2.auxSupported();
  TEST_ASSERT_TRUE_MESSAGE(aux.has_value(), "auxSupported должен вернуть значение");

  // Формула: (A*256 + B) - битовая маска
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      0x1234, aux.value(), "auxSupported должен вернуть правильную битовую маску");

  if (response.data)
    delete[] response.data;
}

// Тест 14: auxSupported - все биты установлены
void test_pid_65_aux_supported_all_bits() {
  g_mock_iso_tp.reset();

  // Все биты установлены: 0xFFFF
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x65, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto aux = obd2.auxSupported();
  TEST_ASSERT_TRUE_MESSAGE(aux.has_value(), "auxSupported должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      0xFFFF, aux.value(), "auxSupported должен обрабатывать все установленные биты");

  if (response.data)
    delete[] response.data;
}

// Тест 15: auxSupported - нет поддерживаемых входов/выходов
void test_pid_65_aux_supported_none() {
  g_mock_iso_tp.reset();

  // Нет поддерживаемых входов/выходов: 0x0000
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, SERVICE_01, 0x65, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(false);

  OBD2 obd2(g_mock_iso_tp);
  auto aux = obd2.auxSupported();
  TEST_ASSERT_TRUE_MESSAGE(aux.has_value(), "auxSupported должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      0x0000,
      aux.value(),
      "auxSupported должен обрабатывать отсутствие поддерживаемых входов/выходов");

  if (response.data)
    delete[] response.data;
}

// ============================================================================
// ФУНКЦИЯ ЗАПУСКА ВСЕХ ТЕСТОВ PID ГРУППЫ 61-80
// ============================================================================

extern "C" void run_obd_pid_group_61_80_tests() {
  printf("\n=== Запуск тестов PID группы 61-80 ===\n");

  // PID 61 - Driver's demanded engine percent torque
  RUN_TEST(test_pid_61_demanded_torque_valid_data);
  RUN_TEST(test_pid_61_demanded_torque_negative);
  RUN_TEST(test_pid_61_demanded_torque_max);
  RUN_TEST(test_pid_61_demanded_torque_min);

  // PID 62 - Actual engine percent torque
  RUN_TEST(test_pid_62_actual_torque_valid_data);
  RUN_TEST(test_pid_62_actual_torque_zero);
  RUN_TEST(test_pid_62_actual_torque_negative);
  RUN_TEST(test_pid_62_actual_torque_min);

  // PID 63 - Engine reference torque
  RUN_TEST(test_pid_63_reference_torque_valid_data);
  RUN_TEST(test_pid_63_reference_torque_high);
  RUN_TEST(test_pid_63_reference_torque_max);
  RUN_TEST(test_pid_63_reference_torque_zero);

  // PID 65 - Auxiliary input/output supported
  RUN_TEST(test_pid_65_aux_supported_valid_data);
  RUN_TEST(test_pid_65_aux_supported_all_bits);
  RUN_TEST(test_pid_65_aux_supported_none);

  printf("=== Тесты PID группы 61-80 завершены ===\n");
}