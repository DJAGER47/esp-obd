#include "can_subscriber.h"

CanSubscriber::CanSubscriber(CanMessageCallback callback) :
    callback_(callback),
    message_queue_(nullptr) {
  // Создаем очередь для сообщений
  message_queue_ = xQueueCreate(10, sizeof(TwaiFrame));
}

QueueHandle_t CanSubscriber::onTwaiMessage() {
  // Возвращаем очередь для помещения сообщения
  return message_queue_;
}

bool CanSubscriber::isInterested(const TwaiFrame& frame) {
  // Нас интересуют все сообщения
  return true;
}

void CanSubscriber::ProcessMessages() {
  TwaiFrame frame;
  // Обрабатываем все сообщения в очереди
  while (xQueueReceive(message_queue_, &frame, 0) == pdTRUE) {
    if (callback_ != nullptr) {
      callback_(frame);
    }
  }
}