#pragma once

#include <cstdint>

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

  /**
   * @brief Установка и запуск TWAI драйвера
   * @return TwaiError::OK при успешном выполнении
   */
  virtual TwaiError install_and_start() = 0;

  /**
   * @brief Передача CAN сообщения
   * @param message Указатель на сообщение для передачи
   * @param ticks_to_wait Время ожидания в тиках FreeRTOS
   * @return TwaiError::OK при успешной передаче
   */
  virtual TwaiError transmit(?? message,
                             TickType_t ticks_to_wait) = 0;

  /**
   * @brief Прием CAN сообщения
   * @param message Указатель на структуру для сохранения принятого сообщения
   * @param ticks_to_wait Время ожидания в тиках FreeRTOS
   * @return TwaiError::OK при успешном приеме
   */
  virtual TwaiError receive(?? message,
                            TickType_t ticks_to_wait) = 0;

 protected:
  ~ITwaiInterface() = default;
};
