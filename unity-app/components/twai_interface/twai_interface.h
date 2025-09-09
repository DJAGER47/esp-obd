#pragma once

#include <cstdint>

// Заменяем TickType_t на int для Linux
typedef int TickType_t;

class ITwaiInterface {
 public:
  enum class TwaiError : uint32_t {
    OK = 0,
    GENERAL_FAILURE,
    INVALID_STATE,
    NOT_INITIALIZED,
    ALREADY_INITIALIZED,
    TRANSMIT_FAILED,
    RECEIVE_FAILED,
    DRIVER_INSTALL_FAILED,
    DRIVER_START_FAILED,
    DRIVER_STOP_FAILED,
    DRIVER_UNINSTALL_FAILED,
    INVALID_MESSAGE,
    TIMEOUT
  };

  struct TwaiFrame {
    uint32_t id;       // Идентификатор сообщения
    bool is_extended;  // Флаг расширенного идентификатора (29 бит)
    bool is_rtr;       // Флаг удаленного запроса (Remote Transmission Request)
    bool is_fd;        // Флаг CAN FD
    bool brs;          // Bit Rate Switch (для CAN FD)
    uint8_t data[8];   // Данные (максимум 64 байта для CAN FD)
    uint8_t data_length;  // Длина данных (DLC)
  };

  /**
   * @brief Установка и запуск TWAI драйвера
   * @return TwaiError::OK при успешном выполнении
   */
  virtual TwaiError install_and_start() = 0;

  /**
   * @brief Передача CAN сообщения
   * @param message Указатель на сообщение для передачи
   * @param ticks_to_wait Время ожидания в тиках
   * @return TwaiError::OK при успешной передаче
   */
  virtual TwaiError transmit(const TwaiFrame& message, TickType_t ticks_to_wait) = 0;

  /**
   * @brief Прием CAN сообщения
   * @param message Указатель на структуру для сохранения принятого сообщения
   * @param ticks_to_wait Время ожидания в тиках
   * @return TwaiError::OK при успешном приеме
   */
  virtual TwaiError receive(TwaiFrame& message, TickType_t ticks_to_wait) = 0;

 protected:
  ~ITwaiInterface() = default;
};