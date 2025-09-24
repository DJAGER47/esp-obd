/**
 * @file tests_obd_services.cpp
 * @brief Тесты критического приоритета для различных сервисов OBD2
 *
 * ПОКРЫТИЕ ТЕСТАМИ:
 * ✅ SERVICE_02 - Freeze frame data (5 тестов)
 * ✅ SERVICE_03 - Diagnostic trouble codes (4 теста)
 * ✅ SERVICE_22 - Read data by identifier (3 теста)
 * ✅ Обработка ошибок неподдерживаемых сервисов (4 теста)
 * ✅ Граничные случаи и валидация (4 теста)
 *
 * Приоритет: КРИТИЧЕСКИЙ (1-2 недели)
 * Цель: Обеспечить надежность различных сервисов OBD2
 */

#include <cstdio>
#include <cstring>
#include <functional>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// ============================================================================
// ТЕСТЫ РАЗЛИЧНЫХ СЕРВИСОВ OBD2 - КРИТИЧЕСКИЙ ПРИОРИТЕТ
// ============================================================================

// Глобальные объекты для тестов
static MockIsoTp g_mock_iso_tp;

// ============================================================================
// ТЕСТЫ SERVICE_02 - FREEZE FRAME DATA (5 ТЕСТОВ)
// ============================================================================

// Тест 1: SERVICE_02 - запрос freeze frame data для ENGINE_RPM
void test_obd2_service02_freeze_frame_rpm() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для SERVICE_02, frame 0, PID 0x0C (ENGINE_RPM)
  // Ответ: 42 0C 00 1A 2B (Service 02, PID 0x0C, frame 0, RPM data)
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 6;
  response.data    = new uint8_t[6];
  response.data[0] = 0x05;  // Длина данных
  response.data[1] = 0x42;  // SERVICE_02 response
  response.data[2] = 0x0C;  // PID ENGINE_RPM
  response.data[3] = 0x00;  // Frame number
  response.data[4] = 0x1A;  // RPM high byte
  response.data[5] = 0x2B;  // RPM low byte

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  // Первый вызов - отправка команды SERVICE_02
  double result1 = obd2.processPID(0x02, 0x0C, 2, 2, 0.25, 0);
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      0.0, result1, "Первый вызов SERVICE_02 должен вернуть 0 (отправка команды)");
  TEST_ASSERT_TRUE_MESSAGE(g_mock_iso_tp.send_called, "Команда SERVICE_02 должна быть отправлена");

  // Второй вызов - получение и обработка ответа
  double result2 = obd2.processPID(0x02, 0x0C, 2, 2, 0.25, 0);

  // Ожидаемый результат: (0x1A2B) * 0.25 = 6699 * 0.25 = 1674.75
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, 1674.75, result2, "SERVICE_02 должен вернуть правильные freeze frame RPM");
}

// Тест 2: SERVICE_02 - запрос freeze frame data для ENGINE_LOAD
void test_obd2_service02_freeze_frame_load() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для SERVICE_02, frame 0, PID 0x04 (ENGINE_LOAD)
  // Ответ: 42 04 00 7F (Service 02, PID 0x04, frame 0, Load = 49.8%)
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 5;
  response.data    = new uint8_t[5];
  response.data[0] = 0x04;  // Длина данных
  response.data[1] = 0x42;  // SERVICE_02 response
  response.data[2] = 0x04;  // PID ENGINE_LOAD
  response.data[3] = 0x00;  // Frame number
  response.data[4] = 0x7F;  // Load value

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  // Первый вызов - отправка команды
  double result1 = obd2.processPID(0x02, 0x04, 2, 1, 100.0 / 255.0, 0);
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result1, "Первый вызов должен вернуть 0");

  // Второй вызов - получение и обработка ответа
  double result2 = obd2.processPID(0x02, 0x04, 2, 1, 100.0 / 255.0, 0);

  // Ожидаемый результат: 0x7F * (100/255) = 127 * 0.392 = 49.8
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, 49.8, result2, "SERVICE_02 ENGINE_LOAD должен быть рассчитан правильно");
}

// Тест 3: SERVICE_02 - запрос с несуществующим frame
void test_obd2_service02_invalid_frame() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка негативного ответа для несуществующего frame
  IIsoTp::Message error_response =
      create_obd_error_response(0x7E8, 0x02, 0x12);  // Service not supported
  g_mock_iso_tp.add_receive_message(error_response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x02, 0x0C, 2, 2, 0.25, 0);

  // При ошибке должен вернуться 0
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      0.0, result, "SERVICE_02 с несуществующим frame должен вернуть 0");
}

// Тест 4: SERVICE_02 - проверка количества доступных freeze frames
void test_obd2_service02_freeze_frame_count() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для PID 0x02 (количество freeze frames)
  // Ответ: 42 02 03 (Service 02, PID 0x02, 3 freeze frames доступно)
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 4;
  response.data    = new uint8_t[4];
  response.data[0] = 0x03;  // Длина данных
  response.data[1] = 0x42;  // SERVICE_02 response
  response.data[2] = 0x02;  // PID для количества freeze frames
  response.data[3] = 0x03;  // 3 freeze frames

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x02, 0x02, 1, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      3.0, result, "SERVICE_02 должен вернуть количество freeze frames");
}

// Тест 5: SERVICE_02 - тест с максимальным frame number
void test_obd2_service02_max_frame_number() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для максимального frame number (255)
  // Ответ: 42 0D FF 50 (Service 02, PID 0x0D, frame 255, Speed = 80 km/h)
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 5;
  response.data    = new uint8_t[5];
  response.data[0] = 0x04;  // Длина данных
  response.data[1] = 0x42;  // SERVICE_02 response
  response.data[2] = 0x0D;  // PID VEHICLE_SPEED
  response.data[3] = 0xFF;  // Frame number 255
  response.data[4] = 0x50;  // Speed value

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x02, 0x0D, 2, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(80.0, result, "SERVICE_02 с максимальным frame должен работать");
}

// ============================================================================
// ТЕСТЫ SERVICE_03 - DIAGNOSTIC TROUBLE CODES (4 ТЕСТА)
// ============================================================================

// Тест 6: SERVICE_03 - запрос количества DTC
void test_obd2_service03_dtc_count() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для SERVICE_03 (количество DTC)
  // Ответ: 43 02 P0301 P0302 (Service 03, 2 DTC: P0301, P0302)
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 7;
  response.data    = new uint8_t[7];
  response.data[0] = 0x06;  // Длина данных
  response.data[1] = 0x43;  // SERVICE_03 response
  response.data[2] = 0x02;  // Количество DTC
  response.data[3] = 0x03;  // P0301 high byte
  response.data[4] = 0x01;  // P0301 low byte
  response.data[5] = 0x03;  // P0302 high byte
  response.data[6] = 0x02;  // P0302 low byte

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x03, 0x00, 1, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(2.0, result, "SERVICE_03 должен вернуть количество DTC");
}

// Тест 7: SERVICE_03 - нет активных DTC
void test_obd2_service03_no_dtc() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для SERVICE_03 без DTC
  // Ответ: 43 00 (Service 03, 0 DTC)
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 3;
  response.data    = new uint8_t[3];
  response.data[0] = 0x02;  // Длина данных
  response.data[1] = 0x43;  // SERVICE_03 response
  response.data[2] = 0x00;  // Количество DTC = 0

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x03, 0x00, 1, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "SERVICE_03 без DTC должен вернуть 0");
}

// Тест 8: SERVICE_03 - максимальное количество DTC
void test_obd2_service03_max_dtc() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для максимального количества DTC
  // Ответ: 43 FF ... (Service 03, 255 DTC - максимум)
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 8;  // Ограничиваем для теста
  response.data    = new uint8_t[8];
  response.data[0] = 0x07;  // Длина данных
  response.data[1] = 0x43;  // SERVICE_03 response
  response.data[2] = 0xFF;  // Количество DTC = 255
  response.data[3] = 0x01;  // Первый DTC
  response.data[4] = 0x00;
  response.data[5] = 0x01;  // Второй DTC
  response.data[6] = 0x01;
  response.data[7] = 0x01;  // Третий DTC (частично)

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x03, 0x00, 1, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      255.0, result, "SERVICE_03 должен обрабатывать максимальное количество DTC");
}

// Тест 9: SERVICE_03 - ошибка при запросе DTC
void test_obd2_service03_error_response() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка негативного ответа
  IIsoTp::Message error_response =
      create_obd_error_response(0x7E8, 0x03, 0x11);  // Service not supported
  g_mock_iso_tp.add_receive_message(error_response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x03, 0x00, 1, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "SERVICE_03 с ошибкой должен вернуть 0");
}

// ============================================================================
// ТЕСТЫ SERVICE_22 - READ DATA BY IDENTIFIER (3 ТЕСТА)
// ============================================================================

// Тест 10: SERVICE_22 - чтение данных по идентификатору
void test_obd2_service22_read_data_by_id() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для SERVICE_22
  // Ответ: 62 F1 90 12 34 56 78 (Service 22, DID F190, data 12345678)
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 8;
  response.data    = new uint8_t[8];
  response.data[0] = 0x07;  // Длина данных
  response.data[1] = 0x62;  // SERVICE_22 response
  response.data[2] = 0xF1;  // DID high byte
  response.data[3] = 0x90;  // DID low byte
  response.data[4] = 0x12;  // Data byte 1
  response.data[5] = 0x34;  // Data byte 2
  response.data[6] = 0x56;  // Data byte 3
  response.data[7] = 0x78;  // Data byte 4

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x22, 0xF1, 3, 4, 1.0, 0);

  // Ожидаемый результат: 0x12345678 = 305419896
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      1.0, 305419896.0, result, "SERVICE_22 должен правильно читать данные по идентификатору");
}

// Тест 11: SERVICE_22 - несуществующий идентификатор
void test_obd2_service22_invalid_identifier() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка негативного ответа для несуществующего DID
  IIsoTp::Message error_response =
      create_obd_error_response(0x7E8, 0x22, 0x31);  // Request out of range
  g_mock_iso_tp.add_receive_message(error_response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x22, 0xFF, 3, 2, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "SERVICE_22 с несуществующим DID должен вернуть 0");
}

// Тест 12: SERVICE_22 - чтение VIN номера
void test_obd2_service22_read_vin() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка мок-ответа для чтения VIN (DID F190)
  // Ответ: 62 F1 90 "1HGBH41JXMN109186" (первые 8 байт)
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 8;
  response.data    = new uint8_t[8];
  response.data[0] = 0x07;  // Длина данных
  response.data[1] = 0x62;  // SERVICE_22 response
  response.data[2] = 0xF1;  // DID high byte
  response.data[3] = 0x90;  // DID low byte
  response.data[4] = '1';   // VIN char 1
  response.data[5] = 'H';   // VIN char 2
  response.data[6] = 'G';   // VIN char 3
  response.data[7] = 'B';   // VIN char 4

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x22, 0xF1, 3, 4, 1.0, 0);

  // Проверяем, что данные получены (не равны 0)
  TEST_ASSERT_NOT_EQUAL_MESSAGE(0.0, result, "SERVICE_22 должен успешно читать VIN");
}

// ============================================================================
// ТЕСТЫ ОБРАБОТКИ ОШИБОК НЕПОДДЕРЖИВАЕМЫХ СЕРВИСОВ (4 ТЕСТА)
// ============================================================================

// Тест 13: Неподдерживаемый сервис 0x04
void test_obd2_unsupported_service_04() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка негативного ответа для неподдерживаемого сервиса
  IIsoTp::Message error_response =
      create_obd_error_response(0x7E8, 0x04, 0x11);  // Service not supported
  g_mock_iso_tp.add_receive_message(error_response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x04, 0x00, 1, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "Неподдерживаемый SERVICE_04 должен вернуть 0");
}

// Тест 14: Неподдерживаемый сервис 0x07
void test_obd2_unsupported_service_07() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка негативного ответа
  IIsoTp::Message error_response =
      create_obd_error_response(0x7E8, 0x07, 0x11);  // Service not supported
  g_mock_iso_tp.add_receive_message(error_response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x07, 0x00, 1, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "Неподдерживаемый SERVICE_07 должен вернуть 0");
}

// Тест 15: Некорректный код сервиса (больше 0x22)
void test_obd2_invalid_service_code() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка негативного ответа для некорректного сервиса
  IIsoTp::Message error_response =
      create_obd_error_response(0x7E8, 0xFF, 0x11);  // Service not supported
  g_mock_iso_tp.add_receive_message(error_response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0xFF, 0x00, 1, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "Некорректный код сервиса должен вернуть 0");
}

// Тест 16: Таймаут при запросе неподдерживаемого сервиса
void test_obd2_unsupported_service_timeout() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp, 100);  // Короткий таймаут

  // Не добавляем ответ - симулируем таймаут
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x05, 0x00, 1, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      0.0, result, "Таймаут неподдерживаемого сервиса должен вернуть 0");
}

// ============================================================================
// ТЕСТЫ ГРАНИЧНЫХ СЛУЧАЕВ И ВАЛИДАЦИИ (4 ТЕСТА)
// ============================================================================

// Тест 17: Валидация длины ответа SERVICE_02
void test_obd2_service02_response_length_validation() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка ответа с некорректной длиной
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 3;  // Слишком короткий ответ для SERVICE_02
  response.data    = new uint8_t[3];
  response.data[0] = 0x02;  // Длина данных
  response.data[1] = 0x42;  // SERVICE_02 response
  response.data[2] = 0x0C;  // PID, но нет данных

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x02, 0x0C, 2, 2, 0.25, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      0.0, result, "SERVICE_02 с некорректной длиной должен вернуть 0");
}

// Тест 18: Валидация длины ответа SERVICE_03
void test_obd2_service03_response_length_validation() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка ответа с некорректной длиной для SERVICE_03
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 2;  // Слишком короткий ответ
  response.data    = new uint8_t[2];
  response.data[0] = 0x01;  // Длина данных
  response.data[1] = 0x43;  // SERVICE_03 response, но нет количества DTC

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x03, 0x00, 1, 1, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      0.0, result, "SERVICE_03 с некорректной длиной должен вернуть 0");
}

// Тест 19: Валидация ответа SERVICE_22 с неправильным DID
void test_obd2_service22_wrong_did_in_response() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Настройка ответа с неправильным DID
  IIsoTp::Message response;
  response.rx_id   = 0x7E8;
  response.len     = 6;
  response.data    = new uint8_t[6];
  response.data[0] = 0x05;  // Длина данных
  response.data[1] = 0x62;  // SERVICE_22 response
  response.data[2] = 0xF2;  // Неправильный DID (запрашивали F1)
  response.data[3] = 0x00;
  response.data[4] = 0x12;  // Data
  response.data[5] = 0x34;

  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x22, 0xF1, 3, 2, 1.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "SERVICE_22 с неправильным DID должен вернуть 0");
}

// Тест 20: Тест с нулевыми параметрами
void test_obd2_services_zero_parameters() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Тест с нулевыми параметрами scale и offset
  IIsoTp::Message response = create_obd_response_1_byte(0x7E8, 0x01, 0x04, 0x80);
  g_mock_iso_tp.add_receive_message(response);
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x01, 0x04, 1, 1, 0.0, 0.0);  // scale = 0

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, "Нулевой scale должен давать результат 0");
}

// Функция запуска всех тестов
extern "C" void run_obd_services_tests() {
  printf("\n=== Запуск тестов различных сервисов OBD2 ===\n");

  // Тесты SERVICE_02 - Freeze frame data
  RUN_TEST(test_obd2_service02_freeze_frame_rpm);
  RUN_TEST(test_obd2_service02_freeze_frame_load);
  RUN_TEST(test_obd2_service02_invalid_frame);
  RUN_TEST(test_obd2_service02_freeze_frame_count);
  RUN_TEST(test_obd2_service02_max_frame_number);

  // Тесты SERVICE_03 - Diagnostic trouble codes
  RUN_TEST(test_obd2_service03_dtc_count);
  RUN_TEST(test_obd2_service03_no_dtc);
  RUN_TEST(test_obd2_service03_max_dtc);
  RUN_TEST(test_obd2_service03_error_response);

  // Тесты SERVICE_22 - Read data by identifier
  RUN_TEST(test_obd2_service22_read_data_by_id);
  RUN_TEST(test_obd2_service22_invalid_identifier);
  RUN_TEST(test_obd2_service22_read_vin);

  // Тесты обработки ошибок неподдерживаемых сервисов
  RUN_TEST(test_obd2_unsupported_service_04);
  RUN_TEST(test_obd2_unsupported_service_07);
  RUN_TEST(test_obd2_invalid_service_code);
  RUN_TEST(test_obd2_unsupported_service_timeout);

  // Тесты граничных случаев и валидации
  RUN_TEST(test_obd2_service02_response_length_validation);
  RUN_TEST(test_obd2_service03_response_length_validation);
  RUN_TEST(test_obd2_service22_wrong_did_in_response);
  RUN_TEST(test_obd2_services_zero_parameters);

  printf("=== Тесты различных сервисов OBD2 завершены ===\n");
}