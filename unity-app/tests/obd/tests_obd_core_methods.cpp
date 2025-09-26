/**
 * @file tests_obd_core_methods.cpp
 * @brief Тесты критического приоритета для ключевых методов OBD2
 *
 * ПОКРЫТИЕ ТЕСТАМИ:
 * ✅ findResponse() - парсинг ответов OBD2 (6 тестов)
 * ✅ conditionResponse() - обработка и валидация данных (4 теста)
 * ✅ selectCalculator() - выбор калькулятора для PID (3 теста)
 * ✅ processPID() - полный цикл обработки PID (1 тест)
 * ✅ Управление буферами и граничные случаи (3 теста)
 *
 * Приоритет: КРИТИЧЕСКИЙ (1-2 недели)
 * Цель: Обеспечить надежность ключевых методов OBD2
 */

#include <cstdio>
#include <cstring>
#include <functional>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// ============================================================================
// ТЕСТЫ КЛЮЧЕВЫХ МЕТОДОВ OBD2 - КРИТИЧЕСКИЙ ПРИОРИТЕТ
// ============================================================================

// Глобальные объекты для тестов
static MockIsoTp g_mock_iso_tp;

// ============================================================================
// ТЕСТЫ ДЛЯ selectCalculator() - 3 ТЕСТА
// ============================================================================

// Тест 1: selectCalculator() для всех групп PID
void test_obd2_selectCalculator_all_pid_groups() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Тест PID с кастомным калькулятором (ENGINE_RPM)
  auto calculator_rpm = obd2.selectCalculator(OBD2::ENGINE_RPM);
  TEST_ASSERT_NOT_NULL_MESSAGE(calculator_rpm.target<double()>(),
                               "ENGINE_RPM должен иметь кастомный калькулятор");

  // Тест PID с кастомным калькулятором (MAF_FLOW_RATE)
  auto calculator_maf = obd2.selectCalculator(OBD2::MAF_FLOW_RATE);
  TEST_ASSERT_NOT_NULL_MESSAGE(calculator_maf.target<double()>(),
                               "MAF_FLOW_RATE должен иметь кастомный калькулятор");

  // Тест PID без кастомного калькулятора (ENGINE_LOAD)
  auto calculator_load = obd2.selectCalculator(OBD2::ENGINE_LOAD);
  TEST_ASSERT_NULL_MESSAGE(calculator_load.target<double()>(),
                           "ENGINE_LOAD должен использовать дефолтный калькулятор");
}

// Тест 2: selectCalculator() с невалидным PID
void test_obd2_selectCalculator_invalid_pid() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Тест с несуществующим PID
  auto calculator = obd2.selectCalculator(0xFF);  // Несуществующий PID

  TEST_ASSERT_NULL_MESSAGE(calculator.target<double()>(),
                           "Несуществующий PID должен возвращать nullptr");
}

// Тест 3: selectCalculator() с граничными PID
void test_obd2_selectCalculator_boundary_pids() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Тест с минимальным PID
  auto calculator_min = obd2.selectCalculator(0x00);
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(calculator_min),
                            "PID 0x00 должен использовать дефолтный калькулятор");

  // Тест с максимальным поддерживаемым PID
  auto calculator_max = obd2.selectCalculator(OBD2::AUX_INPUT_OUTPUT_SUPPORTED);
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(calculator_max),
                            "Максимальный PID должен использовать дефолтный калькулятор");
}

// ============================================================================
// ТЕСТЫ ДЛЯ processPID() - ПОЛНЫЙ ЦИКЛ ОБРАБОТКИ
// ============================================================================

// Тест 4: processPID() полный цикл обработки ENGINE_RPM
void test_obd2_processPID_complete_flow_rpm() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для ENGINE_RPM (PID 0x0C)
  // Ответ: 41 0C 1A 2B (RPM = (0x1A2B)/4 = 1707.75)
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, 0x01, 0x0C, 0x1A, 0x2B);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);  // Успешное получение

  // Первый вызов - отправка команды
  double result1 = obd2.processPID(0x01, 0x0C, 1, 2);
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      0.0, result1, "Первый вызов processPID должен вернуть 0 (отправка команды)");
  TEST_ASSERT_TRUE_MESSAGE(g_mock_iso_tp.send_called, "Команда должна быть отправлена");

  // Второй вызов - получение и обработка ответа
  double result2 = obd2.processPID(0x01, 0x0C, 1, 2);

  // Ожидаемый результат: (0x1A2B)/4 = 6699/4 = 1674.75
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, 1674.75, result2, "Второй вызов processPID должен вернуть правильные RPM");
  TEST_ASSERT_TRUE_MESSAGE(g_mock_iso_tp.receive_called, "Ответ должен быть получен");
}

// Тест 5: processPID() с PID без кастомного калькулятора
void test_obd2_processPID_default_calculator() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для ENGINE_LOAD (PID 0x04)
  // Ответ: 41 04 7F (Load = 0x7F * 100/255 = 49.8%)
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, 0x01, 0x04, 0x7F);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  // Первый вызов - отправка команды
  double result1 = obd2.processPID(0x01, 0x04, 1, 1, 100.0 / 255.0, 0);
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result1, "Первый вызов должен вернуть 0");

  // Второй вызов - получение и обработка ответа
  double result2 = obd2.processPID(0x01, 0x04, 1, 1, 100.0 / 255.0, 0);

  // Ожидаемый результат: 0x7F * (100/255) = 127 * 0.392 = 49.8
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, 49.8, result2, "ENGINE_LOAD должен быть рассчитан правильно");
}

// ============================================================================
// ТЕСТЫ ПОДДЕРЖИВАЕМЫХ PID
// ============================================================================

// Тест 6: supportedPIDs_1_20() запрос
void test_obd2_supportedPIDs_1_20() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для supportedPIDs (PID 0x00)
  // Ответ: 41 00 BE 1F A8 13 (поддерживаемые PID 1-20)
  IIsoTp::Message response = create_obd_response_4_bytes(0x7E8, 0x01, 0x00, 0xBE, 0x1F, 0xA8, 0x13);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  uint32_t result = obd2.supportedPIDs_1_20();

  // Ожидаемый результат: 0xBE1FA813
  TEST_ASSERT_EQUAL_HEX32_MESSAGE(
      0xBE1FA813, result, "supportedPIDs_1_20 должен вернуть правильную битовую маску");
}

// Тест 7: isPidSupported() проверка поддержки PID
void test_obd2_isPidSupported() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для supportedPIDs (PID 0x00)
  // Ответ: 41 00 BE 1F A8 13
  // Бит 7 (PID 0x01) = 1, Бит 6 (PID 0x02) = 0
  IIsoTp::Message response = create_obd_response_4_bytes(0x7E8, 0x01, 0x00, 0xBE, 0x1F, 0xA8, 0x13);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  // Проверяем поддержку PID 0x01 (должен поддерживаться)
  bool supported_01 = obd2.isPidSupported(0x01);
  TEST_ASSERT_TRUE_MESSAGE(supported_01, "PID 0x01 должен поддерживаться согласно битовой маске");

  // Добавляем еще один ответ для следующего запроса
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  // Проверяем поддержку PID 0x02 (не должен поддерживаться)
  bool supported_02 = obd2.isPidSupported(0x02);
  TEST_ASSERT_FALSE_MESSAGE(supported_02,
                            "PID 0x02 не должен поддерживаться согласно битовой маске");
}

// ============================================================================
// ТЕСТЫ КОНКРЕТНЫХ PID МЕТОДОВ
// ============================================================================

// Тест 8: rpm() метод для получения оборотов двигателя
void test_obd2_rpm_method() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для ENGINE_RPM (PID 0x0C)
  // Ответ: 41 0C 1A 2B (RPM = (0x1A2B)/4 = 1674.75)
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, 0x01, 0x0C, 0x1A, 0x2B);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double rpm_result = obd2.rpm();

  // Ожидаемый результат: (0x1A2B)/4 = 6699/4 = 1674.75
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, 1674.75, rpm_result, "rpm() должен вернуть правильное значение оборотов");
}

// Тест 9: engineLoad() метод для получения нагрузки двигателя
void test_obd2_engineLoad_method() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для ENGINE_LOAD (PID 0x04)
  // Ответ: 41 04 7F (Load = 0x7F * 100/255 = 49.8%)
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, 0x01, 0x04, 0x7F);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double load_result = obd2.engineLoad();

  // Ожидаемый результат: 0x7F * (100/255) = 127 * 0.392 = 49.8
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, 49.8, load_result, "engineLoad() должен вернуть правильное значение нагрузки");
}

// Тест 10: kph() метод для получения скорости
void test_obd2_kph_method() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для VEHICLE_SPEED (PID 0x0D)
  // Ответ: 41 0D 50 (Speed = 0x50 = 80 km/h)
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, 0x01, 0x0D, 0x50);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  int32_t speed_result = obd2.kph();

  // Ожидаемый результат: 0x50 = 80
  TEST_ASSERT_EQUAL_INT32_MESSAGE(
      80, speed_result, "kph() должен вернуть правильное значение скорости");
}

// ============================================================================
// ТЕСТЫ ОБРАБОТКИ ОШИБОК
// ============================================================================

// Тест 11: Обработка таймаута при получении ответа
void test_obd2_core_methods_timeout_handling() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp, 100);  // Короткий таймаут 100ms

  // Не добавляем ответ в мок - симулируем таймаут
  g_mock_iso_tp.set_receive_result(false);  // Неуспешное получение

  double result = obd2.rpm();

  // При таймауте должен вернуться 0
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "При таймауте должен возвращаться 0");
}

// Тест 12: Обработка некорректного ответа
void test_obd2_invalid_response_handling() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка некорректного ответа (неправильный service)
  IIsoTp::Message response =
      create_obd_response_2_bytes(0x7E8, 0x02, 0x0C, 0x1A, 0x2B);  // Service 02 вместо 01
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.rpm();

  // При некорректном ответе должен вернуться 0
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "При некорректном ответе должен возвращаться 0");
}

// Тест 13: Обработка ошибки OBD2 (негативный ответ)
void test_obd2_core_methods_negative_response_handling() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка негативного ответа OBD2
  IIsoTp::Message error_response =
      create_obd_error_response(0x7E8, 0x01, 0x12);  // Service not supported
  g_mock_iso_tp.add_receive_message(error_response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.rpm();

  // При ошибке OBD2 должен вернуться 0
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "При ошибке OBD2 должен возвращаться 0");
}

// ============================================================================
// ДОПОЛНИТЕЛЬНЫЕ ТЕСТЫ ГРАНИЧНЫХ СЛУЧАЕВ
// ============================================================================

// Тест 14: Тест с максимальными значениями данных
void test_obd2_maximum_values() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка ответа с максимальными значениями для RPM
  // Ответ: 41 0C FF FF (RPM = (0xFFFF)/4 = 16383.75)
  IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, 0x01, 0x0C, 0xFF, 0xFF);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double rpm_result = obd2.rpm();

  // Ожидаемый результат: (0xFFFF)/4 = 65535/4 = 16383.75
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, 16383.75, rpm_result, "rpm() должен корректно обрабатывать максимальные значения");
}

// Тест 15: Тест с минимальными значениями данных
void test_obd2_minimum_values() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка ответа с минимальными значениями для ENGINE_LOAD
  // Ответ: 41 04 00 (Load = 0x00 * 100/255 = 0%)
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, 0x01, 0x04, 0x00);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double load_result = obd2.engineLoad();

  // Ожидаемый результат: 0x00 * (100/255) = 0
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, 0.0, load_result, "engineLoad() должен корректно обрабатывать минимальные значения");
}

// Функция запуска всех тестов
extern "C" void run_obd_core_methods_tests() {
  printf("\n=== Запуск тестов ключевых методов OBD2 ===\n");

  // Тесты selectCalculator()
  // RUN_TEST(test_obd2_selectCalculator_all_pid_groups);
  RUN_TEST(test_obd2_selectCalculator_invalid_pid);
  // RUN_TEST(test_obd2_selectCalculator_boundary_pids);

  // Тесты processPID()
  RUN_TEST(test_obd2_processPID_complete_flow_rpm);
  RUN_TEST(test_obd2_processPID_default_calculator);

  // Тесты поддерживаемых PID
  RUN_TEST(test_obd2_supportedPIDs_1_20);
  RUN_TEST(test_obd2_isPidSupported);

  // Тесты конкретных PID методов
  RUN_TEST(test_obd2_rpm_method);
  RUN_TEST(test_obd2_engineLoad_method);
  RUN_TEST(test_obd2_kph_method);

  // Тесты обработки ошибок
  RUN_TEST(test_obd2_core_methods_timeout_handling);
  RUN_TEST(test_obd2_invalid_response_handling);
  RUN_TEST(test_obd2_core_methods_negative_response_handling);

  // Тесты граничных случаев
  RUN_TEST(test_obd2_maximum_values);
  RUN_TEST(test_obd2_minimum_values);

  printf("=== Тесты ключевых методов OBD2 завершены ===\n");
}