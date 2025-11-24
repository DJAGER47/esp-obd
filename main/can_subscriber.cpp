#include "can_subscriber.h"

CanSubscriber::CanSubscriber(CanMessageCallback callback) :
    callback_(callback) {}

bool CanSubscriber::onTwaiMessage(const TwaiFrame& frame) {
  // Вызываем функцию обратного вызова для обработки сообщения
  if (callback_ != nullptr) {
    callback_(frame);
  }
  return true;
}

bool CanSubscriber::isInterested(const TwaiFrame& frame) {
  // Нас интересуют все сообщения
  return true;
}