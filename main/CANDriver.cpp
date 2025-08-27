#include "CANDriver.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

CANDriver::CANDriver() {
  // Конструктор по умолчанию
}

CANDriver::~CANDriver() {
  stop();
}

bool CANDriver::init() {
  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    m_isInitialized = true;
    return true;
  }
  return false;
}

void CANDriver::start() {
  if (m_isInitialized) {
    twai_start();
    // Настройка прерывания
    // Настройка прерываний
    twai_reconfigure_alerts(TWAI_ALERT_RX_DATA, NULL);
    // Используем стандартный обработчик прерываний ESP-IDF
  }
}

void CANDriver::stop() {
  if (m_isInitialized) {
    twai_stop();
    twai_driver_uninstall();
    m_isInitialized = false;
  }
}

void CANDriver::processMessages() {
  twai_message_t msg;
  while (m_rxBuffer.pop(&msg)) {
    processMessage(msg);
  }
}

void CANDriver::twaiISRHandler(void* arg) {
  CANDriver* driver = static_cast<CANDriver*>(arg);
  driver->handleISR();
}

void CANDriver::handleISR() {
  twai_message_t msg;
  while (twai_receive(&msg, 0) == ESP_OK) {
    if (!m_rxBuffer.push(&msg)) {
      // Буфер переполнен, просто игнорируем сообщение
    }
  }
}

void CANDriver::processMessage(const twai_message_t& msg) {
  if (msg.extd || msg.rtr) {
    return;  // Игнорируем расширенные и remote-фреймы
  }
  if (msg.identifier == 0x7DF) {  // OBD2 broadcast
    switch (msg.data[2]) {
      case 0x2F:  // Fuel level
        m_parser.parseFuelConsumption(msg);
        break;
      case 0x31:  // Distance
        // Обработка пробега
        break;
    }
  }
}