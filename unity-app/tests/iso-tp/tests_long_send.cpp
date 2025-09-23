#include <stdio.h>
#include <string.h>

#include "iso_tp.h"
#include "mock_twai_interface.h"
#include "unity.h"

// ============================================================================
// РАСШИРЕННЫЕ ТЕСТЫ ISO-TP ПРОТОКОЛА
// ============================================================================

/*
 * ПОКРЫТИЕ ТЕСТАМИ:
 *
 * ✅ МНОГОКАДРОВЫЕ СООБЩЕНИЯ:
 * - Отправка сообщений длиной > 7 байт (First Frame + Consecutive Frames)
 * - Приём многокадровых сообщений с автоматической отправкой Flow Control
 *
 * ✅ FLOW CONTROL:
 * - CTS (Clear To Send) - разрешение на продолжение передачи
 * - WAIT - запрос на ожидание
 * - OVERFLOW - переполнение буфера получателя
 *
 * ✅ ОБРАБОТКА ОШИБОК:
 * - Неправильная последовательность кадров
 * - Блочная передача с различными размерами блоков
 *
 * ⚠️ ОГРАНИЧЕНИЯ:
 * - Тесты с таймаутами могут работать медленно (500ms)
 */

// Тест 1: Отправка многокадрового сообщения (8 байт - минимум для FF+CF)
void test_iso_tp_send_multi_frame_8_bytes() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Настраиваем Flow Control CTS
  IPhyInterface::TwaiFrame fc_frame = create_flow_control_frame(0x7E8, 0, 0, 0);
  mock_can.add_receive_frame(fc_frame);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed");
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, mock_can.transmitted_frames.size(), "Should transmit FF + CF");

  // Проверяем First Frame
  const auto& ff = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x7DF, ff.id, "FF TX ID should match");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x10, ff.data[0] & 0xF0, "Should be First Frame");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(8, ff.data[1], "FF length should be 8");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
      test_data, &ff.data[2], 6, "FF data should match first 6 bytes");

  // Проверяем Consecutive Frame
  const auto& cf = mock_can.transmitted_frames[1];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x21, cf.data[0], "Should be CF with sequence 1");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
      &test_data[6], &cf.data[1], 2, "CF data should match last 2 bytes");
}

// Тест 2: Отправка длинного многокадрового сообщения (15 байт)
void test_iso_tp_send_multi_frame_15_bytes() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[15];
  for (int i = 0; i < 15; i++) {
    test_data[i] = i + 0x10;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Настраиваем Flow Control CTS
  IPhyInterface::TwaiFrame fc_frame = create_flow_control_frame(0x456, 0, 0, 0);
  mock_can.add_receive_frame(fc_frame);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed");
  TEST_ASSERT_EQUAL_INT_MESSAGE(3, mock_can.transmitted_frames.size(), "Should transmit FF + 2 CF");

  // Проверяем First Frame
  const auto& ff = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x10, ff.data[0] & 0xF0, "Should be First Frame");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(15, ff.data[1], "FF length should be 15");

  // Проверяем первый Consecutive Frame
  const auto& cf1 = mock_can.transmitted_frames[1];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x21, cf1.data[0], "Should be CF with sequence 1");

  // Проверяем второй Consecutive Frame
  const auto& cf2 = mock_can.transmitted_frames[2];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x22, cf2.data[0], "Should be CF with sequence 2");
}

// Тест 3: Flow Control WAIT - получатель просит подождать
void test_iso_tp_send_flow_control_wait() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[10] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44};
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Сначала WAIT, потом CTS
  IPhyInterface::TwaiFrame fc_wait = create_flow_control_frame(0x7E8, 1, 0, 0);  // WAIT
  IPhyInterface::TwaiFrame fc_cts  = create_flow_control_frame(0x7E8, 0, 0, 0);  // CTS
  mock_can.add_receive_frame(fc_wait);
  mock_can.add_receive_frame(fc_cts);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed after FC WAIT");
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, mock_can.transmitted_frames.size(), "Should transmit FF + CF");
}

// Тест 4: Flow Control OVERFLOW - получатель сообщает о переполнении
void test_iso_tp_send_flow_control_overflow() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Отправляем OVERFLOW
  IPhyInterface::TwaiFrame fc_overflow = create_flow_control_frame(0x7E8, 2, 0, 0);  // OVERFLOW
  mock_can.add_receive_frame(fc_overflow);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_FALSE_MESSAGE(result, "Send should fail on FC OVERFLOW");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_can.transmitted_frames.size(), "Should only transmit FF");
}

// Тест 5: Блочная передача с размером блока 2
void test_iso_tp_send_block_size_2() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[20];
  for (int i = 0; i < 20; i++) {
    test_data[i] = i + 0x30;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x100;
  msg.rx_id = 0x200;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // FC с блочным размером 2 (второй FC не нужен, так как все данные передаются в первом блоке)
  IPhyInterface::TwaiFrame fc1 = create_flow_control_frame(0x200, 0, 2, 0);  // Block size = 2
  mock_can.add_receive_frame(fc1);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with block size");
  // FF + 2 CF (блок) = 3 кадра (все данные переданы)
  TEST_ASSERT_EQUAL_INT_MESSAGE(3, mock_can.transmitted_frames.size(), "Should transmit FF + 2 CF");
}

// Тест 6: Приём многокадрового сообщения
void test_iso_tp_receive_multi_frame() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t expected_data[12] = {
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};

  // Создаем First Frame
  IPhyInterface::TwaiFrame ff_frame = create_first_frame(0x7E8, 12, expected_data);

  // Создаем Consecutive Frames
  IPhyInterface::TwaiFrame cf1_frame = create_consecutive_frame(0x7E8, 1, &expected_data[6], 6);

  mock_can.add_receive_frame(ff_frame);
  mock_can.add_receive_frame(cf1_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg, sizeof(receive_buffer));

  TEST_ASSERT_TRUE_MESSAGE(result, "Receive should succeed (returns 0)");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(12, msg.len, "Received length should be 12");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected_data, msg.data, 12, "Received data should match");

  // Проверяем, что был отправлен Flow Control
  TEST_ASSERT_TRUE_MESSAGE(mock_can.transmit_called, "Should send Flow Control");
  TEST_ASSERT_GREATER_THAN_INT_MESSAGE(
      0, mock_can.transmitted_frames.size(), "Should have transmitted FC");

  const auto& fc_sent = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x7DF, fc_sent.id, "FC should be sent to TX ID");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x30, fc_sent.data[0], "Should be Flow Control CTS");
}

// Тест 7: Обработка неправильной последовательности кадров
void test_iso_tp_receive_wrong_sequence() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[15];
  for (int i = 0; i < 15; i++) {
    test_data[i] = i + 0x50;
  }

  // Создаем First Frame
  IPhyInterface::TwaiFrame ff_frame = create_first_frame(0x7E8, 15, test_data);

  // Создаем CF с неправильной последовательностью (3 вместо 1)
  IPhyInterface::TwaiFrame cf_wrong = create_consecutive_frame(0x7E8, 3, &test_data[6], 7);

  mock_can.add_receive_frame(ff_frame);
  mock_can.add_receive_frame(cf_wrong);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg, sizeof(receive_buffer));

  TEST_ASSERT_FALSE_MESSAGE(result, "Receive should fail due to wrong sequence");
}

// Тест 8: Максимальная длина сообщения (4095 байт)
void test_iso_tp_send_max_length() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Создаем большой массив данных
  static uint8_t test_data[4095];
  for (int i = 0; i < 4095; i++) {
    test_data[i] = i & 0xFF;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Добавляем много FC кадров для поддержки длинной передачи
  for (int i = 0; i < 600; i++) {
    IPhyInterface::TwaiFrame fc_frame = create_flow_control_frame(0x456, 0, 0, 0);
    mock_can.add_receive_frame(fc_frame);
  }

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed for max length");
  TEST_ASSERT_GREATER_THAN_INT_MESSAGE(
      580, mock_can.transmitted_frames.size(), "Should transmit many frames");

  // Проверяем первый кадр
  const auto& ff = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x1F, ff.data[0], "FF should have max length in high nibble");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0xFF, ff.data[1], "FF should have max length in low byte");
}

// Тест 9: Separation Time в Flow Control
void test_iso_tp_send_with_separation_time() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[15];
  for (int i = 0; i < 15; i++) {
    test_data[i] = i + 0x60;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // FC с временем разделения 10ms
  IPhyInterface::TwaiFrame fc_frame = create_flow_control_frame(0x7E8, 0, 0, 10);
  mock_can.add_receive_frame(fc_frame);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with separation time");
  TEST_ASSERT_EQUAL_INT_MESSAGE(3, mock_can.transmitted_frames.size(), "Should transmit FF + 2 CF");
}

// Тест 10: Проверка корректности PCI для многокадровых сообщений
void test_iso_tp_multi_frame_pci_validation() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[50];
  for (int i = 0; i < 50; i++) {
    test_data[i] = i;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x100;
  msg.rx_id = 0x200;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Добавляем FC кадры
  for (int i = 0; i < 10; i++) {
    IPhyInterface::TwaiFrame fc_frame = create_flow_control_frame(0x200, 0, 0, 0);
    mock_can.add_receive_frame(fc_frame);
  }

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed");
  TEST_ASSERT_EQUAL_INT_MESSAGE(8, mock_can.transmitted_frames.size(), "Should transmit FF + 7 CF");

  // Проверяем последовательность PCI
  const auto& ff = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x10, ff.data[0] & 0xF0, "First frame PCI");

  for (int i = 1; i < 8; i++) {
    const auto& cf       = mock_can.transmitted_frames[i];
    uint8_t expected_seq = 0x20 | (i & 0x0F);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected_seq, cf.data[0], "CF sequence should be correct");
  }
}

// Функция запуска всех расширенных тестов
extern "C" void run_iso_tp_extended_tests() {
  // Тесты многокадровых сообщений
  RUN_TEST(test_iso_tp_send_multi_frame_8_bytes);
  RUN_TEST(test_iso_tp_send_multi_frame_15_bytes);

  // Тесты Flow Control
  RUN_TEST(test_iso_tp_send_flow_control_wait);
  RUN_TEST(test_iso_tp_send_flow_control_overflow);
  RUN_TEST(test_iso_tp_send_block_size_2);

  // Тесты приёма
  RUN_TEST(test_iso_tp_receive_multi_frame);
  RUN_TEST(test_iso_tp_receive_wrong_sequence);

  // Тесты граничных случаев
  RUN_TEST(test_iso_tp_send_max_length);
  RUN_TEST(test_iso_tp_send_with_separation_time);
  RUN_TEST(test_iso_tp_multi_frame_pci_validation);
}
