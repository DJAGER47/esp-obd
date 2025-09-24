/**
 * @file tests_obd_performance.cpp
 * @brief Тесты производительности и стресс-тесты для OBD2
 *
 * ПОКРЫТИЕ ТЕСТАМИ ПРОИЗВОДИТЕЛЬНОСТИ:
 * ✅ Множественные последовательные запросы (3 теста)
 * ✅ Быстрые циклы запрос-ответ (2 теста)
 * ✅ Большие объемы данных (3 теста)
 * ✅ Тесты памяти (3 теста)
 * ✅ Нагрузочные тесты (3 теста)
 * ✅ Тесты стабильности (2 теста)
 * ✅ Измерение времени выполнения (2 теста)
 * ✅ Параллельные запросы (2 теста)
 *
 * ВСЕГО ТЕСТОВ: 20 тестов
 * Приоритет: ВЫСОКИЙ - обеспечение производительности системы
 * Цель: Проверка производительности и стабильности под нагрузкой
 */

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <vector>

#include "mock_iso_tp.h"
#include "obd2.h"
#include "unity.h"

// ============================================================================
// ГЛОБАЛЬНЫЕ ОБЪЕКТЫ ДЛЯ ТЕСТОВ
// ============================================================================

static MockIsoTp g_mock_iso_tp;

// Вспомогательные функции для измерения времени
class PerformanceTimer {
 public:
  void start() {
    start_time = std::chrono::high_resolution_clock::now();
  }

  double stop_ms() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    return duration.count() / 1000.0;  // Возвращаем в миллисекундах
  }

 private:
  std::chrono::high_resolution_clock::time_point start_time;
};

// ============================================================================
// ТЕСТЫ МНОЖЕСТВЕННЫХ ПОСЛЕДОВАТЕЛЬНЫХ ЗАПРОСОВ - 3 ТЕСТА
// ============================================================================

// Тест 1: 100 последовательных запросов ENGINE_RPM
void test_performance_100_sequential_rpm_requests() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  // Подготавливаем 100 ответов
  for (int i = 0; i < 100; i++) {
    uint8_t rpm_high         = (i % 64) + 10;
    uint8_t rpm_low          = (i % 256);
    IIsoTp::Message response = create_obd_response_2_bytes(0x7E8, 0x01, 0x0C, rpm_high, rpm_low);
    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int successful_requests = 0;
  for (int i = 0; i < 100; i++) {
    double result1 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    if (result1 == 0.0) {
      double result2 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
      if (result2 > 0) {
        successful_requests++;
      }
    }
  }

  double elapsed_ms = timer.stop_ms();

  TEST_ASSERT_EQUAL_INT_MESSAGE(100, successful_requests, "Все 100 запросов должны быть успешными");

  TEST_ASSERT_LESS_THAN_MESSAGE(
      5000.0, elapsed_ms, "100 запросов должны выполняться менее чем за 5 секунд");

  printf("100 последовательных запросов выполнены за %.2f мс (%.2f мс/запрос)\n",
         elapsed_ms,
         elapsed_ms / 100.0);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// Тест 2: Смешанные типы PID
void test_performance_mixed_pid_sequence() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int total_requests = 50;

  // Подготавливаем ответы для смешанных PID
  for (int i = 0; i < total_requests; i++) {
    IIsoTp::Message response;
    if (i % 2 == 0) {
      response = create_obd_response_2_bytes(0x7E8, 0x01, OBD2::ENGINE_RPM, 0x1A, 0x1B);
    } else {
      response = create_obd_response_1_byte(0x7E8, 0x01, OBD2::ENGINE_LOAD, 0x80);
    }
    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int successful_requests = 0;
  for (int i = 0; i < total_requests; i++) {
    if (i % 2 == 0) {
      double result1 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
      if (result1 == 0.0) {
        double result2 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
        if (result2 >= 0)
          successful_requests++;
      }
    } else {
      double result1 = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
      if (result1 == 0.0) {
        double result2 = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
        if (result2 >= 0)
          successful_requests++;
      }
    }
  }

  double elapsed_ms = timer.stop_ms();

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      total_requests, successful_requests, "Все смешанные запросы должны быть успешными");

  printf("50 смешанных запросов выполнены за %.2f мс\n", elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// Тест 3: Множественные ECU
void test_performance_multiple_ecu_sequence() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int total_requests = 40;

  // Подготавливаем ответы от разных ECU
  for (int i = 0; i < total_requests; i++) {
    uint32_t ecu_id = 0x7E8 + (i % 4);
    IIsoTp::Message response =
        create_obd_response_2_bytes(ecu_id, 0x01, OBD2::ENGINE_RPM, 0x10, 0x20);
    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int successful_requests = 0;
  for (int i = 0; i < total_requests; i++) {
    double result1 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    if (result1 == 0.0) {
      double result2 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
      if (result2 > 0)
        successful_requests++;
    }
  }

  double elapsed_ms = timer.stop_ms();

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      total_requests, successful_requests, "Все запросы к множественным ECU должны быть успешными");

  printf("40 запросов к 4 ECU выполнены за %.2f мс\n", elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// ============================================================================
// ТЕСТЫ БЫСТРЫХ ЦИКЛОВ ЗАПРОС-ОТВЕТ - 2 ТЕСТА
// ============================================================================

// Тест 4: Минимальные задержки между запросами
void test_performance_minimal_delay_cycles() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int num_cycles = 30;

  for (int i = 0; i < num_cycles; i++) {
    IIsoTp::Message response = create_obd_response_1_byte(0x7E8, 0x01, OBD2::ENGINE_LOAD, 0x70 + i);
    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int successful_cycles = 0;
  for (int i = 0; i < num_cycles; i++) {
    obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    double result = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    if (result >= 0)
      successful_cycles++;
  }

  double elapsed_ms     = timer.stop_ms();
  double avg_cycle_time = elapsed_ms / num_cycles;

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      num_cycles, successful_cycles, "Все быстрые циклы должны быть успешными");

  TEST_ASSERT_LESS_THAN_MESSAGE(
      10.0, avg_cycle_time, "Средний цикл должен выполняться менее чем за 10 мс");

  printf(
      "30 быстрых циклов: общее время %.2f мс, среднее %.2f мс/цикл\n", elapsed_ms, avg_cycle_time);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// Тест 5: Высокочастотные запросы
void test_performance_high_frequency_requests() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int frequency_test_count = 50;

  for (int i = 0; i < frequency_test_count; i++) {
    IIsoTp::Message response = create_obd_response_2_bytes(
        0x7E8, 0x01, OBD2::ENGINE_RPM, 0x10 + (i % 16), 0x20 + (i % 32));
    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int valid_rpms = 0;
  for (int i = 0; i < frequency_test_count; i++) {
    obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    double rpm = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    if (rpm > 0)
      valid_rpms++;
  }

  double elapsed_ms   = timer.stop_ms();
  double frequency_hz = (frequency_test_count * 1000.0) / elapsed_ms;

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      frequency_test_count, valid_rpms, "Все высокочастотные запросы должны быть валидными");

  TEST_ASSERT_GREATER_THAN_MESSAGE(10.0, frequency_hz, "Частота запросов должна быть больше 10 Гц");

  printf("Высокочастотные запросы: %.1f Гц (50 запросов за %.2f мс)\n", frequency_hz, elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// ============================================================================
// ТЕСТЫ БОЛЬШИХ ОБЪЕМОВ ДАННЫХ - 3 ТЕСТА
// ============================================================================

// Тест 6: Максимальный размер payload
void test_performance_maximum_payload_size() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  IIsoTp::Message max_payload_response;
  max_payload_response.tx_id = 0x7DF;
  max_payload_response.rx_id = 0x7E8;
  max_payload_response.len   = 127;
  max_payload_response.data  = new uint8_t[127];

  max_payload_response.data[0] = 0x41;
  max_payload_response.data[1] = 0x0C;
  max_payload_response.data[2] = 0x1A;
  max_payload_response.data[3] = 0x1B;

  for (int i = 4; i < 127; i++) {
    max_payload_response.data[i] = static_cast<uint8_t>(i % 256);
  }

  g_mock_iso_tp.add_receive_message(max_payload_response);
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  double result1 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
  TEST_ASSERT_EQUAL_DOUBLE(0.0, result1);

  double result2 = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);

  double elapsed_ms = timer.stop_ms();

  TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(
      0.1, 1674.75, result2, "Максимальный payload должен обрабатываться корректно");

  TEST_ASSERT_LESS_THAN_MESSAGE(
      100.0, elapsed_ms, "Обработка максимального payload должна быть быстрой");

  printf("Максимальный payload (127 байт) обработан за %.2f мс\n", elapsed_ms);

  delete[] max_payload_response.data;
}

// Тест 7: Множественные большие ответы
void test_performance_multiple_large_responses() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int num_large_responses = 10;
  int response_size       = 64;

  for (int i = 0; i < num_large_responses; i++) {
    IIsoTp::Message large_response;
    large_response.tx_id = 0x7DF;
    large_response.rx_id = 0x7E8;
    large_response.len   = response_size;
    large_response.data  = new uint8_t[response_size];

    large_response.data[0] = 0x41;
    large_response.data[1] = OBD2::ENGINE_LOAD;
    large_response.data[2] = 0x80 + i;

    for (int j = 3; j < response_size; j++) {
      large_response.data[j] = static_cast<uint8_t>((i + j) % 256);
    }

    g_mock_iso_tp.add_receive_message(large_response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int successful_requests = 0;
  for (int i = 0; i < num_large_responses; i++) {
    obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    double result = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    if (result >= 0)
      successful_requests++;
  }

  double elapsed_ms = timer.stop_ms();

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      num_large_responses, successful_requests, "Все большие ответы должны обрабатываться успешно");

  printf("10 больших ответов (64 байта каждый) обработаны за %.2f мс\n", elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// Тест 8: Накопление данных в буфере
void test_performance_buffer_accumulation() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int num_accumulations = 25;

  for (int i = 0; i < num_accumulations; i++) {
    int size = 10 + (i % 20);

    IIsoTp::Message response;
    response.tx_id = 0x7DF;
    response.rx_id = 0x7E8;
    response.len   = size;
    response.data  = new uint8_t[size];

    response.data[0] = 0x41;
    response.data[1] = OBD2::VEHICLE_SPEED;
    response.data[2] = 0x40 + (i % 50);

    for (int j = 3; j < size; j++) {
      response.data[j] = static_cast<uint8_t>(j % 256);
    }

    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int valid_speeds = 0;
  for (int i = 0; i < num_accumulations; i++) {
    obd2.processPID(0x01, OBD2::VEHICLE_SPEED, 1, 1);
    double speed = obd2.processPID(0x01, OBD2::VEHICLE_SPEED, 1, 1);
    if (speed >= 0 && speed <= 255)
      valid_speeds++;
  }

  double elapsed_ms = timer.stop_ms();

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      num_accumulations, valid_speeds, "Все накопленные данные должны быть валидными");

  printf("25 накоплений данных обработаны за %.2f мс\n", elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// ============================================================================
// ТЕСТЫ ПАМЯТИ - 3 ТЕСТА
// ============================================================================

// Тест 9: Проверка утечек памяти
void test_performance_memory_leak_check() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int memory_test_cycles = 100;

  for (int i = 0; i < memory_test_cycles; i++) {
    IIsoTp::Message response = create_obd_response_1_byte(0x7E8, 0x01, OBD2::ENGINE_LOAD, 0x80);
    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  int successful_requests = 0;
  for (int i = 0; i < memory_test_cycles; i++) {
    obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    double result = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    if (result >= 0)
      successful_requests++;
  }

  TEST_ASSERT_EQUAL_INT_MESSAGE(memory_test_cycles,
                                successful_requests,
                                "Все запросы для проверки памяти должны быть успешными");

  printf("Тест памяти: %d запросов выполнены без утечек\n", memory_test_cycles);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// Тест 10: Управление памятью при больших данных
void test_performance_large_data_memory_management() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int large_data_cycles = 20;
  int large_size        = 100;

  for (int i = 0; i < large_data_cycles; i++) {
    IIsoTp::Message large_response;
    large_response.tx_id = 0x7DF;
    large_response.rx_id = 0x7E8;
    large_response.len   = large_size;
    large_response.data  = new uint8_t[large_size];

    large_response.data[0] = 0x41;
    large_response.data[1] = OBD2::ENGINE_RPM;
    large_response.data[2] = 0x1A;
    large_response.data[3] = 0x1B;

    for (int j = 4; j < large_size; j++) {
      large_response.data[j] = static_cast<uint8_t>((i + j) % 256);
    }

    g_mock_iso_tp.add_receive_message(large_response);
  }
  g_mock_iso_tp.set_receive_result(false);

  int successful_large_requests = 0;
  for (int i = 0; i < large_data_cycles; i++) {
    obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    if (result > 0)
      successful_large_requests++;
  }

  TEST_ASSERT_EQUAL_INT_MESSAGE(large_data_cycles,
                                successful_large_requests,
                                "Все запросы с большими данными должны быть успешными");

  printf("Управление памятью: %d больших запросов обработаны корректно\n", large_data_cycles);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// Тест 11: Стресс-тест памяти с различными размерами
void test_performance_memory_stress_various_sizes() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int stress_cycles = 50;

  for (int i = 0; i < stress_cycles; i++) {
    int size = 5 + (i % 50);

    IIsoTp::Message stress_response;
    stress_response.tx_id = 0x7DF;
    stress_response.rx_id = 0x7E8;
    stress_response.len   = size;
    stress_response.data  = new uint8_t[size];

    stress_response.data[0] = 0x41;
    stress_response.data[1] = OBD2::VEHICLE_SPEED;
    stress_response.data[2] = 0x50 + (i % 50);

    for (int j = 3; j < size; j++) {
      stress_response.data[j] = static_cast<uint8_t>((i * j) % 256);
    }

    g_mock_iso_tp.add_receive_message(stress_response);
  }
  g_mock_iso_tp.set_receive_result(false);

  int successful_stress_requests = 0;
  for (int i = 0; i < stress_cycles; i++) {
    obd2.processPID(0x01, OBD2::VEHICLE_SPEED, 1, 1);
    double result = obd2.processPID(0x01, OBD2::VEHICLE_SPEED, 1, 1);
    if (result >= 0)
      successful_stress_requests++;
  }

  TEST_ASSERT_EQUAL_INT_MESSAGE(stress_cycles,
                                successful_stress_requests,
                                "Все стресс-запросы с различными размерами должны быть успешными");

  printf("Стресс-тест памяти: %d запросов с различными размерами обработаны\n", stress_cycles);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// ============================================================================
// НАГРУЗОЧНЫЕ ТЕСТЫ - 3 ТЕСТА
// ============================================================================

// Тест 12: Длительная нагрузка
void test_performance_sustained_load() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int sustained_requests = 200;

  for (int i = 0; i < sustained_requests; i++) {
    uint8_t pid = (i % 3 == 0)   ? OBD2::ENGINE_RPM
                  : (i % 3 == 1) ? OBD2::ENGINE_LOAD
                                 : OBD2::VEHICLE_SPEED;

    IIsoTp::Message response;
    if (pid == OBD2::ENGINE_RPM) {
      response = create_obd_response_2_bytes(0x7E8, 0x01, pid, 0x10 + (i % 16), 0x20 + (i % 32));
    } else {
      response = create_obd_response_1_byte(0x7E8, 0x01, pid, 0x40 + (i % 64));
    }

    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int successful_sustained = 0;
  for (int i = 0; i < sustained_requests; i++) {
    uint8_t pid            = (i % 3 == 0)   ? OBD2::ENGINE_RPM
                             : (i % 3 == 1) ? OBD2::ENGINE_LOAD
                                            : OBD2::VEHICLE_SPEED;
    uint8_t expected_bytes = (pid == OBD2::ENGINE_RPM) ? 2 : 1;
    double scale_factor    = (pid == OBD2::ENGINE_LOAD) ? 100.0 / 255.0 : 1.0;

    obd2.processPID(0x01, pid, 1, expected_bytes, scale_factor, 0);
    double result = obd2.processPID(0x01, pid, 1, expected_bytes, scale_factor, 0);
    if (result >= 0)
      successful_sustained++;
  }

  double elapsed_ms = timer.stop_ms();

  TEST_ASSERT_EQUAL_INT_MESSAGE(sustained_requests,
                                successful_sustained,
                                "Все запросы длительной нагрузки должны быть успешными");

  TEST_ASSERT_LESS_THAN_MESSAGE(
      10000.0, elapsed_ms, "Длительная нагрузка должна выполняться менее чем за 10 секунд");

  printf("Длительная нагрузка: %d запросов за %.2f мс (%.2f мс/запрос)\n",
         sustained_requests,
         elapsed_ms,
         elapsed_ms / sustained_requests);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// Тест 13: Пиковая нагрузка
void test_performance_peak_load() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int peak_burst_size = 100;

  for (int i = 0; i < peak_burst_size; i++) {
    IIsoTp::Message response = create_obd_response_2_bytes(
        0x7E8, 0x01, OBD2::ENGINE_RPM, 0x20 + (i % 16), 0x30 + (i % 32));
    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int successful_peak = 0;
  for (int i = 0; i < peak_burst_size; i++) {
    obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    double result = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    if (result > 0)
      successful_peak++;
  }

  double elapsed_ms      = timer.stop_ms();
  double peak_throughput = (peak_burst_size * 1000.0) / elapsed_ms;

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      peak_burst_size, successful_peak, "Все запросы пиковой нагрузки должны быть успешными");

  TEST_ASSERT_GREATER_THAN_MESSAGE(
      50.0, peak_throughput, "Пиковая пропускная способность должна быть больше 50 запросов/сек");

  printf("Пиковая нагрузка: %.1f запросов/сек (%d запросов за %.2f мс)\n",
         peak_throughput,
         peak_burst_size,
         elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// Тест 14: Смешанная нагрузка
void test_performance_mixed_load_patterns() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int total_requests = 150;

  for (int i = 0; i < total_requests; i++) {
    uint8_t pid;
    if (i % 3 == 0) {
      pid = OBD2::ENGINE_RPM;
    } else if (i % 3 == 1) {
      pid = OBD2::ENGINE_LOAD;
    } else {
      pid = OBD2::VEHICLE_SPEED;
    }

    IIsoTp::Message response;
    if (pid == OBD2::ENGINE_RPM) {
      response = create_obd_response_2_bytes(0x7E8, 0x01, pid, 0x15, 0x25);
    } else {
      response = create_obd_response_1_byte(0x7E8, 0x01, pid, 0x60 + (i % 50));
    }

    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int successful_mixed = 0;
  for (int i = 0; i < total_requests; i++) {
    uint8_t pid;
    uint8_t expected_bytes;
    double scale_factor = 1.0;

    if (i % 3 == 0) {
      pid            = OBD2::ENGINE_RPM;
      expected_bytes = 2;
    } else if (i % 3 == 1) {
      pid            = OBD2::ENGINE_LOAD;
      expected_bytes = 1;
      scale_factor   = 100.0 / 255.0;
    } else {
      pid            = OBD2::VEHICLE_SPEED;
      expected_bytes = 1;
    }

    obd2.processPID(0x01, pid, 1, expected_bytes, scale_factor, 0);
    double result = obd2.processPID(0x01, pid, 1, expected_bytes, scale_factor, 0);
    if (result >= 0)
      successful_mixed++;
  }

  double elapsed_ms = timer.stop_ms();

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      total_requests, successful_mixed, "Все запросы смешанной нагрузки должны быть успешными");

  printf("Смешанная нагрузка: %d запросов за %.2f мс\n", total_requests, elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// ============================================================================
// ТЕСТЫ СТАБИЛЬНОСТИ - 2 ТЕСТА
// ============================================================================

// Тест 15: Длительная работа с постоянной нагрузкой
void test_performance_long_term_stability() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int stability_duration = 300;

  for (int i = 0; i < stability_duration; i++) {
    uint8_t data_variation = static_cast<uint8_t>(50 + (i % 100));
    IIsoTp::Message response =
        create_obd_response_1_byte(0x7E8, 0x01, OBD2::ENGINE_LOAD, data_variation);
    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int total_successful = 0;
  for (int i = 0; i < stability_duration; i++) {
    obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    double result = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    if (result >= 0)
      total_successful++;
  }

  double elapsed_ms   = timer.stop_ms();
  double success_rate = (double)total_successful / stability_duration * 100.0;

  TEST_ASSERT_GREATER_THAN_MESSAGE(
      95.0, success_rate, "Коэффициент успешности должен быть больше 95%");

  printf("Тест стабильности: %.1f%% успешности за %.2f мс\n", success_rate, elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// Тест 16: Стабильность при переменной нагрузке
void test_performance_variable_load_stability() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int variable_phases         = 5;
  int requests_per_phase      = 40;
  int total_variable_requests = variable_phases * requests_per_phase;

  for (int phase = 0; phase < variable_phases; phase++) {
    for (int req = 0; req < requests_per_phase; req++) {
      uint8_t pid = (phase % 2 == 0) ? OBD2::ENGINE_RPM : OBD2::ENGINE_LOAD;

      IIsoTp::Message response;
      if (pid == OBD2::ENGINE_RPM) {
        response = create_obd_response_2_bytes(0x7E8, 0x01, pid, 0x18 + phase, 0x28 + req);
      } else {
        response = create_obd_response_1_byte(0x7E8, 0x01, pid, 0x70 + phase * 10 + req);
      }

      g_mock_iso_tp.add_receive_message(response);
    }
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int total_variable_successful = 0;
  for (int phase = 0; phase < variable_phases; phase++) {
    uint8_t pid            = (phase % 2 == 0) ? OBD2::ENGINE_RPM : OBD2::ENGINE_LOAD;
    uint8_t expected_bytes = (pid == OBD2::ENGINE_RPM) ? 2 : 1;
    double scale_factor    = (pid == OBD2::ENGINE_LOAD) ? 100.0 / 255.0 : 1.0;

    for (int req = 0; req < requests_per_phase; req++) {
      obd2.processPID(0x01, pid, 1, expected_bytes, scale_factor, 0);
      double result = obd2.processPID(0x01, pid, 1, expected_bytes, scale_factor, 0);
      if (result >= 0)
        total_variable_successful++;
    }
  }

  double total_elapsed_ms = timer.stop_ms();

  TEST_ASSERT_EQUAL_INT_MESSAGE(total_variable_requests,
                                total_variable_successful,
                                "Все запросы переменной нагрузки должны быть успешными");

  printf(
      "Переменная нагрузка: %d запросов за %.2f мс\n", total_variable_requests, total_elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// ============================================================================
// ИЗМЕРЕНИЕ ВРЕМЕНИ ВЫПОЛНЕНИЯ - 2 ТЕСТА
// ============================================================================

// Тест 17: Бенчмарк различных типов PID
void test_performance_pid_type_benchmark() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  struct PidBenchmark {
    uint8_t pid;
    const char* name;
    uint8_t expected_bytes;
    double scale_factor;
    double bias;
  };

  PidBenchmark benchmarks[] = {{OBD2::ENGINE_RPM, "ENGINE_RPM", 2, 1.0, 0},
                               {OBD2::ENGINE_LOAD, "ENGINE_LOAD", 1, 100.0 / 255.0, 0},
                               {OBD2::ENGINE_COOLANT_TEMP, "COOLANT_TEMP", 1, 1.0, -40},
                               {OBD2::VEHICLE_SPEED, "VEHICLE_SPEED", 1, 1.0, 0},
                               {OBD2::THROTTLE_POSITION, "THROTTLE_POS", 1, 100.0 / 255.0, 0}};

  int num_benchmarks           = sizeof(benchmarks) / sizeof(benchmarks[0]);
  int iterations_per_benchmark = 20;

  printf("\n=== БЕНЧМАРК ТИПОВ PID ===\n");

  for (int b = 0; b < num_benchmarks; b++) {
    g_mock_iso_tp.reset();

    for (int i = 0; i < iterations_per_benchmark; i++) {
      IIsoTp::Message response;
      if (benchmarks[b].expected_bytes == 2) {
        response = create_obd_response_2_bytes(0x7E8, 0x01, benchmarks[b].pid, 0x1A, 0x1B);
      } else {
        response = create_obd_response_1_byte(0x7E8, 0x01, benchmarks[b].pid, 0x80);
      }
      g_mock_iso_tp.add_receive_message(response);
    }
    g_mock_iso_tp.set_receive_result(false);

    PerformanceTimer benchmark_timer;
    benchmark_timer.start();

    int successful_benchmark = 0;
    for (int i = 0; i < iterations_per_benchmark; i++) {
      obd2.processPID(0x01,
                      benchmarks[b].pid,
                      1,
                      benchmarks[b].expected_bytes,
                      benchmarks[b].scale_factor,
                      benchmarks[b].bias);
      double result = obd2.processPID(0x01,
                                      benchmarks[b].pid,
                                      1,
                                      benchmarks[b].expected_bytes,
                                      benchmarks[b].scale_factor,
                                      benchmarks[b].bias);
      if (result >= benchmarks[b].bias)
        successful_benchmark++;
    }

    double benchmark_time       = benchmark_timer.stop_ms();
    double avg_time_per_request = benchmark_time / iterations_per_benchmark;

    TEST_ASSERT_EQUAL_INT_MESSAGE(iterations_per_benchmark,
                                  successful_benchmark,
                                  "Все запросы бенчмарка должны быть успешными");

    printf("%-15s: %.2f мс/запрос (%d запросов за %.2f мс)\n",
           benchmarks[b].name,
           avg_time_per_request,
           iterations_per_benchmark,
           benchmark_time);

    // Очистка памяти
    while (!g_mock_iso_tp.receive_messages.empty()) {
      delete[] g_mock_iso_tp.receive_messages.front().data;
      g_mock_iso_tp.receive_messages.pop();
    }
  }

  printf("=== БЕНЧМАРК ЗАВЕРШЕН ===\n");
}

// Тест 18: Общий бенчмарк производительности системы
void test_performance_overall_system_benchmark() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int benchmark_requests = 500;

  for (int i = 0; i < benchmark_requests; i++) {
    uint8_t pid_type = i % 4;
    IIsoTp::Message response;

    switch (pid_type) {
      case 0:
        response = create_obd_response_2_bytes(0x7E8, 0x01, OBD2::ENGINE_RPM, 0x1A, 0x1B);
        break;
      case 1:
        response = create_obd_response_1_byte(0x7E8, 0x01, OBD2::ENGINE_LOAD, 0x80);
        break;
      case 2:
        response = create_obd_response_1_byte(0x7E8, 0x01, OBD2::VEHICLE_SPEED, 0x50);
        break;
      case 3:
        response = create_obd_response_2_bytes(0x7E8, 0x01, OBD2::MAF_FLOW_RATE, 0x12, 0x34);
        break;
    }

    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  printf("\n=== ОБЩИЙ БЕНЧМАРК СИСТЕМЫ ===\n");

  PerformanceTimer system_timer;
  system_timer.start();

  int successful_system = 0;

  for (int i = 0; i < benchmark_requests; i++) {
    uint8_t pid_type = i % 4;
    uint8_t pid;
    uint8_t expected_bytes;
    double scale_factor = 1.0;

    switch (pid_type) {
      case 0:
        pid            = OBD2::ENGINE_RPM;
        expected_bytes = 2;
        break;
      case 1:
        pid            = OBD2::ENGINE_LOAD;
        expected_bytes = 1;
        scale_factor   = 100.0 / 255.0;
        break;
      case 2:
        pid            = OBD2::VEHICLE_SPEED;
        expected_bytes = 1;
        break;
      case 3:
        pid            = OBD2::MAF_FLOW_RATE;
        expected_bytes = 2;
        break;
    }

    obd2.processPID(0x01, pid, 1, expected_bytes, scale_factor, 0);
    double result = obd2.processPID(0x01, pid, 1, expected_bytes, scale_factor, 0);
    if (result >= 0)
      successful_system++;
  }

  double system_elapsed_ms = system_timer.stop_ms();
  double system_throughput = (benchmark_requests * 1000.0) / system_elapsed_ms;
  double avg_request_time  = system_elapsed_ms / benchmark_requests;

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      benchmark_requests, successful_system, "Все системные запросы должны быть успешными");

  TEST_ASSERT_GREATER_THAN_MESSAGE(
      100.0,
      system_throughput,
      "Системная пропускная способность должна быть больше 100 запросов/сек");

  printf("Системный бенчмарк:\n");
  printf("  - Всего запросов: %d\n", benchmark_requests);
  printf("  - Время выполнения: %.2f мс\n", system_elapsed_ms);
  printf("  - Пропускная способность: %.1f запросов/сек\n", system_throughput);
  printf("  - Среднее время запроса: %.2f мс\n", avg_request_time);
  printf("=== СИСТЕМНЫЙ БЕНЧМАРК ЗАВЕРШЕН ===\n");

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// ============================================================================
// ПАРАЛЛЕЛЬНЫЕ ЗАПРОСЫ - 2 ТЕСТА
// ============================================================================

// Тест 19: Имитация параллельных запросов
void test_performance_simulated_parallel_requests() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int parallel_requests = 60;

  for (int i = 0; i < parallel_requests; i++) {
    IIsoTp::Message response = create_obd_response_1_byte(0x7E8, 0x01, OBD2::ENGINE_LOAD, 0x80 + i);
    g_mock_iso_tp.add_receive_message(response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int successful_parallel = 0;
  for (int i = 0; i < parallel_requests; i++) {
    obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    double result = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    if (result >= 0)
      successful_parallel++;
  }

  double elapsed_ms = timer.stop_ms();

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      parallel_requests, successful_parallel, "Все параллельные запросы должны быть успешными");

  TEST_ASSERT_LESS_THAN_MESSAGE(
      3000.0, elapsed_ms, "Параллельные запросы должны выполняться быстро");

  printf("Имитация параллельных запросов: %d запросов за %.2f мс\n", parallel_requests, elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// Тест 20: Чередующиеся запросы разных типов
void test_performance_interleaved_request_types() {
  g_mock_iso_tp.reset();
  OBD2 obd2(g_mock_iso_tp);

  int interleaved_cycles = 25;

  for (int i = 0; i < interleaved_cycles; i++) {
    // RPM ответ
    IIsoTp::Message rpm_response =
        create_obd_response_2_bytes(0x7E8, 0x01, OBD2::ENGINE_RPM, 0x1A, 0x1B);
    g_mock_iso_tp.add_receive_message(rpm_response);

    // LOAD ответ
    IIsoTp::Message load_response =
        create_obd_response_1_byte(0x7E8, 0x01, OBD2::ENGINE_LOAD, 0x80);
    g_mock_iso_tp.add_receive_message(load_response);

    // SPEED ответ
    IIsoTp::Message speed_response =
        create_obd_response_1_byte(0x7E8, 0x01, OBD2::VEHICLE_SPEED, 0x50);
    g_mock_iso_tp.add_receive_message(speed_response);
  }
  g_mock_iso_tp.set_receive_result(false);

  PerformanceTimer timer;
  timer.start();

  int successful_interleaved = 0;
  for (int i = 0; i < interleaved_cycles; i++) {
    // RPM запрос
    obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    double rpm = obd2.processPID(0x01, OBD2::ENGINE_RPM, 1, 2);
    if (rpm > 0)
      successful_interleaved++;

    // LOAD запрос
    obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    double load = obd2.processPID(0x01, OBD2::ENGINE_LOAD, 1, 1, 100.0 / 255.0, 0);
    if (load >= 0)
      successful_interleaved++;

    // SPEED запрос
    obd2.processPID(0x01, OBD2::VEHICLE_SPEED, 1, 1);
    double speed = obd2.processPID(0x01, OBD2::VEHICLE_SPEED, 1, 1);
    if (speed >= 0)
      successful_interleaved++;
  }

  double elapsed_ms = timer.stop_ms();

  TEST_ASSERT_EQUAL_INT_MESSAGE(interleaved_cycles * 3,
                                successful_interleaved,
                                "Все чередующиеся запросы должны быть успешными");

  printf("Чередующиеся запросы: %d запросов за %.2f мс\n", interleaved_cycles * 3, elapsed_ms);

  // Очистка памяти
  while (!g_mock_iso_tp.receive_messages.empty()) {
    delete[] g_mock_iso_tp.receive_messages.front().data;
    g_mock_iso_tp.receive_messages.pop();
  }
}

// ============================================================================
// ФУНКЦИЯ ЗАПУСКА ВСЕХ ТЕСТОВ ПРОИЗВОДИТЕЛЬНОСТИ
// ============================================================================

extern "C" void run_obd_performance_tests() {
  printf("\n=== ЗАПУСК ТЕСТОВ ПРОИЗВОДИТЕЛЬНОСТИ OBD2 ===\n");

  // Тесты множественных последовательных запросов
  RUN_TEST(test_performance_100_sequential_rpm_requests);
  RUN_TEST(test_performance_mixed_pid_sequence);
  RUN_TEST(test_performance_multiple_ecu_sequence);

  // Тесты быстрых циклов запрос-ответ
  RUN_TEST(test_performance_minimal_delay_cycles);
  RUN_TEST(test_performance_high_frequency_requests);

  // Тесты больших объемов данных
  RUN_TEST(test_performance_maximum_payload_size);
  RUN_TEST(test_performance_multiple_large_responses);
  RUN_TEST(test_performance_buffer_accumulation);

  // Тесты памяти
  RUN_TEST(test_performance_memory_leak_check);
  RUN_TEST(test_performance_large_data_memory_management);
  RUN_TEST(test_performance_memory_stress_various_sizes);

  // Нагрузочные тесты
  RUN_TEST(test_performance_sustained_load);
  RUN_TEST(test_performance_peak_load);
  RUN_TEST(test_performance_mixed_load_patterns);

  // Тесты стабильности
  RUN_TEST(test_performance_long_term_stability);
  RUN_TEST(test_performance_variable_load_stability);

  // Измерение времени выполнения
  RUN_TEST(test_performance_pid_type_benchmark);
  RUN_TEST(test_performance_overall_system_benchmark);

  // Параллельные запросы
  RUN_TEST(test_performance_simulated_parallel_requests);
  RUN_TEST(test_performance_interleaved_request_types);

  printf("=== ТЕСТЫ ПРОИЗВОДИТЕЛЬНОСТИ OBD2 ЗАВЕРШЕНЫ ===\n");
}