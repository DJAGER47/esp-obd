#include <cstdio>
#include <cstring>

#include "mock_twai_interface.h"
#include "twai_subscriber_iso_tp.h"
#include "unity.h"

// ============================================================================
// ТЕСТЫ ДЛЯ TWAI SUBSCRIBER ISO-TP
// ============================================================================

/*
 * ПОКРЫТИЕ ТЕСТАМИ:
 *
 * ✅ ФУНКЦИОНАЛЬНОСТЬ:
 * - isInterested всегда возвращает true
 * - onTwaiMessage помещает сообщение в очередь
 * - Receive читает из очереди
 * - Обработка ошибок (неинициализированная очередь, переполнение, таймаут)
 */

// Тест 1: Проверка, что isInterested всегда возвращает true
void test_twai_subscriber_iso_tp_is_interested() {
  TwaiSubscriberIsoTp subscriber;

  // Создаем различные фреймы для проверки
  TwaiFrame frame1 = {};
  frame1.id        = 0x123;

  TwaiFrame frame2   = {};
  frame2.id          = 0x456;
  frame2.is_extended = true;

  TwaiFrame frame3 = {};
  frame3.id        = 0x789;
  frame3.is_rtr    = true;

  // Проверяем, что isInterested всегда возвращает true
  TEST_ASSERT_TRUE_MESSAGE(subscriber.isInterested(frame1),
                           "isInterested должен возвращать true для любого фрейма");
  TEST_ASSERT_TRUE_MESSAGE(subscriber.isInterested(frame2),
                           "isInterested должен возвращать true для расширенного фрейма");
  TEST_ASSERT_TRUE_MESSAGE(subscriber.isInterested(frame3),
                           "isInterested должен возвращать true для RTR фрейма");
}

// Тест 2: Проверка, что onTwaiMessage помещает сообщение в очередь
void test_twai_subscriber_iso_tp_on_twai_message() {
  TwaiSubscriberIsoTp subscriber;

  // Создаем тестовый фрейм
  TwaiFrame send_frame   = {};
  send_frame.id          = 0x123;
  send_frame.data_length = 8;
  send_frame.data[0]     = 0x01;
  send_frame.data[1]     = 0x02;
  send_frame.data[2]     = 0x03;
  send_frame.data[3]     = 0x04;

  // Отправляем фрейм
  bool result = subscriber.onTwaiMessage(send_frame);
  TEST_ASSERT_TRUE_MESSAGE(result, "onTwaiMessage должен успешно поместить фрейм в очередь");

  // Получаем фрейм
  TwaiFrame receive_frame = {};
  result                  = subscriber.Receive(receive_frame, 0);

  // Проверяем, что фрейм получен корректно
  TEST_ASSERT_TRUE_MESSAGE(result, "Receive должен успешно получить фрейм из очереди");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(send_frame.id, receive_frame.id, "ID фрейма должен совпадать");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      send_frame.data_length, receive_frame.data_length, "Длина данных должна совпадать");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
      send_frame.data, receive_frame.data, send_frame.data_length, "Данные должны совпадать");
}

// Тест 3: Проверка таймаута при чтении из пустой очереди
void test_twai_subscriber_iso_tp_receive_timeout() {
  TwaiSubscriberIsoTp subscriber;

  // Пытаемся получить фрейм из пустой очереди с нулевым таймаутом
  TwaiFrame frame = {};
  bool result     = subscriber.Receive(frame, 0);

  // Проверяем, что получен таймаут
  TEST_ASSERT_FALSE_MESSAGE(result, "Receive должен вернуть false при таймауте");
}

// Тест 4: Проверка переполнения очереди
void test_twai_subscriber_iso_tp_queue_overflow() {
  // Создаем подписчика с очередью минимального размера
  TwaiSubscriberIsoTp subscriber(1);

  // Создаем тестовые фреймы
  TwaiFrame frame1 = {};
  frame1.id        = 0x123;

  TwaiFrame frame2 = {};
  frame2.id        = 0x456;

  // Отправляем первый фрейм
  bool result1 = subscriber.onTwaiMessage(frame1);
  TEST_ASSERT_TRUE_MESSAGE(result1, "Первый фрейм должен быть успешно помещен в очередь");

  // Отправляем второй фрейм, который должен вызвать переполнение
  bool result2 = subscriber.onTwaiMessage(frame2);
  TEST_ASSERT_FALSE_MESSAGE(result2, "Второй фрейм должен вызвать переполнение очереди");

  // Получаем фрейм из очереди
  TwaiFrame receive_frame = {};
  bool receive_result     = subscriber.Receive(receive_frame, 0);

  // Проверяем, что получен первый фрейм
  TEST_ASSERT_TRUE_MESSAGE(receive_result, "Receive должен успешно получить фрейм из очереди");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      frame1.id, receive_frame.id, "ID фрейма должен совпадать с первым фреймом");
}

// Функция запуска всех тестов
extern "C" void run_twai_subscriber_iso_tp_tests() {
  printf("\n=== Запуск тестов TwaiSubscriberIsoTp ===\n");
  RUN_TEST(test_twai_subscriber_iso_tp_is_interested);
  RUN_TEST(test_twai_subscriber_iso_tp_on_twai_message);
  RUN_TEST(test_twai_subscriber_iso_tp_receive_timeout);
  RUN_TEST(test_twai_subscriber_iso_tp_queue_overflow);
}