#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "phy_interface.h"

class TwaiSubscriberIsoTp final : public ITwaiSubscriber {
 public:
  explicit TwaiSubscriberIsoTp(uint32_t queue_size = 10);
#ifdef TEST_INSTANCES
  ~TwaiSubscriberIsoTp();
#endif
  bool isInterested(const TwaiFrame& frame) override;

  /**
   * @brief Получение очереди для входящих TWAI сообщений
   * @return Указатель на очередь для помещения сообщений
   */
  QueueHandle_t onTwaiMessage() override;

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