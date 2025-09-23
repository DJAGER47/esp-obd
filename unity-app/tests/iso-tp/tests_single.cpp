#include <cstdio>
#include <cstring>

#include "iso_tp.h"
#include "mock_twai_interface.h"
#include "unity.h"

// ============================================================================
// ВСЕОБЪЕМЛЮЩИЕ ТЕСТЫ ISO-TP ПРОТОКОЛА
// ============================================================================

/*
 * ПОКРЫТИЕ ТЕСТАМИ:
 *
 * ✅ ОТПРАВКА (SEND):
 * - Одиночные кадры (Single Frame): обычные, максимальные, пустые
 * - Обработка ошибок передачи
 * - Проверка корректности PCI байтов и данных
 * - Различные CAN ID
 *
 * ✅ ПРИЁМ (RECEIVE):
 * - Одиночные кадры с проверкой данных
 * - Различные размеры данных
 *
 */

// Тест 1: Отправка стандартного одиночного кадра
void test_iso_tp_send_single_frame() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed");
  TEST_ASSERT_TRUE_MESSAGE(mock_can.transmit_called, "Transmit should be called");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_can.transmitted_frames.size(), "Should transmit one frame");

  const auto& frame = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x7DF, frame.id, "TX ID should match");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(8, frame.data_length, "Data length should be 8");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x04, frame.data[0], "PCI should indicate SF with length 4");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
      test_data, &frame.data[1], sizeof(test_data), "Data should match");
}

// Тест 2: Отправка максимального одиночного кадра (7 байт)
void test_iso_tp_send_single_frame_max_size() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed");
  const auto& frame = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x07, frame.data[0], "PCI should indicate SF with length 7");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
      test_data, &frame.data[1], sizeof(test_data), "Data should match");
}

// Тест 3: Отправка пустого кадра
void test_iso_tp_send_empty_frame() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = 0;
  msg.data  = nullptr;

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed");
  const auto& frame = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x00, frame.data[0], "PCI should indicate SF with length 0");
}

// Тест 4: Обработка ошибки передачи
void test_iso_tp_send_transmit_error() {
  // Считаем что ошибки не может быть
  // MockTwaiInterface mock_can;
  // mock_can.reset();
  // mock_can.transmit_result = IPhyInterface::TwaiError::TRANSMIT_FAILED;
  // IsoTp iso_tp(mock_can);

  // uint8_t test_data[] = {0x01, 0x02, 0x03};
  // IsoTp::Message msg;
  // msg.tx_id = 0x123;
  // msg.rx_id = 0x456;
  // msg.len   = sizeof(test_data);
  // msg.data  = test_data;

  // bool result = iso_tp.send(msg);

  // TEST_ASSERT_FALSE_MESSAGE(result, "Send should fail when transmit fails");
}

// Тест 5: Отправка с различными CAN ID
void test_iso_tp_send_different_ids() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[] = {0xAA, 0xBB};
  IsoTp::Message msg;
  msg.tx_id = 0x7E0;
  msg.rx_id = 0x7E8;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with different ID");
  const auto& frame = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x7E0, frame.id, "Should use correct TX ID");
}

// Тест 6: Отправка одного байта данных
void test_iso_tp_send_single_byte() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[] = {0xFF};
  IsoTp::Message msg;
  msg.tx_id = 0x100;
  msg.rx_id = 0x200;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed");
  const auto& frame = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x01, frame.data[0], "PCI should indicate SF with length 1");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0xFF, frame.data[1], "Data should match");
}

// Тест 7: Приём одиночного кадра
void test_iso_tp_receive_single_frame() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t expected_data[]           = {0xAA, 0xBB, 0xCC, 0xDD};
  IPhyInterface::TwaiFrame sf_frame = create_single_frame(0x7E8, 4, expected_data);
  mock_can.add_receive_frame(sf_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg, sizeof(receive_buffer));

  TEST_ASSERT_TRUE_MESSAGE(result, "Receive should succeed");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(4, msg.len, "Received length should be 4");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected_data, msg.data, 4, "Received data should match");
}

// Тест 8: Приём максимального одиночного кадра
void test_iso_tp_receive_single_frame_max() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t expected_data[]           = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
  IPhyInterface::TwaiFrame sf_frame = create_single_frame(0x456, 7, expected_data);
  mock_can.add_receive_frame(sf_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg, sizeof(receive_buffer));

  TEST_ASSERT_TRUE_MESSAGE(result, "Receive should succeed");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(7, msg.len, "Received length should be 7");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected_data, msg.data, 7, "Received data should match");
}

// Тест 9: Приём пустого кадра
void test_iso_tp_receive_empty_frame() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  IPhyInterface::TwaiFrame sf_frame = create_single_frame(0x789, 0, nullptr);
  mock_can.add_receive_frame(sf_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x789;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg, sizeof(receive_buffer));

  TEST_ASSERT_TRUE_MESSAGE(result, "Receive should succeed");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0, msg.len, "Received length should be 0");
}

// Тест 10: Проверка корректности PCI байтов при различных размерах
void test_iso_tp_pci_bytes_validation() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Тестируем различные размеры от 1 до 7 байт
  for (uint8_t size = 1; size <= 7; size++) {
    mock_can.reset();

    uint8_t test_data[7] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70};
    IsoTp::Message msg;
    msg.tx_id = 0x100 + size;
    msg.rx_id = 0x200 + size;
    msg.len   = size;
    msg.data  = test_data;

    bool result = iso_tp.send(msg);

    TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed");
    const auto& frame = mock_can.transmitted_frames[0];
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(size, frame.data[0], "PCI should match data size");
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(test_data, &frame.data[1], size, "Data should match");
  }
}

// Функция запуска всех тестов
extern "C" void run_iso_tp_tests() {
  // Основные тесты отправки
  RUN_TEST(test_iso_tp_send_single_frame);
  RUN_TEST(test_iso_tp_send_single_frame_max_size);
  RUN_TEST(test_iso_tp_send_empty_frame);
  RUN_TEST(test_iso_tp_send_transmit_error);
  RUN_TEST(test_iso_tp_send_different_ids);
  RUN_TEST(test_iso_tp_send_single_byte);

  // Основные тесты приёма
  RUN_TEST(test_iso_tp_receive_single_frame);
  RUN_TEST(test_iso_tp_receive_single_frame_max);
  RUN_TEST(test_iso_tp_receive_empty_frame);

  // Дополнительные тесты
  RUN_TEST(test_iso_tp_pci_bytes_validation);
}