#pragma once

#include <cstring>
#include <queue>
#include <vector>

#include "iso_tp_interface.h"

// Мок-класс для IsoTp для тестирования OBD2 протокола
// Наследуется от IIsoTp и полностью мокает поведение IsoTp
class MockIsoTp : public IIsoTp {
 public:
  MockIsoTp() {
    reset();
  }

  // Переопределенные виртуальные методы из IIsoTp
  bool send(Message& message) override {
    send_called       = true;
    last_sent_message = message;
    sent_messages.push_back(message);
    return send_result;
  }

  bool receive(Message& message, size_t size_buffer) override {
    receive_called = true;
    if (!receive_messages.empty()) {
      message = receive_messages.front();
      receive_messages.pop();
      return receive_result;
    }
    return false;  // Нет сообщений для получения
  }

  // Методы для управления состоянием мока
  void reset() {
    sent_messages.clear();
    while (!receive_messages.empty()) {
      receive_messages.pop();
    }
    send_called    = false;
    receive_called = false;
    send_result    = true;
    receive_result = false;  // false означает успех для receive
  }

  void add_receive_message(const Message& message) {
    receive_messages.push(message);
  }

  void set_send_result(bool result) {
    send_result = result;
  }

  void set_receive_result(bool result) {
    receive_result = result;
  }

  // Публичные поля для проверки в тестах
  std::vector<Message> sent_messages;
  std::queue<Message> receive_messages;
  Message last_sent_message;
  bool send_called    = false;
  bool receive_called = false;
  bool send_result    = true;
  bool receive_result = false;
};

// Вспомогательные функции для создания OBD2 сообщений

// Создание OBD2 ответа для одного байта данных
inline IIsoTp::Message create_obd_response_1_byte(uint32_t rx_id,
                                                  uint8_t service,
                                                  uint8_t pid,
                                                  uint8_t data) {
  IIsoTp::Message msg;
  msg.tx_id   = 0x7DF;  // Стандартный OBD2 запрос ID
  msg.rx_id   = rx_id;
  msg.len     = 4;
  msg.data    = new uint8_t[4];
  msg.data[0] = service + 0x40;  // Положительный ответ
  msg.data[1] = pid;
  msg.data[2] = data;
  msg.data[3] = 0x00;  // Заполнение
  return msg;
}

// Создание OBD2 ответа для двух байт данных
inline IIsoTp::Message create_obd_response_2_bytes(
    uint32_t rx_id, uint8_t service, uint8_t pid, uint8_t data_a, uint8_t data_b) {
  IIsoTp::Message msg;
  msg.tx_id   = 0x7DF;
  msg.rx_id   = rx_id;
  msg.len     = 8;
  msg.data    = new uint8_t[8];
  msg.data[0] = 4;
  msg.data[1] = service + 0x40;
  msg.data[2] = pid;
  msg.data[3] = data_a;
  msg.data[4] = data_b;
  msg.data[5] = 0x00;
  msg.data[6] = 0x00;
  msg.data[7] = 0x00;
  return msg;
}

// Создание OBD2 ответа для четырех байт данных (например, для supportedPIDs)
inline IIsoTp::Message create_obd_response_4_bytes(uint32_t rx_id,
                                                   uint8_t service,
                                                   uint8_t pid,
                                                   uint8_t data_a,
                                                   uint8_t data_b,
                                                   uint8_t data_c,
                                                   uint8_t data_d) {
  IIsoTp::Message msg;
  msg.tx_id   = 0x7DF;
  msg.rx_id   = rx_id;
  msg.len     = 7;
  msg.data    = new uint8_t[7];
  msg.data[0] = service + 0x40;
  msg.data[1] = pid;
  msg.data[2] = data_a;
  msg.data[3] = data_b;
  msg.data[4] = data_c;
  msg.data[5] = data_d;
  msg.data[6] = 0x00;
  return msg;
}

// Создание сообщения об ошибке OBD2
inline IIsoTp::Message create_obd_error_response(uint32_t rx_id,
                                                 uint8_t service,
                                                 uint8_t error_code) {
  IIsoTp::Message msg;
  msg.tx_id   = 0x7DF;
  msg.rx_id   = rx_id;
  msg.len     = 3;
  msg.data    = new uint8_t[3];
  msg.data[0] = 0x7F;  // Негативный ответ
  msg.data[1] = service;
  msg.data[2] = error_code;
  return msg;
}