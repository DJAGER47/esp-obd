#pragma once

#include "phy_interface.h"

// Определяем тип функции обратного вызова для CAN сообщений
typedef void (*CanMessageCallback)(const TwaiFrame& frame);

class CanSubscriber : public ITwaiSubscriber {
 public:
  explicit CanSubscriber(CanMessageCallback callback);

  bool onTwaiMessage(const TwaiFrame& frame) override;
  bool isInterested(const TwaiFrame& frame) override;

 private:
  CanMessageCallback callback_;
};