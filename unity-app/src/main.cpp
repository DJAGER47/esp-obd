#include <cstdio>

// Подключение заголовочных файлов Unity
#include "unity.h"

extern "C" void run_iso_tp_tests();
extern "C" void run_iso_tp_extended_tests();
extern "C" void run_obd_basic_tests();
extern "C" void run_obd_pids_tests();

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

  run_iso_tp_tests();
  run_iso_tp_extended_tests();

  printf("\n=== Запуск тестов OBD2 ===\n");
  run_obd_basic_tests();
  run_obd_pids_tests();

  // Завершение Unity и получение результата
  int failures = UNITY_END();

  printf("Тесты завершены. Количество ошибок: %d\n", failures);

  // Возвращаем код ошибки, если есть ошибки
  return failures;
}