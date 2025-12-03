#pragma once

#include "phy_interface.h"

// Определяем тип функции обратного вызова для CAN сообщений
typedef void (*CanMessageCallback)(const TwaiFrame& frame);

class CanSubscriber : public ITwaiSubscriber {
 public:
  explicit CanSubscriber(CanMessageCallback callback);

  QueueHandle_t onTwaiMessage() override;
  bool isInterested(const TwaiFrame& frame) override;
  void ProcessMessages();  // Обработка сообщений из очереди

 private:
  CanMessageCallback callback_;
  QueueHandle_t message_queue_;
};