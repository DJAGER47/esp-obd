#pragma once

#include <queue>
#include <vector>

#include "twai_interface.h"

// Мок-класс для ITwaiInterface для тестирования ISO-TP протокола
class MockTwaiInterface : public ITwaiInterface {
 public:
  TwaiError install_and_start() override {
    return TwaiError::OK;
  }

  TwaiError transmit(const TwaiFrame& message, uint32_t ticks_to_wait) override {
    transmitted_frames.push_back(message);
    transmit_called = true;
    return transmit_result;
  }

  TwaiError receive(TwaiFrame& message, uint32_t ticks_to_wait) override {
    receive_called = true;
    if (!receive_frames.empty()) {
      message = receive_frames.front();
      receive_frames.pop();
      return TwaiError::OK;
    }
    return TwaiError::TIMEOUT;
  }

  // Методы для управления состоянием мока
  void reset() {
    transmitted_frames.clear();
    while (!receive_frames.empty()) {
      receive_frames.pop();
    }
    transmit_called = false;
    receive_called  = false;
    transmit_result = TwaiError::OK;
  }

  void add_receive_frame(const TwaiFrame& frame) {
    receive_frames.push(frame);
  }

  // Публичные поля для проверки в тестах
  std::vector<TwaiFrame> transmitted_frames;
  std::queue<TwaiFrame> receive_frames;
  bool transmit_called      = false;
  bool receive_called       = false;
  TwaiError transmit_result = TwaiError::OK;
};

// Вспомогательная функция для создания одиночного кадра
inline ITwaiInterface::TwaiFrame create_single_frame(uint32_t id,
                                                     uint8_t length,
                                                     const uint8_t* data) {
  ITwaiInterface::TwaiFrame frame = {};
  frame.id                        = id;
  frame.data_length               = 8;
  frame.data[0]                   = length;  // SF PCI
  if (data && length <= 7) {
    memcpy(&frame.data[1], data, length);
  }
  return frame;
}