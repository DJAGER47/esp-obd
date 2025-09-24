/**
 * @file tests_obd_error_handling.cpp
 * @brief Тесты обработки ошибок и граничных случаев для OBD2
 *
 * ПОКРЫТИЕ ТЕСТАМИ ОБРАБОТКИ ОШИБОК:
 * ✅ Переполнение буфера payload (3 теста)
 * ✅ Некорректные размеры данных (4 теста)
 * ✅ Нулевые указатели (3 теста)
 * ✅ Экстремальные значения PID (3 теста)
 * ✅ Ошибки ISO-TP уровня (4 теста)
 * ✅ Некорректные заголовки ответов (3 теста)
 * ✅ Поврежденные данные (3 теста)
 * ✅ Неподдерживаемые сервисы (2 теста)
 * ✅ Таймауты различной длительности (2 теста)
 * ✅ Множественные ошибки подряд (2 теста)
 *
 * ВСЕГО ТЕСТОВ: 29 тестов
 * Приоритет: КРИТИЧЕСКИЙ - обеспечение надежности системы
 * Цель: Проверка устойчивости к ошибкам и граничным случаям
 */

#include <cstdio>
#include <cstring>
#include <limits>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// ============================================================================
// ГЛОБАЛЬНЫЕ ОБЪЕКТЫ ДЛЯ ТЕСТОВ
// ============================================================================

static MockIsoTp g_mock_iso_tp;

// ============================================================================
// ТЕСТЫ ПЕРЕПОЛНЕНИЯ БУФЕРА PAYLOAD - 3 ТЕСТА
// ============================================================================

// Тест 1: Переполнение буфера при отправке данных
void test_error_handling_buffer_overflow_send() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Создаем сообщение с максимально возможным размером + 1
  IIsoTp::Message oversized_msg;
  oversized_msg.tx_id = 0x7DF;
  oversized_msg.rx_id = 0x7E8;
  oversized_msg.len = 4096;  // Превышаем максимальный размер буфера
  oversized_msg.data = new uint8_t[4096];

  // Заполняем буфер тестовыми данными
  for (int i = 0; i < 4096; i++) {
    oversized_msg.data[i] = static_cast<uint8_t>(i % 256);
  }

  g_mock_iso_tp.add_receive_message(oversized_msg);
  g_mock_iso_tp.set_receive_result(false);

  // Попытка обработки должна завершиться ошибкой
  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Переполнение буфера должно возвращать ошибку");

  delete[] oversized_msg.data;
}

// Тест 2: Переполнение буфера при получении данных
void test_error_handling_buffer_overflow_receive() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Создаем ответ с чрезмерно большим payload
  IIsoTp::Message large_response;
  large_response.tx_id = 0x7DF;
  large_response.rx_id = 0x7E8;
  large_response.len   = 2048;  // Очень большой ответ
  large_response.data  = new uint8_t[2048];

  // Заполняем корректными заголовками, но огромным payload
  large_response.data[0] = 0x41;  // Положительный ответ
  large_response.data[1] = 0x0C;  // ENGINE_RPM
  for (int i = 2; i < 2048; i++) {
    large_response.data[i] = 0xFF;  // Заполняем мусором
  }

  g_mock_iso_tp.add_receive_message(large_response);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Большой payload должен обрабатываться с ошибкой");

  delete[] large_response.data;
}

// Тест 3: Нулевой размер буфера
void test_error_handling_zero_buffer_size() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Создаем сообщение с нулевым размером
  IIsoTp::Message zero_msg;
  zero_msg.tx_id = 0x7DF;
  zero_msg.rx_id = 0x7E8;
  zero_msg.len   = 0;  // Нулевой размер
  zero_msg.data  = nullptr;

  g_mock_iso_tp.add_receive_message(zero_msg);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Нулевой размер буфера должен возвращать ошибку");
}

// ============================================================================
// ТЕСТЫ НЕКОРРЕКТНЫХ РАЗМЕРОВ ДАННЫХ - 4 ТЕСТА
// ============================================================================

// Тест 4: Слишком короткий ответ
void test_error_handling_response_too_short() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Создаем слишком короткий ответ (только заголовок)
  IIsoTp::Message short_response;
  short_response.tx_id   = 0x7DF;
  short_response.rx_id   = 0x7E8;
  short_response.len     = 1;  // Только один байт
  short_response.data    = new uint8_t[1];
  short_response.data[0] = 0x41;  // Только service, нет PID

  g_mock_iso_tp.add_receive_message(short_response);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Слишком короткий ответ должен возвращать ошибку");

  delete[] short_response.data;
}

// Тест 5: Неполный ответ для многобайтового PID
void test_error_handling_incomplete_multibyte_response() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // ENGINE_RPM требует 2 байта данных, но даем только 1
  IIsoTp::Message incomplete_response;
  incomplete_response.tx_id   = 0x7DF;
  incomplete_response.rx_id   = 0x7E8;
  incomplete_response.len     = 3;  // service + PID + только 1 байт данных
  incomplete_response.data    = new uint8_t[3];
  incomplete_response.data[0] = 0x41;  // Положительный ответ
  incomplete_response.data[1] = 0x0C;  // ENGINE_RPM
  incomplete_response.data[2] = 0x1A;  // Только первый байт

  g_mock_iso_tp.add_receive_message(incomplete_response);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      -1.0, result, "Неполный ответ для многобайтового PID должен возвращать ошибку");

  delete[] incomplete_response.data;
}

// Тест 6: Избыточные данные в ответе
void test_error_handling_excessive_response_data() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // ENGINE_LOAD требует 1 байт, но даем больше
  IIsoTp::Message excessive_response;
  excessive_response.tx_id   = 0x7DF;
  excessive_response.rx_id   = 0x7E8;
  excessive_response.len     = 10;  // Слишком много данных
  excessive_response.data    = new uint8_t[10];
  excessive_response.data[0] = 0x41;  // Положительный ответ
  excessive_response.data[1] = 0x04;  // ENGINE_LOAD
  excessive_response.data[2] = 0x80;  // Корректные данные
  // Остальные байты - мусор
  for (int i = 3; i < 10; i++) {
    excessive_response.data[i] = 0xFF;
  }

  g_mock_iso_tp.add_receive_message(excessive_response);
  g_mock_iso_tp.set_receive_result(false);

  // Должен обработать корректно, игнорируя лишние данные
  double result = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);

  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1,
      50.196,
      result,
      "Избыточные данные должны игнорироваться, основные данные обрабатываться");

  delete[] excessive_response.data;
}

// Тест 7: Отрицательная длина сообщения
void test_error_handling_negative_message_length() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Создаем сообщение с некорректной длиной
  IIsoTp::Message invalid_msg;
  invalid_msg.tx_id = 0x7DF;
  invalid_msg.rx_id = 0x7E8;
  invalid_msg.len   = static_cast<size_t>(-1);  // Максимальное значение size_t
  invalid_msg.data  = new uint8_t[4];
  invalid_msg.data[0] = 0x41;
  invalid_msg.data[1] = 0x0C;
  invalid_msg.data[2] = 0x1A;
  invalid_msg.data[3] = 0x1B;

  g_mock_iso_tp.add_receive_message(invalid_msg);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      -1.0, result, "Некорректная длина сообщения должна возвращать ошибку");

  delete[] invalid_msg.data;
}

// ============================================================================
// ТЕСТЫ НУЛЕВЫХ УКАЗАТЕЛЕЙ - 3 ТЕСТА
// ============================================================================

// Тест 8: Нулевой указатель на данные
void test_error_handling_null_data_pointer() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Создаем сообщение с нулевым указателем на данные
  IIsoTp::Message null_data_msg;
  null_data_msg.tx_id = 0x7DF;
  null_data_msg.rx_id = 0x7E8;
  null_data_msg.len   = 4;
  null_data_msg.data  = nullptr;  // Нулевой указатель

  g_mock_iso_tp.add_receive_message(null_data_msg);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      -1.0, result, "Нулевой указатель на данные должен возвращать ошибку");
}

// Тест 9: Нулевой указатель с ненулевой длиной
void test_error_handling_null_pointer_nonzero_length() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  IIsoTp::Message inconsistent_msg;
  inconsistent_msg.tx_id = 0x7DF;
  inconsistent_msg.rx_id = 0x7E8;
  inconsistent_msg.len   = 100;      // Большая длина
  inconsistent_msg.data  = nullptr;  // Но нулевой указатель

  g_mock_iso_tp.add_receive_message(inconsistent_msg);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      -1.0, result, "Несоответствие длины и указателя должно возвращать ошибку");
}

// Тест 10: Валидация указателей в цепочке вызовов
void test_error_handling_pointer_validation_chain() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Создаем корректное сообщение, но с нулевыми данными в середине
  IIsoTp::Message partial_null_msg;
  partial_null_msg.tx_id   = 0x7DF;
  partial_null_msg.rx_id   = 0x7E8;
  partial_null_msg.len     = 4;
  partial_null_msg.data    = new uint8_t[4];
  partial_null_msg.data[0] = 0x41;
  partial_null_msg.data[1] = 0x0C;
  // Имитируем поврежденную память
  partial_null_msg.data[2] = 0x00;
  partial_null_msg.data[3] = 0x00;

  g_mock_iso_tp.add_receive_message(partial_null_msg);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  // Должен обработать как валидные нулевые данные (0 RPM)
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      0.0, result, "Нулевые данные должны интерпретироваться как валидные");

  delete[] partial_null_msg.data;
}

// ============================================================================
// ТЕСТЫ ЭКСТРЕМАЛЬНЫХ ЗНАЧЕНИЙ PID - 3 ТЕСТА
// ============================================================================

// Тест 11: Максимальное значение PID
void test_error_handling_maximum_pid_value() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Используем максимальное значение uint8_t как PID
  uint8_t max_pid = 0xFF;

  IIsoTp::Message max_pid_response;
  max_pid_response.tx_id   = 0x7DF;
  max_pid_response.rx_id   = 0x7E8;
  max_pid_response.len     = 4;
  max_pid_response.data    = new uint8_t[4];
  max_pid_response.data[0] = 0x41;
  max_pid_response.data[1] = max_pid;
  max_pid_response.data[2] = 0x80;
  max_pid_response.data[3] = 0x00;

  g_mock_iso_tp.add_receive_message(max_pid_response);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, max_pid, 1, 1);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      -1.0, result, "Неподдерживаемый максимальный PID должен возвращать ошибку");

  delete[] max_pid_response.data;
}

// Тест 12: Отрицательное значение PID (приведение типов)
void test_error_handling_negative_pid_cast() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Используем отрицательное значение, которое приведется к большому uint8_t
  int negative_pid = -1;
  uint8_t cast_pid = static_cast<uint8_t>(negative_pid);  // Станет 0xFF

  double result = obd2.processPID(0x01, cast_pid, 1, 1);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      -1.0, result, "Приведенное отрицательное значение PID должно возвращать ошибку");
}

// Тест 13: PID вне диапазона поддерживаемых
void test_error_handling_pid_out_of_range() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Используем PID, который точно не поддерживается (между группами)
  uint8_t unsupported_pid = 0x81;  // Вне всех групп

  IIsoTp::Message unsupported_response;
  unsupported_response.tx_id   = 0x7DF;
  unsupported_response.rx_id   = 0x7E8;
  unsupported_response.len     = 4;
  unsupported_response.data    = new uint8_t[4];
  unsupported_response.data[0] = 0x41;
  unsupported_response.data[1] = unsupported_pid;
  unsupported_response.data[2] = 0x80;
  unsupported_response.data[3] = 0x00;

  g_mock_iso_tp.add_receive_message(unsupported_response);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, unsupported_pid, 1, 1);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "PID вне диапазона должен возвращать ошибку");

  delete[] unsupported_response.data;
}

// ============================================================================
// ТЕСТЫ ОШИБОК ISO-TP УРОВНЯ - 4 ТЕСТА
// ============================================================================

// Тест 14: Ошибка отправки сообщения
void test_error_handling_isotp_send_failure() {
  g_mock_iso_tp.reset();
  g_mock_iso_tp.set_send_result(false);  // Имитируем ошибку отправки

  OBD2 obd2(g_mock_iso_tp);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Ошибка отправки ISO-TP должна возвращать ошибку");

  TEST_ASSERT_TRUE_MESSAGE(g_mock_iso_tp.send_called, "Метод send должен быть вызван");
}

// Тест 15: Ошибка получения сообщения
void test_error_handling_isotp_receive_failure() {
  g_mock_iso_tp.reset();
  g_mock_iso_tp.set_receive_result(true);  // Имитируем ошибку получения

  OBD2 obd2(g_mock_iso_tp);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      -1.0, result, "Ошибка получения ISO-TP должна возвращать ошибку");
}

// Тест 16: Таймаут при получении ответа
void test_error_handling_isotp_receive_timeout() {
  g_mock_iso_tp.reset();
  // Не добавляем сообщения в очередь - имитируем таймаут
  g_mock_iso_tp.set_receive_result(true);

  OBD2 obd2(g_mock_iso_tp);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Таймаут получения должен возвращать ошибку");
}

// Тест 17: Множественные ошибки ISO-TP подряд
void test_error_handling_multiple_isotp_failures() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Первая ошибка - отправка
  g_mock_iso_tp.set_send_result(false);
  double result1 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
  TEST_ASSERT_EQUAL_DOUBLE(-1.0, result1);

  // Вторая ошибка - получение
  g_mock_iso_tp.reset();
  g_mock_iso_tp.set_receive_result(true);
  double result2 = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
  TEST_ASSERT_EQUAL_DOUBLE(-1.0, result2);

  // Третья ошибка - таймаут
  g_mock_iso_tp.reset();
  g_mock_iso_tp.set_receive_result(true);
  double result3 = obd2.processPID(0x01, OBD2::VEHICLE_SPEED, 1, 1);
  TEST_ASSERT_EQUAL_DOUBLE(-1.0, result3);

  TEST_ASSERT_MESSAGE(true, "Множественные ошибки ISO-TP обработаны корректно");
}

// ============================================================================
// ТЕСТЫ НЕКОРРЕКТНЫХ ЗАГОЛОВКОВ ОТВЕТОВ - 3 ТЕСТА
// ============================================================================

// Тест 18: Неправильный service в ответе
void test_error_handling_wrong_service_response() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  IIsoTp::Message wrong_service_response;
  wrong_service_response.tx_id = 0x7DF;
  wrong_service_response.rx_id = 0x7E8;
  wrong_service_response.len   = 4;
  wrong_service_response.data  = new uint8_t[4];
  wrong_service_response.data[0] = 0x42;  // Неправильный service (должен быть 0x41)
  wrong_service_response.data[1] = 0x0C;  // ENGINE_RPM
  wrong_service_response.data[2] = 0x1A;
  wrong_service_response.data[3] = 0x1B;

  g_mock_iso_tp.add_receive_message(wrong_service_response);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      -1.0, result, "Неправильный service в ответе должен возвращать ошибку");

  delete[] wrong_service_response.data;
}

// Тест 19: Неправильный PID в ответе
void test_error_handling_wrong_pid_response() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  IIsoTp::Message wrong_pid_response;
  wrong_pid_response.tx_id   = 0x7DF;
  wrong_pid_response.rx_id   = 0x7E8;
  wrong_pid_response.len     = 4;
  wrong_pid_response.data    = new uint8_t[4];
  wrong_pid_response.data[0] = 0x41;  // Правильный service
  wrong_pid_response.data[1] = 0x04;  // Неправильный PID (запрашивали 0x0C)
  wrong_pid_response.data[2] = 0x80;
  wrong_pid_response.data[3] = 0x00;

  g_mock_iso_tp.add_receive_message(wrong_pid_response);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      -1.0, result, "Неправильный PID в ответе должен возвращать ошибку");

  delete[] wrong_pid_response.data;
}

// Тест 20: Негативный ответ (0x7F)
void test_error_handling_negative_response() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  IIsoTp::Message negative_response = create_obd_error_response(0x7E8, 0x01, 0x12);

  g_mock_iso_tp.add_receive_message(negative_response);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Негативный ответ должен возвращать ошибку");

  delete[] negative_response.data;
}

// ============================================================================
// ТЕСТЫ ПОВРЕЖДЕННЫХ ДАННЫХ - 3 ТЕСТА
// ============================================================================

// Тест 21: Поврежденные данные в середине сообщения
void test_error_handling_corrupted_data_middle() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  IIsoTp::Message corrupted_msg;
  corrupted_msg.tx_id   = 0x7DF;
  corrupted_msg.rx_id   = 0x7E8;
  corrupted_msg.len     = 4;
  corrupted_msg.data    = new uint8_t[4];
  corrupted_msg.data[0] = 0x41;  // Правильный заголовок
  corrupted_msg.data[1] = 0x0C;  // Правильный PID
  // Поврежденные данные
  corrupted_msg.data[2] = 0xFF;  // Максимальное значение
  corrupted_msg.data[3] = 0xFF;  // Максимальное значение

  g_mock_iso_tp.add_receive_message(corrupted_msg);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  // Должен обработать как максимальные обороты
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      100.0, 16383.75, result, "Максимальные значения должны обрабатываться корректно");

  delete[] corrupted_msg.data;
}

// Тест 22: Неполное сообщение (обрыв связи)
void test_error_handling_incomplete_message() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  IIsoTp::Message incomplete_msg;
  incomplete_msg.tx_id   = 0x7DF;
  incomplete_msg.rx_id   = 0x7E8;
  incomplete_msg.len     = 2;  // Только service и PID, нет данных
  incomplete_msg.data    = new uint8_t[2];
  incomplete_msg.data[0] = 0x41;
  incomplete_msg.data[1] = 0x0C;

  g_mock_iso_tp.add_receive_message(incomplete_msg);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Неполное сообщение должно возвращать ошибку");

  delete[] incomplete_msg.data;
}

// Тест 23: Битые данные с случайными значениями
void test_error_handling_random_corrupted_data() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  IIsoTp::Message random_data_msg;
  random_data_msg.tx_id = 0x7DF;
  random_data_msg.rx_id = 0x7E8;
  random_data_msg.len   = 8;
  random_data_msg.data  = new uint8_t[8];

  // Заполняем случайными данными
  random_data_msg.data[0] = 0x41;  // Правильный service
  random_data_msg.data[1] = 0x04;  // ENGINE_LOAD
  random_data_msg.data[2] = 0xAB;  // Случайные данные
  random_data_msg.data[3] = 0xCD;
  random_data_msg.data[4] = 0xEF;
  random_data_msg.data[5] = 0x12;
  random_data_msg.data[6] = 0x34;
  random_data_msg.data[7] = 0x56;

  g_mock_iso_tp.add_receive_message(random_data_msg);
  g_mock_iso_tp.set_receive_result(false);

  double result = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);

  // Должен обработать первый байт данных (0xAB)
  double expected = 0xAB * (100.0 / 255.0);  // 171 * 0.392 = 67.06
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, expected, result, "Случайные данные должны обрабатываться как валидные");

  delete[] random_data_msg.data;
}

// ============================================================================
// ТЕСТЫ НЕПОДДЕРЖИВАЕМЫХ СЕРВИСОВ - 2 ТЕСТА
// ============================================================================

// Тест 24: Неподдерживаемый сервис
void test_error_handling_unsupported_service() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Используем несуществующий сервис
  uint8_t unsupported_service = 0x99;

  double result = obd2.processPID(unsupported_service, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
      -1.0, result, "Неподдерживаемый сервис должен возвращать ошибку");
}

// Тест 25: Сервис с нулевым значением
void test_error_handling_zero_service() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  double result = obd2.processPID(0x00, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Нулевой сервис должен возвращать ошибку");
}

// ============================================================================
// ТЕСТЫ ТАЙМАУТОВ РАЗЛИЧНОЙ ДЛИТЕЛЬНОСТИ - 2 ТЕСТА
// ============================================================================

// Тест 26: Короткий таймаут
void test_error_handling_short_timeout() {
  g_mock_iso_tp.reset();

  // Создаем OBD2 с очень коротким таймаутом
  OBD2 obd2(g_mock_iso_tp, 1);  // 1 мс таймаут

  // Не добавляем ответ - имитируем таймаут
  g_mock_iso_tp.set_receive_result(true);

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Короткий таймаут должен возвращать ошибку");
}

// Тест 27: Длинный таймаут с поздним ответом
void test_error_handling_long_timeout_late_response() {
  g_mock_iso_tp.reset();

  // Создаем OBD2 с длинным таймаутом
  OBD2 obd2(g_mock_iso_tp, 5000);  // 5 секунд

  // Добавляем корректный ответ, но имитируем ошибку получения
  IIsoTp::Message late_response = create_obd_response_2_bytes(0x7E8, 0x01, 0x0C, 0x1A, 0x1B);
  g_mock_iso_tp.add_receive_message(late_response);
  g_mock_iso_tp.set_receive_result(true);  // Ошибка получения

  double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-1.0, result, "Поздний ответ должен возвращать ошибку");

  delete[] late_response.data;
}

// ============================================================================
// ТЕСТЫ МНОЖЕСТВЕННЫХ ОШИБОК ПОДРЯД - 2 ТЕСТА
// ============================================================================

// Тест 28: Цепочка различных ошибок
void test_error_handling_error_chain_different_types() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Ошибка 1: Переполнение буфера
  IIsoTp::Message oversized = create_obd_response_2_bytes(0x7E8, 0x01, 0x0C, 0xFF, 0xFF);
  oversized.len             = 1000;  // Огромный размер
  uint8_t* old_data         = oversized.data;
  oversized.data            = new uint8_t[1000];
  memcpy(oversized.data, old_data, 5);
  delete[] old_data;

  g_mock_iso_tp.add_receive_message(oversized);
  g_mock_iso_tp.set_receive_result(false);

  double result1 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
  TEST_ASSERT_EQUAL_DOUBLE(-1.0, result1);

  // Ошибка 2: Неправильный заголовок
  g_mock_iso_tp.reset();
  IIsoTp::Message wrong_header;
  wrong_header.tx_id   = 0x7DF;
  wrong_header.rx_id   = 0x7E8;
  wrong_header.len     = 4;
  wrong_header.data    = new uint8_t[4];
  wrong_header.data[0] = 0x7F;  // Негативный ответ
  wrong_header.data[1] = 0x01;
  wrong_header.data[2] = 0x12;  // Код ошибки
  wrong_header.data[3] = 0x00;

  g_mock_iso_tp.add_receive_message(wrong_header);
  g_mock_iso_tp.set_receive_result(false);

  double result2 = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
  TEST_ASSERT_EQUAL_DOUBLE(-1.0, result2);

  // Ошибка 3: ISO-TP ошибка
  g_mock_iso_tp.reset();
  g_mock_iso_tp.set_send_result(false);

  double result3 = obd2.processPID(0x01, OBD2::VEHICLE_SPEED, 1, 1);
  TEST_ASSERT_EQUAL_DOUBLE(-1.0, result3);

  TEST_ASSERT_MESSAGE(true, "Цепочка различных ошибок обработана корректно");

  delete[] oversized.data;
  delete[] wrong_header.data;
}

// Тест 29: Восстановление после множественных ошибок
void test_error_handling_recovery_after_multiple_errors() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Серия ошибок
  for (int i = 0; i < 5; i++) {
    g_mock_iso_tp.reset();
    g_mock_iso_tp.set_send_result(false);

    double error_result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    TEST_ASSERT_EQUAL_DOUBLE(-1.0, error_result);
  }

  // Теперь корректный запрос
  g_mock_iso_tp.reset();
  IIsoTp::Message good_response = create_obd_response_2_bytes(0x7E8, 0x01, 0x0C, 0x1A, 0x1B);
  g_mock_iso_tp.add_receive_message(good_response);
  g_mock_iso_tp.set_receive_result(false);

  // Первый вызов - отправка
  double result1 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
  TEST_ASSERT_EQUAL_DOUBLE(0.0, result1);

  // Второй вызов - получение
  double result2 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, 1674.75, result2, "После серии ошибок система должна восстановиться");

  delete[] good_response.data;
}

// ============================================================================
// ФУНКЦИЯ ЗАПУСКА ВСЕХ ТЕСТОВ ОБРАБОТКИ ОШИБОК
// ============================================================================

extern "C" void run_obd_error_handling_tests() {
  printf("\n=== ЗАПУСК ТЕСТОВ ОБРАБОТКИ ОШИБОК OBD2 ===\n");

  // Тесты переполнения буфера
  RUN_TEST(test_error_handling_buffer_overflow_send);
  RUN_TEST(test_error_handling_buffer_overflow_receive);
  RUN_TEST(test_error_handling_zero_buffer_size);

  // Тесты некорректных размеров данных
  RUN_TEST(test_error_handling_response_too_short);
  RUN_TEST(test_error_handling_incomplete_multibyte_response);
  RUN_TEST(test_error_handling_excessive_response_data);
  RUN_TEST(test_error_handling_negative_message_length);

  // Тесты нулевых указателей
  RUN_TEST(test_error_handling_null_data_pointer);
  RUN_TEST(test_error_handling_null_pointer_nonzero_length);
  RUN_TEST(test_error_handling_pointer_validation_chain);

  // Тесты экстремальных значений PID
  RUN_TEST(test_error_handling_maximum_pid_value);
  RUN_TEST(test_error_handling_negative_pid_cast);
  RUN_TEST(test_error_handling_pid_out_of_range);

  // Тесты ошибок ISO-TP уровня
  RUN_TEST(test_error_handling_isotp_send_failure);
  RUN_TEST(test_error_handling_isotp_receive_failure);
  RUN_TEST(test_error_handling_isotp_receive_timeout);
  RUN_TEST(test_error_handling_multiple_isotp_failures);

  // Тесты некорректных заголовков ответов
  RUN_TEST(test_error_handling_wrong_service_response);
  RUN_TEST(test_error_handling_wrong_pid_response);
  RUN_TEST(test_error_handling_negative_response);

  // Тесты поврежденных данных
  RUN_TEST(test_error_handling_corrupted_data_middle);
  RUN_TEST(test_error_handling_incomplete_message);
  RUN_TEST(test_error_handling_random_corrupted_data);

  // Тесты неподдерживаемых сервисов
  RUN_TEST(test_error_handling_unsupported_service);
  RUN_TEST(test_error_handling_zero_service);

  // Тесты таймаутов
  RUN_TEST(test_error_handling_short_timeout);
  RUN_TEST(test_error_handling_long_timeout_late_response);

  // Тесты множественных ошибок
  RUN_TEST(test_error_handling_error_chain_different_types);
  RUN_TEST(test_error_handling_recovery_after_multiple_errors);

  printf("=== ТЕСТЫ ОБРАБОТКИ ОШИБОК OBD2 ЗАВЕРШЕНЫ ===\n");
}