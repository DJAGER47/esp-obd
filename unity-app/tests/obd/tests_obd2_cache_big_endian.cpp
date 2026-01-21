#include <cstdio>
#include <cstring>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// ============================================================================
// ТЕСТЫ OBD2 CACHE BIG ENDIAN ФОРМАТА
// ============================================================================

/*
 * Тесты для проверки корректности работы с big endian форматом данных PID
 * на основе реальных данных из автомобиля:
 * BE3FB803 = 10111110 00111111 10111000 00000011
 */

// Глобальные объекты для тестов
static MockIsoTp g_mock_iso_tp;

// ============================================================================
// ТЕСТЫ ПРОВЕРКИ BIG ENDIAN ФОРМАТА
// ============================================================================

// Тест 1: Проверка корректности работы с big endian форматом на реальных данных
void test_obd2_cache_big_endian_real_data() {
  g_mock_iso_tp.reset();

  // Реальные данные из автомобиля: BE3FB803
  // Биты: 10111110 00111111 10111000 00000011
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, SUPPORTED_PIDS_1_20, 0xBE, 0x3F, 0xB8, 0x03);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);

  // Проверяем, что кэш правильно инициализируется и работает с big endian форматом
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x01), "PID 0x01 должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x02), "PID 0x02 не должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x03), "PID 0x03 должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x04), "PID 0x04 должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x05), "PID 0x05 должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x06), "PID 0x06 должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x07), "PID 0x07 должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x08), "PID 0x08 не должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x09), "PID 0x09 не должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x0A), "PID 0x0A не должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x0B), "PID 0x0B должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x0C), "PID 0x0C должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x0D), "PID 0x0D должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x0E), "PID 0x0E должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x0F), "PID 0x0F должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x10), "PID 0x10 должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x11), "PID 0x11 должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x12), "PID 0x12 не должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x13), "PID 0x13 должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x14), "PID 0x14 должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x15), "PID 0x15 должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x16), "PID 0x16 не должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x17), "PID 0x17 не должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x18), "PID 0x18 не должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x19), "PID 0x19 не должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x1A), "PID 0x1A не должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x1B), "PID 0x1B не должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x1C), "PID 0x1C не должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x1D), "PID 0x1D не должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x1E), "PID 0x1E не должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x1F), "PID 0x1F должен поддерживаться");
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x20), "PID 0x20 должен поддерживаться");
}

// Тест 2: Проверка корректности работы с кэшем для big endian формата
void test_obd2_cache_big_endian_cache_functionality() {
  g_mock_iso_tp.reset();

  // Реальные данные из автомобиля: BE3FB803
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, SUPPORTED_PIDS_1_20, 0xBE, 0x3F, 0xB8, 0x03);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);

  // Первый вызов должен запросить данные и заполнить кэш
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x01), "PID 0x01 должен поддерживаться");

  // Последующие вызовы должны использовать кэш (без дополнительных запросов)
  TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(0x04), "PID 0x04 должен поддерживаться");
  TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(0x02), "PID 0x02 не должен поддерживаться");

  // Проверяем, что не было дополнительных запросов к ISO-TP
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(4,
                                   g_mock_iso_tp.sent_messages.size(),
                                   "Должен быть только один запрос к ISO-TP для заполнения кэша и 3 без ответа");
}

// Тест 3: Проверка граничных значений для big endian формата
void test_obd2_cache_big_endian_boundary_values() {
  // Настраиваем мок для поддержки всех PID
  setup_mock_for_all_pids(g_mock_iso_tp);

  // Все PID поддерживаются: 0xFFFFFFFF
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, SUPPORTED_PIDS_1_20, 0xFF, 0xFF, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);

  // Проверяем, что все PID поддерживаются
  for (uint8_t pid = 0x01; pid <= 0x20; pid++) {
    char msg[64];
    snprintf(msg, sizeof(msg), "PID 0x%02X должен поддерживаться", pid);
    TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(pid), msg);
  }
}

// Тест 4: Проверка нулевых значений для big endian формата
void test_obd2_cache_big_endian_zero_values() {
  g_mock_iso_tp.reset();

  // Ни один PID не поддерживается: 0x00000000
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, SUPPORTED_PIDS_1_20, 0x00, 0x00, 0x00, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);

  // Проверяем, что ни один PID не поддерживается
  for (uint8_t pid = 0x01; pid <= 0x20; pid++) {
    char msg[64];
    snprintf(msg, sizeof(msg), "PID 0x%02X не должен поддерживаться", pid);
    TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(pid), msg);
  }
}

// Тест 5: Проверка чередующихся битов для big endian формата
void test_obd2_cache_big_endian_alternating_bits() {
  g_mock_iso_tp.reset();

  // Чередующиеся биты: 0xAAAAAAAA (10101010 10101010 10101010 10101010)
  MockMessage response = create_obd_response_4_bytes(0x7E8, SERVICE_01, SUPPORTED_PIDS_1_20, 0xAA, 0xAA, 0xAA, 0xAA);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);

  // Проверяем чередующиеся PID (нечетные поддерживаются, четные нет)
  for (uint8_t pid = 0x01; pid <= 0x20; pid++) {
    char msg[64];
    if (pid % 2 == 1) {
      snprintf(msg, sizeof(msg), "Нечетный PID 0x%02X поддерживается", pid);
      TEST_ASSERT_TRUE_MESSAGE(obd2.IsPidSupported(pid), msg);
    } else {
      snprintf(msg, sizeof(msg), "Четный PID 0x%02X не поддерживается", pid);
      TEST_ASSERT_FALSE_MESSAGE(obd2.IsPidSupported(pid), msg);
    }
  }
}

// ============================================================================
// ФУНКЦИЯ ЗАПУСКА ТЕСТОВ
// ============================================================================

extern "C" void run_obd2_cache_big_endian_tests() {
  UNITY_BEGIN();

  RUN_TEST(test_obd2_cache_big_endian_real_data);
  RUN_TEST(test_obd2_cache_big_endian_cache_functionality);
  RUN_TEST(test_obd2_cache_big_endian_boundary_values);
  RUN_TEST(test_obd2_cache_big_endian_zero_values);
  RUN_TEST(test_obd2_cache_big_endian_alternating_bits);

  UNITY_END();
}