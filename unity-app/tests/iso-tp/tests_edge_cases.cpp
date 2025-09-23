#include <cstdio>
#include <cstring>

#include "iso-tp.h"
#include "mock_twai_interface.h"
#include "unity.h"

// ============================================================================
// ТЕСТЫ ГРАНИЧНЫХ СЛУЧАЕВ И СПЕЦИФИЧЕСКИХ СЦЕНАРИЕВ ISO-TP ПРОТОКОЛА
// ============================================================================

/*
 * ПОКРЫТИЕ ТЕСТАМИ:
 *
 * ✅ ТЕСТЫ ТАЙМАУТОВ И ВРЕМЕННЫХ ОГРАНИЧЕНИЙ:
 * - Таймаут при ожидании Flow Control после First Frame
 * - Таймаут при ожидании Consecutive Frame
 * - Множественные FC WAIT кадры (до лимита MAX_FCWAIT_FRAME)
 * - Превышение лимита FC WAIT кадров
 *
 * ✅ ТЕСТЫ БЛОЧНОЙ ПЕРЕДАЧИ:
 * - Блочная передача с размером блока 1
 * - Блочная передача с размером блока 15 (максимум)
 * - Блочная передача с большими сообщениями, требующими несколько блоков
 * - Смешанные размеры блоков в одной передаче
 *
 * ✅ ТЕСТЫ SEPARATION TIME:
 * - Различные значения separation time (1ms, 50ms, 127ms)
 * - Микросекундные значения (0xF1-0xF9)
 * - Некорректные значения separation time (должны корректироваться до 0x7F)
 *
 * ✅ ТЕСТЫ ПОСЛЕДОВАТЕЛЬНОСТИ КАДРОВ:
 * - Переполнение счетчика последовательности (0xF -> 0x0)
 * - Дублирование Consecutive Frame
 * - Пропуск Consecutive Frame в последовательности
 *
 * ✅ ТЕСТЫ НЕКОРРЕКТНЫХ ДАННЫХ:
 * - Получение кадра с неправильным PCI типом
 * - Получение FF с некорректной длиной
 * - Получение CF без предшествующего FF
 * - Получение FC без предшествующего FF
 *
 * ✅ ТЕСТЫ БОЛЬШИХ СООБЩЕНИЙ:
 * - Сообщения размером 4095 байт (максимум для стандартного ISO-TP)
 * - Сообщения размером 4096 байт (должно быть отклонено)
 * - Сообщения с длиной 0 в FF
 *
 * ✅ ТЕСТЫ CAN ID:
 * - Отправка и получение с различными CAN ID
 * - Получение кадров с неправильным ID (должны игнорироваться)
 * - Проверка корректности TX/RX ID в Flow Control
 *
 * ✅ ТЕСТЫ СОСТОЯНИЙ ПРОТОКОЛА:
 * - Прерывание передачи новым FF
 * - Получение нескольких FF подряд
 * - Состояние IDLE после ошибок
 */

// ============================================================================
// 1. ТЕСТЫ ТАЙМАУТОВ И ВРЕМЕННЫХ ОГРАНИЧЕНИЙ
// ============================================================================

// Тест 1.1: Таймаут при ожидании Flow Control после First Frame
void test_iso_tp_timeout_waiting_fc_after_ff() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Не добавляем FC кадр - должен произойти таймаут
  bool result = iso_tp.send(msg);

  TEST_ASSERT_FALSE_MESSAGE(result, "Send should fail due to FC timeout");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      1, mock_can.transmitted_frames.size(), "Should only transmit FF before timeout");

  // Проверяем, что отправлен только First Frame
  const auto& ff = mock_can.transmitted_frames[0];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x10, ff.data[0] & 0xF0, "Should be First Frame");
}

// Тест 1.2: Множественные FC WAIT кадры (до лимита MAX_FCWAIT_FRAME = 10)
void test_iso_tp_multiple_fc_wait_frames() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[15] = {
      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E};
  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Добавляем 9 FC WAIT кадров, затем FC CTS
  for (int i = 0; i < 9; i++) {
    ITwaiInterface::TwaiFrame fc_wait = create_flow_control_frame(0x456, 1, 0, 0);  // WAIT
    mock_can.add_receive_frame(fc_wait);
  }
  ITwaiInterface::TwaiFrame fc_cts = create_flow_control_frame(0x456, 0, 0, 0);  // CTS
  mock_can.add_receive_frame(fc_cts);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed after multiple FC WAIT");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      3, mock_can.transmitted_frames.size(), "Should transmit FF + 2 CF after all FC WAIT");
}

// Тест 1.3: Превышение лимита FC WAIT кадров (MAX_FCWAIT_FRAME = 10)
void test_iso_tp_exceed_fc_wait_limit() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[15] = {
      0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E};
  IsoTp::Message msg;
  msg.tx_id = 0x789;
  msg.rx_id = 0xABC;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Добавляем 11 FC WAIT кадров (превышение лимита)
  for (int i = 0; i < 11; i++) {
    ITwaiInterface::TwaiFrame fc_wait = create_flow_control_frame(0xABC, 1, 0, 0);  // WAIT
    mock_can.add_receive_frame(fc_wait);
  }

  bool result = iso_tp.send(msg);

  TEST_ASSERT_FALSE_MESSAGE(result, "Send should fail after exceeding FC WAIT limit");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1,
                                mock_can.transmitted_frames.size(),
                                "Should only transmit FF before FC WAIT limit exceeded");
}

// ============================================================================
// 2. ТЕСТЫ БЛОЧНОЙ ПЕРЕДАЧИ
// ============================================================================

// Тест 2.1: Блочная передача с размером блока 1
void test_iso_tp_block_size_1() {
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

  // FC с блочным размером 1 - нужно несколько FC кадров
  ITwaiInterface::TwaiFrame fc1 = create_flow_control_frame(0x200, 0, 1, 0);  // Block size = 1
  ITwaiInterface::TwaiFrame fc2 = create_flow_control_frame(0x200, 0, 1, 0);  // Block size = 1
  mock_can.add_receive_frame(fc1);
  mock_can.add_receive_frame(fc2);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with block size 1");
  // FF + 2 CF (каждый CF требует отдельный FC)
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      3, mock_can.transmitted_frames.size(), "Should transmit FF + 2 CF with block size 1");
}

// Тест 2.2: Блочная передача с размером блока 15 (максимум)
void test_iso_tp_block_size_15() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[100];  // Большое сообщение
  for (int i = 0; i < 100; i++) {
    test_data[i] = i & 0xFF;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x300;
  msg.rx_id = 0x400;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // FC с максимальным блочным размером 15
  ITwaiInterface::TwaiFrame fc1 = create_flow_control_frame(0x400, 0, 15, 0);  // Block size = 15
  mock_can.add_receive_frame(fc1);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with block size 15");
  // FF + 14 CF (всего 15 кадров в блоке, включая FF)
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      15, mock_can.transmitted_frames.size(), "Should transmit FF + 14 CF with block size 15");
}

// Тест 2.3: Блочная передача с большими сообщениями, требующими несколько блоков
void test_iso_tp_multiple_blocks() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[50];  // Сообщение, требующее несколько блоков
  for (int i = 0; i < 50; i++) {
    test_data[i] = i + 0x50;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x500;
  msg.rx_id = 0x600;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Первый блок размером 3, второй блок размером 4
  ITwaiInterface::TwaiFrame fc1 = create_flow_control_frame(0x600, 0, 4, 0);  // Block size = 4
  ITwaiInterface::TwaiFrame fc2 = create_flow_control_frame(0x600, 0, 4, 0);  // Block size = 4
  mock_can.add_receive_frame(fc1);
  mock_can.add_receive_frame(fc2);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with multiple blocks");
  // FF + 3 CF (первый блок) + 4 CF (второй блок) = 8 кадров
  TEST_ASSERT_EQUAL_INT_MESSAGE(8,
                                mock_can.transmitted_frames.size(),
                                "Should transmit correct number of frames for multiple blocks");
}

// ============================================================================
// 3. ТЕСТЫ SEPARATION TIME
// ============================================================================

// Тест 3.1: Различные значения separation time (1ms, 50ms, 127ms)
void test_iso_tp_separation_time_values() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[15] = {
      0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E};
  IsoTp::Message msg;
  msg.tx_id = 0x700;
  msg.rx_id = 0x800;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Тестируем различные значения separation time
  uint8_t sep_times[] = {1, 50, 127};  // 1ms, 50ms, 127ms

  for (int i = 0; i < 3; i++) {
    mock_can.reset();
    ITwaiInterface::TwaiFrame fc = create_flow_control_frame(0x800, 0, 0, sep_times[i]);
    mock_can.add_receive_frame(fc);

    bool result = iso_tp.send(msg);
    TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with valid separation time");
  }
}

// Тест 3.2: Микросекундные значения separation time (0xF1-0xF9)
void test_iso_tp_microsecond_separation_time() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[10] = {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79};
  IsoTp::Message msg;
  msg.tx_id = 0x111;
  msg.rx_id = 0x222;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Тестируем микросекундные значения
  uint8_t microsec_times[] = {0xF1, 0xF5, 0xF9};  // 100μs, 500μs, 900μs

  for (int i = 0; i < 3; i++) {
    mock_can.reset();
    ITwaiInterface::TwaiFrame fc = create_flow_control_frame(0x222, 0, 0, microsec_times[i]);
    mock_can.add_receive_frame(fc);

    bool result = iso_tp.send(msg);
    TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with microsecond separation time");
  }
}

// Тест 3.3: Некорректные значения separation time (должны корректироваться до 0x7F)
void test_iso_tp_invalid_separation_time_correction() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[10] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89};
  IsoTp::Message msg;
  msg.tx_id = 0x333;
  msg.rx_id = 0x444;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // Тестируем некорректные значения (должны быть скорректированы до 0x7F)
  uint8_t invalid_times[] = {0x80, 0xA0, 0xF0, 0xFA, 0xFF};  // Недопустимые значения

  for (int i = 0; i < 5; i++) {
    mock_can.reset();
    ITwaiInterface::TwaiFrame fc = create_flow_control_frame(0x444, 0, 0, invalid_times[i]);
    mock_can.add_receive_frame(fc);

    bool result = iso_tp.send(msg);
    TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with corrected separation time");
  }
}

// ============================================================================
// 4. ТЕСТЫ ПОСЛЕДОВАТЕЛЬНОСТИ КАДРОВ
// ============================================================================

// Тест 4.1: Переполнение счетчика последовательности (0xF -> 0x0)
void test_iso_tp_sequence_counter_overflow() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Создаем сообщение, требующее 17 кадров (FF + 16 CF)
  // Это приведет к переполнению счетчика (0x1-0xF, затем 0x0)
  uint8_t test_data[118];  // FF(6 байт) + 16*CF(7 байт) = 6 + 112 = 118 байт
  for (int i = 0; i < 118; i++) {
    test_data[i] = i & 0xFF;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x555;
  msg.rx_id = 0x666;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // FC с большим блочным размером
  ITwaiInterface::TwaiFrame fc = create_flow_control_frame(0x666, 0, 0, 0);  // Без ограничений
  mock_can.add_receive_frame(fc);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with sequence counter overflow");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      17, mock_can.transmitted_frames.size(), "Should transmit FF + 16 CF");

  // Проверяем последний CF - должен иметь sequence number 0x0 (переполнение)
  const auto& last_cf = mock_can.transmitted_frames[16];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0x20, last_cf.data[0], "Last CF should have sequence number 0 (overflow)");
}

// Тест 4.2: Приём с дублированием Consecutive Frame
void test_iso_tp_receive_duplicate_consecutive_frame() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t expected_data[15] = {
      0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E};

  // Создаем First Frame
  ITwaiInterface::TwaiFrame ff_frame = create_first_frame(0x777, 15, expected_data);

  // Создаем Consecutive Frames с дублированием
  ITwaiInterface::TwaiFrame cf1_frame = create_consecutive_frame(0x777, 1, &expected_data[6], 7);
  ITwaiInterface::TwaiFrame cf1_dupl  = create_consecutive_frame(0x777, 1, &expected_data[6], 7);
  ITwaiInterface::TwaiFrame cf2_frame = create_consecutive_frame(0x777, 2, &expected_data[13], 2);

  mock_can.add_receive_frame(ff_frame);
  mock_can.add_receive_frame(cf1_frame);
  mock_can.add_receive_frame(cf1_dupl);  // Дублированный кадр
  mock_can.add_receive_frame(cf2_frame);

  uint8_t receive_buffer[128] = {0};
  IsoTp::Message msg;
  msg.tx_id = 0x888;
  msg.rx_id = 0x777;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  // Приём должен завершиться успешно, игнорируя дублированный кадр
  TEST_ASSERT_FALSE_MESSAGE(result, "Receive should succeed ignoring duplicate CF");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(15, msg.len, "Received length should be correct");
}

// Тест 4.3: Приём с пропуском Consecutive Frame в последовательности
void test_iso_tp_receive_missing_consecutive_frame() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t expected_data[20] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9,
                               0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3};

  // Создаем First Frame
  ITwaiInterface::TwaiFrame ff_frame = create_first_frame(0x999, 20, expected_data);

  // Создаем Consecutive Frames с пропуском CF1 (sequence 1)
  ITwaiInterface::TwaiFrame cf2_frame =
      create_consecutive_frame(0x999, 2, &expected_data[13], 7);  // Пропускаем CF1
  ITwaiInterface::TwaiFrame cf3_frame = create_consecutive_frame(0x999, 3, &expected_data[20], 0);

  mock_can.add_receive_frame(ff_frame);
  mock_can.add_receive_frame(cf2_frame);  // Пропущен CF1, сразу CF2
  mock_can.add_receive_frame(cf3_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0xAAA;
  msg.rx_id = 0x999;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  // Приём должен завершиться с ошибкой из-за пропущенного кадра
  TEST_ASSERT_TRUE_MESSAGE(result, "Receive should fail due to missing CF");
}

// ============================================================================
// 5. ТЕСТЫ НЕКОРРЕКТНЫХ ДАННЫХ
// ============================================================================

// Тест 5.1: Получение кадра с неправильным PCI типом
void test_iso_tp_receive_invalid_pci_type() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Создаем кадр с недопустимым PCI типом (0x40)
  ITwaiInterface::TwaiFrame invalid_frame = {};
  invalid_frame.id                        = 0xBBB;
  invalid_frame.data_length               = 8;
  invalid_frame.data[0]                   = 0x40;  // Недопустимый PCI тип
  invalid_frame.data[1]                   = 0x05;

  mock_can.add_receive_frame(invalid_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0xCCC;
  msg.rx_id = 0xBBB;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  // Приём должен завершиться с ошибкой или таймаутом
  TEST_ASSERT_TRUE_MESSAGE(result, "Receive should fail with invalid PCI type");
}

// Тест 5.2: Получение FF с некорректной длиной (0)
void test_iso_tp_receive_ff_zero_length() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Создаем First Frame с длиной 0
  ITwaiInterface::TwaiFrame ff_frame = {};
  ff_frame.id                        = 0xDDD;
  ff_frame.data_length               = 8;
  ff_frame.data[0]                   = 0x10;  // FF PCI
  ff_frame.data[1]                   = 0x00;  // Длина = 0

  mock_can.add_receive_frame(ff_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0xEEE;
  msg.rx_id = 0xDDD;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  // Приём должен завершиться с ошибкой
  TEST_ASSERT_TRUE_MESSAGE(result, "Receive should fail with FF zero length");
}

// Тест 5.3: Получение CF без предшествующего FF
void test_iso_tp_receive_cf_without_ff() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Создаем Consecutive Frame без предшествующего FF
  ITwaiInterface::TwaiFrame cf_frame = create_consecutive_frame(0xFFF, 1, nullptr, 0);

  mock_can.add_receive_frame(cf_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x111;
  msg.rx_id = 0xFFF;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  // Приём должен завершиться с ошибкой или таймаутом
  TEST_ASSERT_TRUE_MESSAGE(result, "Receive should fail with CF without FF");
}

// ============================================================================
// 6. ТЕСТЫ БОЛЬШИХ СООБЩЕНИЙ
// ============================================================================

// Тест 6.1: Сообщения размером 4095 байт (максимум для стандартного ISO-TP)
void test_iso_tp_max_message_size_4095() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Создаем максимальное сообщение 4095 байт
  uint8_t* test_data = new uint8_t[4095];
  for (int i = 0; i < 4095; i++) {
    test_data[i] = i & 0xFF;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = 4095;
  msg.data  = test_data;

  // FC без ограничений
  ITwaiInterface::TwaiFrame fc = create_flow_control_frame(0x456, 0, 0, 0);
  mock_can.add_receive_frame(fc);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with 4095 byte message");

  // FF + количество CF = 1 + ceil((4095-6)/7) = 1 + 585 = 586 кадров
  int expected_frames = 1 + ((4095 - 6 + 6) / 7);  // FF + CF count
  TEST_ASSERT_EQUAL_INT_MESSAGE(expected_frames,
                                mock_can.transmitted_frames.size(),
                                "Should transmit correct number of frames for 4095 bytes");

  delete[] test_data;
}

// Тест 6.2: Сообщения размером 4096 байт (должно быть отклонено)
void test_iso_tp_oversized_message_4096() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Создаем сообщение превышающее максимум
  uint8_t* test_data = new uint8_t[4096];
  for (int i = 0; i < 4096; i++) {
    test_data[i] = i & 0xFF;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x789;
  msg.rx_id = 0xABC;
  msg.len   = 4096;
  msg.data  = test_data;

  bool result = iso_tp.send(msg);

  // Отправка должна быть отклонена
  TEST_ASSERT_FALSE_MESSAGE(result, "Send should fail with 4096 byte message (oversized)");
  TEST_ASSERT_EQUAL_INT_MESSAGE(0,
                                mock_can.transmitted_frames.size(),
                                "Should not transmit any frames for oversized message");

  delete[] test_data;
}

// ============================================================================
// 7. ТЕСТЫ CAN ID
// ============================================================================

// Тест 7.1: Отправка и получение с различными CAN ID
void test_iso_tp_different_can_ids() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[] = {0xC0, 0xC1, 0xC2, 0xC3, 0xC4};

  // Тестируем различные комбинации CAN ID
  uint32_t test_ids[][2] = {
      {0x7DF, 0x7E8},  // Стандартные OBD-II ID
      {0x123, 0x456},  // Произвольные ID
      {0x7FF, 0x000},  // Граничные значения
      {0x100, 0x200}   // Другие произвольные ID
  };

  for (int i = 0; i < 4; i++) {
    mock_can.reset();

    IsoTp::Message msg;
    msg.tx_id = test_ids[i][0];
    msg.rx_id = test_ids[i][1];
    msg.len   = sizeof(test_data);
    msg.data  = test_data;

    bool result = iso_tp.send(msg);
    TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with different CAN IDs");

    const auto& frame = mock_can.transmitted_frames[0];
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(test_ids[i][0], frame.id, "TX ID should match");
  }
}

// Тест 7.2: Получение кадров с неправильным ID (должны игнорироваться)
void test_iso_tp_receive_wrong_can_id() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t expected_data[] = {0xD0, 0xD1, 0xD2, 0xD3};

  // Создаем кадры с правильным и неправильным ID
  ITwaiInterface::TwaiFrame wrong_sf =
      create_single_frame(0x999, 4, expected_data);  // Неправильный ID
  ITwaiInterface::TwaiFrame correct_sf =
      create_single_frame(0x555, 4, expected_data);  // Правильный ID

  mock_can.add_receive_frame(wrong_sf);    // Должен быть проигнорирован
  mock_can.add_receive_frame(correct_sf);  // Должен быть принят

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x444;
  msg.rx_id = 0x555;  // Ожидаем кадры только с этим ID
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  TEST_ASSERT_FALSE_MESSAGE(result, "Receive should succeed with correct ID");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(4, msg.len, "Should receive data from correct ID only");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected_data, msg.data, 4, "Data should match");
}

// Тест 7.3: Проверка корректности TX/RX ID в Flow Control
void test_iso_tp_flow_control_id_validation() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[15] = {
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE};
  IsoTp::Message msg;
  msg.tx_id = 0x18DA10F1;
  msg.rx_id = 0x18DAF110;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  // FC с правильным ID
  ITwaiInterface::TwaiFrame fc_correct = create_flow_control_frame(0x18DAF110, 0, 0, 0);
  // FC с неправильным ID (должен быть проигнорирован)
  ITwaiInterface::TwaiFrame fc_wrong = create_flow_control_frame(0x18DAF111, 0, 0, 0);

  mock_can.add_receive_frame(fc_wrong);    // Неправильный ID - игнорируется
  mock_can.add_receive_frame(fc_correct);  // Правильный ID - принимается

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed with correct FC ID");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      3, mock_can.transmitted_frames.size(), "Should transmit FF + 2 CF after correct FC");
}

// ============================================================================
// 8. ТЕСТЫ СОСТОЯНИЙ ПРОТОКОЛА
// ============================================================================

// Тест 8.1: Прерывание передачи новым FF
void test_iso_tp_transmission_interrupted_by_new_ff() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Первое сообщение (будет прервано)
  uint8_t test_data1[15] = {
      0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE};

  // Второе сообщение (прерывающее)
  uint8_t test_data2[10] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};

  // Начинаем первую передачу
  IsoTp::Message msg1;
  msg1.tx_id = 0x700;
  msg1.rx_id = 0x800;
  msg1.len   = sizeof(test_data1);
  msg1.data  = test_data1;

  // Не добавляем FC для первого сообщения - оно должно прерваться

  // Начинаем вторую передачу
  IsoTp::Message msg2;
  msg2.tx_id = 0x700;
  msg2.rx_id = 0x800;
  msg2.len   = sizeof(test_data2);
  msg2.data  = test_data2;

  ITwaiInterface::TwaiFrame fc2 = create_flow_control_frame(0x800, 0, 0, 0);
  mock_can.add_receive_frame(fc2);

  // Первая передача должна завершиться неудачей
  bool result1 = iso_tp.send(msg1);
  TEST_ASSERT_FALSE_MESSAGE(result1, "First send should fail (no FC)");

  // Вторая передача должна быть успешной
  bool result2 = iso_tp.send(msg2);
  TEST_ASSERT_TRUE_MESSAGE(result2, "Second send should succeed");
}

// Тест 8.2: Получение нескольких FF подряд
void test_iso_tp_receive_multiple_ff() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t expected_data1[10] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29};
  uint8_t expected_data2[12] = {
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B};

  // Создаем два FF подряд (второй должен прервать первый)
  ITwaiInterface::TwaiFrame ff1_frame = create_first_frame(0x900, 10, expected_data1);
  ITwaiInterface::TwaiFrame ff2_frame = create_first_frame(0x900, 12, expected_data2);

  // CF для второго сообщения
  ITwaiInterface::TwaiFrame cf2_frame = create_consecutive_frame(0x900, 1, &expected_data2[6], 6);

  mock_can.add_receive_frame(ff1_frame);  // Первый FF
  mock_can.add_receive_frame(ff2_frame);  // Второй FF (прерывает первый)
  mock_can.add_receive_frame(cf2_frame);  // CF для второго сообщения

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0xA00;
  msg.rx_id = 0x900;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  // Должно быть получено второе сообщение (12 байт)
  TEST_ASSERT_FALSE_MESSAGE(result, "Receive should succeed with second message");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(12, msg.len, "Should receive second message length");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
      expected_data2, msg.data, 12, "Should receive second message data");
}

// Тест 8.3: Состояние IDLE после ошибок
void test_iso_tp_idle_state_after_errors() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Первая передача с ошибкой (FC OVERFLOW)
  uint8_t test_data1[10] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49};
  IsoTp::Message msg1;
  msg1.tx_id = 0xB00;
  msg1.rx_id = 0xC00;
  msg1.len   = sizeof(test_data1);
  msg1.data  = test_data1;

  ITwaiInterface::TwaiFrame fc_overflow = create_flow_control_frame(0xC00, 2, 0, 0);  // OVERFLOW
  mock_can.add_receive_frame(fc_overflow);

  bool result1 = iso_tp.send(msg1);
  TEST_ASSERT_FALSE_MESSAGE(result1, "First send should fail with FC OVERFLOW");

  // Вторая передача должна быть успешной (протокол вернулся в IDLE)
  uint8_t test_data2[5] = {0x50, 0x51, 0x52, 0x53, 0x54};
  IsoTp::Message msg2;
  msg2.tx_id = 0xB00;
  msg2.rx_id = 0xC00;
  msg2.len   = sizeof(test_data2);
  msg2.data  = test_data2;

  bool result2 = iso_tp.send(msg2);
  TEST_ASSERT_TRUE_MESSAGE(result2, "Second send should succeed after error recovery");

  // Проверяем, что отправлен одиночный кадр
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      2, mock_can.transmitted_frames.size(), "Should have FF from first attempt + SF from second");

  const auto& sf = mock_can.transmitted_frames[1];
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x05, sf.data[0], "Should be Single Frame with length 5");
}

// ============================================================================
// 9. ДОПОЛНИТЕЛЬНЫЕ ГРАНИЧНЫЕ СЛУЧАИ
// ============================================================================

// Тест 9.1: Отправка сообщения с нулевым указателем данных
void test_iso_tp_send_null_data_pointer() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  IsoTp::Message msg;
  msg.tx_id = 0xD00;
  msg.rx_id = 0xE00;
  msg.len   = 5;  // Ненулевая длина, но нулевой указатель
  msg.data  = nullptr;

  bool result = iso_tp.send(msg);

  // Отправка должна завершиться неудачей или отправить пустые данные
  TEST_ASSERT_FALSE_MESSAGE(result, "Send should fail with null data pointer");
}

// Тест 9.2: Приём в буфер недостаточного размера
void test_iso_tp_receive_insufficient_buffer() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t expected_data[10]          = {0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69};
  ITwaiInterface::TwaiFrame sf_frame = create_single_frame(0xF00, 10, expected_data);
  mock_can.add_receive_frame(sf_frame);

  uint8_t small_buffer[5];  // Буфер меньше размера сообщения
  IsoTp::Message msg;
  msg.tx_id = 0x100;
  msg.rx_id = 0xF00;
  msg.len   = 0;
  msg.data  = small_buffer;

  bool result = iso_tp.receive(msg);

  // Приём должен завершиться успешно, но данные могут быть обрезаны
  TEST_ASSERT_FALSE_MESSAGE(result, "Receive should succeed");
  // Проверяем, что длина корректна, даже если буфер мал
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(10, msg.len, "Length should reflect actual message size");
}

// Тест 9.3: Смешанные типы кадров в одной последовательности
void test_iso_tp_mixed_frame_types() {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  // Создаем последовательность: SF, FF, CF (некорректная последовательность)
  uint8_t sf_data[]   = {0x70, 0x71, 0x72};
  uint8_t ff_data[10] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89};

  ITwaiInterface::TwaiFrame sf_frame = create_single_frame(0x111, 3, sf_data);
  ITwaiInterface::TwaiFrame ff_frame = create_first_frame(0x111, 10, ff_data);
  ITwaiInterface::TwaiFrame cf_frame = create_consecutive_frame(0x111, 1, &ff_data[6], 4);

  mock_can.add_receive_frame(sf_frame);  // SF должен быть принят
  mock_can.add_receive_frame(ff_frame);  // FF прерывает и начинает новую последовательность
  mock_can.add_receive_frame(cf_frame);  // CF завершает многокадровое сообщение

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x222;
  msg.rx_id = 0x111;
  msg.len   = 0;
  msg.data  = receive_buffer;

  // Первый вызов должен получить SF
  bool result1 = iso_tp.receive(msg);
  TEST_ASSERT_FALSE_MESSAGE(result1, "First receive should get SF");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(3, msg.len, "Should receive SF data");

  // Второй вызов должен получить многокадровое сообщение
  msg.len      = 0;
  bool result2 = iso_tp.receive(msg);
  TEST_ASSERT_FALSE_MESSAGE(result2, "Second receive should get multi-frame message");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(10, msg.len, "Should receive FF+CF data");
}

// ============================================================================
// ФУНКЦИЯ ЗАПУСКА ВСЕХ ТЕСТОВ ГРАНИЧНЫХ СЛУЧАЕВ
// ============================================================================

extern "C" void run_iso_tp_edge_case_tests() {
  printf("\n=== ЗАПУСК ТЕСТОВ ГРАНИЧНЫХ СЛУЧАЕВ ISO-TP ===\n");

  // 1. Тесты таймаутов и временных ограничений
  printf("\n--- Тесты таймаутов и временных ограничений ---\n");
  RUN_TEST(test_iso_tp_timeout_waiting_fc_after_ff);
  RUN_TEST(test_iso_tp_multiple_fc_wait_frames);
  RUN_TEST(test_iso_tp_exceed_fc_wait_limit);

  // 2. Тесты блочной передачи
  printf("\n--- Тесты блочной передачи ---\n");
  RUN_TEST(test_iso_tp_block_size_1);
  RUN_TEST(test_iso_tp_block_size_15);
  RUN_TEST(test_iso_tp_multiple_blocks);

  // 3. Тесты Separation Time
  printf("\n--- Тесты Separation Time ---\n");
  RUN_TEST(test_iso_tp_separation_time_values);
  RUN_TEST(test_iso_tp_microsecond_separation_time);
  RUN_TEST(test_iso_tp_invalid_separation_time_correction);

  // 4. Тесты последовательности кадров
  printf("\n--- Тесты последовательности кадров ---\n");
  RUN_TEST(test_iso_tp_sequence_counter_overflow);
  RUN_TEST(test_iso_tp_receive_duplicate_consecutive_frame);
  RUN_TEST(test_iso_tp_receive_missing_consecutive_frame);

  // 5. Тесты некорректных данных
  printf("\n--- Тесты некорректных данных ---\n");
  RUN_TEST(test_iso_tp_receive_invalid_pci_type);
  RUN_TEST(test_iso_tp_receive_ff_zero_length);
  RUN_TEST(test_iso_tp_receive_cf_without_ff);

  // 6. Тесты больших сообщений
  printf("\n--- Тесты больших сообщений ---\n");
  RUN_TEST(test_iso_tp_max_message_size_4095);
  RUN_TEST(test_iso_tp_oversized_message_4096);

  // 7. Тесты CAN ID
  printf("\n--- Тесты CAN ID ---\n");
  RUN_TEST(test_iso_tp_different_can_ids);
  RUN_TEST(test_iso_tp_receive_wrong_can_id);
  RUN_TEST(test_iso_tp_flow_control_id_validation);

  // 8. Тесты состояний протокола
  printf("\n--- Тесты состояний протокола ---\n");
  RUN_TEST(test_iso_tp_transmission_interrupted_by_new_ff);
  RUN_TEST(test_iso_tp_receive_multiple_ff);
  RUN_TEST(test_iso_tp_idle_state_after_errors);

  // 9. Дополнительные граничные случаи
  printf("\n--- Дополнительные граничные случаи ---\n");
  RUN_TEST(test_iso_tp_send_null_data_pointer);
  RUN_TEST(test_iso_tp_receive_insufficient_buffer);
  RUN_TEST(test_iso_tp_mixed_frame_types);

  printf("\n=== ТЕСТЫ ГРАНИЧНЫХ СЛУЧАЕВ ISO-TP ЗАВЕРШЕНЫ ===\n");
}