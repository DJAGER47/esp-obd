#pragma once

#include "PIDParser.hpp"
#include "RingBuffer.hpp"
#include "driver/twai.h"

class CANDriver {
 public:
  CANDriver();
  ~CANDriver();

  bool init();
  void start();
  void stop();
  void processMessages();

 private:
  static void twaiISRHandler(void* arg);
  void handleISR();
  void processMessage(const twai_message_t& msg);

  RingBuffer<twai_message_t, 32> m_rxBuffer;
  PIDParser m_parser;
  bool m_isInitialized = false;
};