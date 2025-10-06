#pragma once

#include <cstdint>

#include "time_utils.h"

// Определение структуры TwaiFrame перед использованием
struct TwaiFrame {
  uint32_t id;       // Идентификатор сообщения
  bool is_extended;  // Флаг расширенного идентификатора (29 бит)
  bool is_rtr;       // Флаг удаленного запроса (Remote Transmission Request)
  bool is_fd;        // Флаг CAN FD
  bool brs;          // Bit Rate Switch (для CAN FD)
  uint8_t data[8];   // Данные (максимум 64 байта для CAN FD)
  uint8_t data_length;  // Длина данных (DLC)
};

class ITwaiSubscriber {
 public:
  /**
   * @brief Обработка входящего TWAI сообщения
   *
   * Этот метод вызывается, когда получено TWAI сообщение,
   * в котором подписчик выразил заинтересованность.
   *
   * @param frame Ссылка на полученный TWAI фрейм
   * @return true если сообщение было успешно обработано, false в противном случае
   */
  virtual bool onTwaiMessage(const TwaiFrame& frame) = 0;

  /**
   * @brief Проверка заинтересованности подписчика в конкретном TWAI сообщении
   *
   * Этот метод используется для определения, хочет ли подписчик
   * получать уведомления о конкретном TWAI сообщении на основе
   * его ID или других атрибутов.
   *
   * @param frame Ссылка на TWAI фрейм для проверки заинтересованности
   * @return true если подписчик заинтересован в этом сообщении, false в противном случае
   */
  virtual bool isInterested(const TwaiFrame& frame) = 0;

 protected:
  virtual ~ITwaiSubscriber() = default;
};

class IPhyInterface {
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
    TIMEOUT,
    NO_MEM
  };

  /**
   * @brief Установка и запуск TWAI драйвера
   * @return TwaiError::OK при успешном выполнении
   */
  virtual TwaiError InstallStart() = 0;

  /**
   * @brief Передача CAN сообщения
   * @param message Указатель на сообщение для передачи
   * @param timeout_ms Время ожидания в ms
   * @return TwaiError::OK при успешной передаче
   */
  virtual TwaiError Transmit(const TwaiFrame& message, Time_ms timeout_ms) = 0;

  /**
   * @brief Регистрация подписчика на сообщения
   * @param subscriber Ссылка на подписчика
   * @return TwaiError::OK при успешной регистрации
   */
  virtual TwaiError RegisterSubscriber(ITwaiSubscriber& subscriber) = 0;

 protected:
  ~IPhyInterface() = default;
};
