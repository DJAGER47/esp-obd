#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

/**
 * @brief RAII-обёртка для критической секции FreeRTOS
 *
 * Критическая секция отключает прерывания, обеспечивая атомарный доступ
 * к разделяемым ресурсам.
 *
 * Пример использования:
 * @code
 * {
 *     CriticalSection cs;  // Вход в критическую секцию
 *     // Защищённый код
 *     shared_variable++;
 * }  // Автоматический выход из критической секции
 * @endcode
 *
 * @note Не используйте в критической секции блокирующие операции
 * @note Критическая секция должна быть максимально короткой
 */
class CriticalSection {
 public:
  CriticalSection() {
    portENTER_CRITICAL(&mux_);
  }

  ~CriticalSection() {
    portEXIT_CRITICAL(&mux_);
  }

  // Запрещаем копирование и перемещение
  CriticalSection(const CriticalSection&)            = delete;
  CriticalSection& operator=(const CriticalSection&) = delete;
  CriticalSection(CriticalSection&&)                 = delete;
  CriticalSection& operator=(CriticalSection&&)      = delete;

 private:
  static portMUX_TYPE mux_;  ///< Мьютекс для критической секции
};

/**
 * @brief RAII-обёртка для критической секции из ISR (обработчика прерываний)
 *
 * Специальная версия критической секции для использования в обработчиках прерываний.
 * Использует ISR-безопасные версии макросов FreeRTOS.
 *
 * Пример использования:
 * @code
 * void IRAM_ATTR my_isr_handler(void* arg) {
 *     CriticalSectionISR cs;  // Вход в критическую секцию ISR
 *     // Защищённый код в ISR
 *     shared_variable++;
 * }  // Автоматический выход из критической секции
 * @endcode
 *
 * @note Используйте только в обработчиках прерываний
 * @note Для обычного кода используйте CriticalSection
 */
class CriticalSectionISR {
 public:
  CriticalSectionISR() {
    portENTER_CRITICAL_ISR(&mux_);
  }

  ~CriticalSectionISR() {
    portEXIT_CRITICAL_ISR(&mux_);
  }

  // Запрещаем копирование и перемещение
  CriticalSectionISR(const CriticalSectionISR&)            = delete;
  CriticalSectionISR& operator=(const CriticalSectionISR&) = delete;
  CriticalSectionISR(CriticalSectionISR&&)                 = delete;
  CriticalSectionISR& operator=(CriticalSectionISR&&)      = delete;

 private:
  static portMUX_TYPE mux_;  ///< Мьютекс для критической секции ISR
};