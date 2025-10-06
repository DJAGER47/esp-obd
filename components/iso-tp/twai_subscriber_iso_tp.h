#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "phy_interface.h"

class TwaiSubscriberIsoTp final : public ITwaiSubscriber {
 public:
  explicit TwaiSubscriberIsoTp(uint32_t queue_size = 10);
  bool isInterested(const TwaiFrame& frame) override;

  /**
   * @brief Обработка входящего TWAI сообщения
   * @param frame Ссылка на полученный TWAI фрейм
   * @return true если сообщение успешно помещено в очередь, false в противном случае
   */
  bool onTwaiMessage(const TwaiFrame& frame) override;

  /**
   * @brief Получение сообщения из очереди
   * @param frame Ссылка на структуру для сохранения полученного сообщения
   * @param timeout_ms Время ожидания в миллисекундах
   * @return true если сообщение успешно получено, false в случае таймаута
   */
  bool Receive(TwaiFrame& frame, TickType_t timeout_ms);

 private:
  QueueHandle_t xQueue_;
};