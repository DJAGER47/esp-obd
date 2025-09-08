#include <stdio.h>
#include <string.h>  // Для memset

// Включения для Unity и FreeRTOS
#include "freertos/FreeRTOS.h"
#include "unity.h"

// Включения для тестируемого компонента и его зависимостей
#include "iso-tp.h"
#include "twai_interface.h"

// Мок-класс для ITwaiInterface
class MockTwaiInterface : public ITwaiInterface {
 public:
  TwaiError install_and_start() override {
    return TwaiError::OK;
  }

  TwaiError transmit(const TwaiFrame& message, TickType_t ticks_to_wait) override {
    last_transmitted_frame = message;
    transmit_called        = true;
    return TwaiError::OK;
  }

  TwaiError receive(TwaiFrame& message, TickType_t ticks_to_wait) override {
    if (should_receive) {
      message        = next_receive_frame;
      receive_called = true;
      return TwaiError::OK;
    }
    receive_called = true;
    return TwaiError::TIMEOUT;  // Имитируем таймаут, если не должно быть приема
  }

  // Поля для проверки в тестах
  TwaiFrame last_transmitted_frame;
  TwaiFrame next_receive_frame;
  bool transmit_called = false;
  bool receive_called  = false;
  bool should_receive  = false;
};

// Тестовая функция
void test_iso_tp_send_single_frame(void) {
  // Создаем мок CAN интерфейса
  MockTwaiInterface mock_can;

  // Создаем экземпляр IsoTp
  IsoTp iso_tp(mock_can);

  // Подготавливаем тестовое сообщение
  uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Отправляем сообщение
  bool result = iso_tp.send(msg);

  // Проверяем результат
  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed");
  TEST_ASSERT_TRUE_MESSAGE(mock_can.transmit_called, "Transmit should be called");

  // Проверяем содержимое отправленного кадра
  TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x7DF, mock_can.last_transmitted_frame.id, "TX ID should match");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(test_data) + 1,
                                  mock_can.last_transmitted_frame.data_length,
                                  "Data length should match");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0x04, mock_can.last_transmitted_frame.data[0], "PCI byte should indicate SF with length 4");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(&test_data[0],
                                        &mock_can.last_transmitted_frame.data[1],
                                        sizeof(test_data),
                                        "Data should match");
}

// Функция для регистрации тестов
void app_main() {
  UNITY_BEGIN();
  RUN_TEST(test_iso_tp_send_single_frame);
  UNITY_END();
}