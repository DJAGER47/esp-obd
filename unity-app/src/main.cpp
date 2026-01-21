#include <cstdio>

// Подключение заголовочных файлов Unity
#include "unity.h"

extern "C" void run_iso_tp_tests();
extern "C" void run_iso_tp_extended_tests();
extern "C" void run_iso_tp_edge_case_tests();
extern "C" void run_twai_subscriber_iso_tp_tests();

extern "C" void run_obd_pid_group_1_20_tests();
extern "C" void run_obd_pid_group_21_40_tests();
extern "C" void run_obd_pid_group_41_60_tests();
extern "C" void run_obd_pid_group_61_80_tests();
extern "C" void run_obd_pid_group_81_xx_tests();
extern "C" void run_obd2_cache_big_endian_tests();

// Функции, необходимые для работы Unity
extern "C" void setUp() {
  // Эта функция вызывается перед каждым тестом
  // Здесь можно разместить код инициализации, общий для всех тестов
}

extern "C" void tearDown() {
  // Эта функция вызывается после каждого теста
  // Здесь можно разместить код очистки, общий для всех тестов
}

int main() {
  printf("Запуск тестов...\n");

  // Инициализация Unity
  UNITY_BEGIN();

  printf("\n=== Запуск тестов ISO-TP ===\n");
  run_iso_tp_tests();
  run_iso_tp_extended_tests();
  run_iso_tp_edge_case_tests();
  run_twai_subscriber_iso_tp_tests();

  // Отключаем тесты OBD2, так как они не работают с текущей версией кода
  // printf("\n=== Запуск тестов OBD2 ===\n");
  run_obd_pid_group_1_20_tests();
  run_obd_pid_group_21_40_tests();
  run_obd_pid_group_41_60_tests();
  run_obd_pid_group_61_80_tests();
  run_obd_pid_group_81_xx_tests();

  printf("\n=== Запуск тестов OBD2 Cache Big Endian ===\n");
  run_obd2_cache_big_endian_tests();

  // Завершение Unity и получение результата
  int failures = UNITY_END();

  printf("Тесты завершены. Количество ошибок: %d\n", failures);

  // Возвращаем код ошибки, если есть ошибки
  return failures;
}