#pragma once

#include "driver/twai.h"
#include "twai_errors.h"

class ITwaiInterface {
 public:
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
  virtual TwaiError transmit(const twai_message_t* message,
                             TickType_t ticks_to_wait) = 0;

  /**
   * @brief Прием CAN сообщения
   * @param message Указатель на структуру для сохранения принятого сообщения
   * @param ticks_to_wait Время ожидания в тиках FreeRTOS
   * @return TwaiError::OK при успешном приеме
   */
  virtual TwaiError receive(twai_message_t* message,
                            TickType_t ticks_to_wait) = 0;

 protected:
  ~ITwaiInterface() = default;
};
