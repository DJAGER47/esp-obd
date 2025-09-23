#pragma once

#include <queue>
#include <vector>

#include "phy_interface.h"

// Мок-класс для IPhyInterface для тестирования ISO-TP протокола
class MockTwaiInterface : public IPhyInterface {
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

// Вспомогательные функции для создания различных типов кадров

// Создание одиночного кадра (Single Frame)
inline IPhyInterface::TwaiFrame create_single_frame(uint32_t id,
                                                    uint8_t length,
                                                    const uint8_t* data) {
  IPhyInterface::TwaiFrame frame = {};
  frame.id                       = id;
  frame.data_length              = 8;
  frame.data[0]                  = length;  // SF PCI
  if (data && length <= 7) {
    memcpy(&frame.data[1], data, length);
  }
  return frame;
}

// Создание первого кадра (First Frame)
inline IPhyInterface::TwaiFrame create_first_frame(uint32_t id,
                                                   uint16_t total_length,
                                                   const uint8_t* data) {
  IPhyInterface::TwaiFrame frame = {};
  frame.id                       = id;
  frame.data_length              = 8;
  frame.data[0]                  = 0x10 | ((total_length >> 8) & 0x0F);  // FF PCI
  frame.data[1]                  = total_length & 0xFF;
  if (data) {
    memcpy(&frame.data[2], data, 6);  // Первые 6 байт данных
  }
  return frame;
}

// Создание последовательного кадра (Consecutive Frame)
inline IPhyInterface::TwaiFrame create_consecutive_frame(uint32_t id,
                                                         uint8_t seq_num,
                                                         const uint8_t* data,
                                                         uint8_t length) {
  IPhyInterface::TwaiFrame frame = {};
  frame.id                       = id;
  frame.data_length              = 8;
  frame.data[0]                  = 0x20 | (seq_num & 0x0F);  // CF PCI
  if (data && length <= 7) {
    memcpy(&frame.data[1], data, length);
  }
  return frame;
}

// Создание кадра управления потоком (Flow Control)
inline IPhyInterface::TwaiFrame create_flow_control_frame(uint32_t id,
                                                          uint8_t flow_status,
                                                          uint8_t block_size,
                                                          uint8_t sep_time) {
  IPhyInterface::TwaiFrame frame = {};
  frame.id                       = id;
  frame.data_length              = 8;
  frame.data[0]                  = 0x30 | (flow_status & 0x0F);  // FC PCI
  frame.data[1]                  = block_size;
  frame.data[2]                  = sep_time;
  return frame;
}