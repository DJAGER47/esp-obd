#include <cstdio>
#include <cstring>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// Глобальные объекты для тестов
static MockIsoTp g_mock_iso_tp;

// ============================================================================
// PID 80 - SUPPORTED PIDS 81-100 ТЕСТЫ
// ============================================================================

// Тест 16: supportedPIDs81_100 - валидные данные
void test_pid_80_supported_pids_81_100_valid_data() {
  g_mock_iso_tp.reset();

  // Поддерживаемые PID: 0x12345678
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, 0x80, 0x12, 0x34, 0x56, 0x78);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto supported = obd2.supportedPIDs81_100();
  TEST_ASSERT_TRUE_MESSAGE(supported.has_value(), "supportedPIDs81_100 должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0x12345678, supported.value(), "supportedPIDs81_100 должен вернуть правильную битовую маску");
}

// Тест 17: supportedPIDs81_100 - нет поддерживаемых PID
void test_pid_80_supported_pids_81_100_none() {
  g_mock_iso_tp.reset();

  // Нет поддерживаемых PID: 0x00000000
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, 0x80, 0x00, 0x00, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto supported = obd2.supportedPIDs81_100();
  TEST_ASSERT_TRUE_MESSAGE(supported.has_value(), "supportedPIDs81_100 должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0x00000000, supported.value(), "supportedPIDs81_100 должен обрабатывать отсутствие поддерживаемых PID");
}

// ============================================================================
// PID A0 - SUPPORTED PIDS 101-120 ТЕСТЫ
// ============================================================================

// Тест 18: supportedPIDs101_120 - валидные данные
void test_pid_a0_supported_pids_101_120_valid_data() {
  g_mock_iso_tp.reset();

  // Сначала настраиваем поддержку PID 81-100, чтобы система знала, что PID A0 поддерживается
  MockMessage pids_81_100 = create_obd_response_4_bytes(0x7E8, SERVICE_01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(pids_81_100);

  // Поддерживаемые PID: 0xABCDEF01
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, 0xA0, 0xAB, 0xCD, 0xEF, 0x01);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto supported = obd2.supportedPIDs101_120();
  TEST_ASSERT_TRUE_MESSAGE(supported.has_value(), "supportedPIDs101_120 должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0xABCDEF01, supported.value(), "supportedPIDs101_120 должен вернуть правильную битовую маску");
}

// Тест 19: supportedPIDs101_120 - все PID поддерживаются
void test_pid_a0_supported_pids_101_120_all() {
  g_mock_iso_tp.reset();

  // Сначала настраиваем поддержку PID 81-100, чтобы система знала, что PID A0 поддерживается
  MockMessage pids_81_100 = create_obd_response_4_bytes(0x7E8, SERVICE_01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(pids_81_100);

  // Все PID поддерживаются: 0xFFFFFFFF
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, 0xA0, 0xFF, 0xFF, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto supported = obd2.supportedPIDs101_120();
  TEST_ASSERT_TRUE_MESSAGE(supported.has_value(), "supportedPIDs101_120 должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0xFFFFFFFF, supported.value(), "supportedPIDs101_120 должен обрабатывать все поддерживаемые PID");
}

// ============================================================================
// PID C0 - SUPPORTED PIDS 121-140 ТЕСТЫ
// ============================================================================

// Тест 20: supportedPIDs121_140 - валидные данные
void test_pid_c0_supported_pids_121_140_valid_data() {
  g_mock_iso_tp.reset();

  // Поддерживаемые PID: 0x87654321
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, 0xC0, 0x87, 0x65, 0x43, 0x21);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto supported = obd2.supportedPIDs121_140();
  TEST_ASSERT_TRUE_MESSAGE(supported.has_value(), "supportedPIDs121_140 должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0x87654321, supported.value(), "supportedPIDs121_140 должен вернуть правильную битовую маску");
}

// Тест 21: supportedPIDs121_140 - частичная поддержка
void test_pid_c0_supported_pids_121_140_partial() {
  g_mock_iso_tp.reset();

  // Частичная поддержка PID: 0x80000001 (только первый и последний биты)
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, 0xC0, 0x80, 0x00, 0x00, 0x01);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);
  auto supported = obd2.supportedPIDs121_140();
  TEST_ASSERT_TRUE_MESSAGE(supported.has_value(), "supportedPIDs121_140 должен вернуть значение");

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0x80000001, supported.value(), "supportedPIDs121_140 должен обрабатывать частичную поддержку PID");
}

// ============================================================================
// ФУНКЦИЯ ЗАПУСКА ВСЕХ ТЕСТОВ PID ГРУППЫ 61-80
// ============================================================================

extern "C" void run_obd_pid_group_81_xx_tests() {
  printf("\n=== Запуск тестов PID группы 81-xx ===\n");

  // PID 80 - Supported PIDs 81-100
  RUN_TEST(test_pid_80_supported_pids_81_100_valid_data);
  RUN_TEST(test_pid_80_supported_pids_81_100_none);

  // PID A0 - Supported PIDs 101-120
  RUN_TEST(test_pid_a0_supported_pids_101_120_valid_data);
  RUN_TEST(test_pid_a0_supported_pids_101_120_all);

  // PID C0 - Supported PIDs 121-140
  RUN_TEST(test_pid_c0_supported_pids_121_140_valid_data);
  RUN_TEST(test_pid_c0_supported_pids_121_140_partial);

  printf("=== Тесты PID группы 81-xx завершены ===\n");
}