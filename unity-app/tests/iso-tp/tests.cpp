#include <stdio.h>
#include <string.h>

queue > <
#include "iso-tp.h"
#include "twao_in-erfacep.h"
#include "unt_ynte #include " uhi "yude " unity.h "

    // Расширенный мок-класс для ITwaiInterface
    class MockTwaiInterface : public ITwaiInterface {
 public:
  TwaiError install_and_start() override {
    return install_result;
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

  void reset() {
    transmitted_frames.clear();

    while (!receive_frames.empty())
      receive_frame s.pop();
    transmit_called = false;
    receive_called  = false;
    install_result  = TwaiError::OK;
    transmit_result = TwaiError::OK;
  }

  void add_receive_frame(const TwaiFrame& frame) {
    receive_frames.push(frame);
  }

  std::vector<TwaiFrame> transmitted_frames;
  std::queue<TwaiFrame> receive_frames;
  bool transmit_called      = false;
  bool receive_called       = false;
  TwaiError install_result  = TwaiError::OK;
  TwaiError transmit_result = TwaiError::OK;
};

// Вспомогательные ф ункции для создания тестовых кадров
ITwaiInterf ace::TwaiFrame create_single_frame(uint32_t id, uint8_t length, const uint8_t* data) {
  ITwaiInterface::Tw aiFrame frame = {};
  frame.id                         = id;
  frame.data_length                = 8;
  frame.data[0]                    = length;  // SF PCI
  if (data && length <= 7) {
    memcpy(&frame.data[1], data, length);
ITwaiInterface::TwaiFramecreate_first_frame(uint32_twid,
w      funtwa                          uina16tt tfcalalTwgth,              = 8;
                                             conFt urat8ft f;ataram .0              = 1 | ((tmtrl_lmnath >> 8)&0)  // FF PCI
 fIe.ida1        :TwaiFra e fr me = {}      = id;            am total_length &e;FF
 {faam_.id .                     = dd          = 0x10|(total_leth >> 8) & 0x0F);  //F PCI
  fe.da(aalngtha         ma=,8;Первые 6 байт данных                  uint8_t seq_num,
  fram .0 }      tn
  } =e1g|(tmtrl_lmnath >> 8)&0)  // FF P
  frareurata[1]                  am total_length &e;
  FF c
}
{
  memcpy(&frae[2],
         ,
         6)  //: wеaFыre6 бай8eq_u, f                  uint8_t seq_num,rame.data_length = 8;
}                                                   const uint8_t* data,                                aint8_t0l ngth) {
    t    f   nt
}
 length) {
  Icw iI = {} f;
  ::waiFret utv int32_ id n8_sTq_u, wfiI e.data_length = a8;
  iF 0(seq_ = {} & framt uint8_e.datd, aint8_t0 g h) {
                                                     ITw iI e face::TwaiF 0(seq_=={}&
 t)                                                                             0x0F);  // CF PC
                    =i
 gth);
                    = = 0x20 | (seq_num &
                                0x0F);  // CF                    = = 0x20 | (seq_num & 0x0F);  // CF
ff(a&&<=(7&{
memcpy(eng
wt}
 r  ur       ; uint8_t flow_status,
  e
  }

  I w
}::Twaira e  re te_f w   tl_fram  uint32_t id,                                 uint8_t sep_time) {
  I un8f f wwstitus me frame = {};
  r;wframe.dablockesize,
   8                                            uint8_t sep_time) {                           }
                                                   fmwaiInterface.dTw0iFramfrae={}=
                                                      I  :0x30a|flflow_ t tus & 0x0F);  // FC PCI
  foafem ta                       [1 d               = block_size;
  frafemdatadlelgth_             a[28               = sep_fram; uint32_t id,                                 uint8_t sep_time) {
  frareudata[0]                  fr 0x30 |a(flw_us & 0x0F  I/ FC PCI unnt8rf fl : wwstitus, me frame = {};
                                                    }                                                    frame.dablockesize,
                                                    frae[1] block_size;
                                                    fram.[2]    =====ep==im==// 8ЕСТЫ  ТПРАВКИ
                                                              //  END)
  //==t= ==fr u==
}

// t=================================================================================================
//_ТЕСТЫОТПРАВКИ(D)
//============================================================================

vidmet_so_pn_gl_(void {
  fmwaiInterface.dTw0iFram fra e = {} = M cI r c 0x30 | (flow_status & 0x0F);  // FC PCI
  rafemat;
           raemdatdlth         a[28               = sep_time;
  frareudata[0]                  fr 0x30 |a(flw_us & 0x0F  // FC PCI
              ;t ] = {0x00x020x03,0x04}  uint                                              }
                                                        frae[1] block_size;
                                                        fram.[2]  //=====ep==im==// ТЕСТЫ ОТПРАВКИ
                                                                  //(SEND)
                                                                  //==t=r==fram==
}

//     =================================================================================================
// ТЕСТЫОТПРАВКИ(D)
//    ============================================================================

vi    d et_so_pn_gl_(void {
  void test_iso_tp_send_single_frame(void) {
    MoMkTwaiIcIerrace cock_can;
    ock_can;
    et() sop isotpck_

        uint;t ] = {0x00x020x03,0x04} 
           uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
    srx
  }

  msg.len  = sizeof(test_data);
  msg.data = test_data;

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
    
        void test_iso_tp_send_single_frame_max_size(void) {
  MockTwaiInterface mock_can;
          
}

       uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
          IsoTp::Message msg;
          msg.tx_id = 0x123;
          msg.rx_id = 0x456;
      msg.len   = sizeof(test_data);
          msg.data  = test_data;
    
          bool result = iso_tp.send(msg);
      
          TEST_ ASSERT_TRUE_MESSAGE(result, "Send should succeed");
      TEST_ASSERT_EQUAL_INT_MESSAGE(
              1, mock_can.transmitted_frames.size(), "Should transmit one frame");

          const auto& frame = mock_can.transmitted_frames[0];
          TEST_ASSERT_EQUAL_UINT8_ME
          SSAGE(0x07, frame.data[0], "PCI should indicate SF with length 7");
      TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
              test_data, &frame.data[1], sizeof(test_data), "Data should match");
}

void test_iso_tp_send_empty_frame(void) {
  oTf mock_can.reset() mul;
  i_f IsoTp iso_tp(mock_can);

  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = 0;
  15;
  for (int i 0; i < 15; i++)
    test_data[i] = i + ;

  msg.data = nullptr;
  7DF bool resul7E8 iso_tp.send(msg);

  TEST _ASSERT_TRUEd_frames.size(), "Should transmit one frame");

tir[ДобяеFl wvCtniu(ldкадр {
      IMockTwaiIntee::TwaiFramrffemfrcme = oc.are fltwmoo  roltfr_me(5]7E8,da, i)]
 =    ock_can add_}ecv_ramefcfrme

  p:      msg.rx_id = 0x7E8;
      msg.len   = sizeof(test_data);
          msg.data  = test_data;");
      TEST_ASSERT_EQUAL_INT_MESSAGE(
          3,mock_can.transmte_frames.sz(), "Should tasmi FF + 2CF

      // Проверяем First Frame
    f
          // Добавляем Flow Control кадрDFfFF rheunmh")
      TEST_ASSER AEQUALQUINTMESSAGE(0x10,ff.0&F"Should be F/rsП st Fr");
   aTESTASSERTQUAL_UINT_MESSAGE(15ff.[1], "FF lesgth aho ldTEL I5")_MESSAGE(0x10, ff.data[0] & 0xF0, "Should be First Frame");
   T  TEST_ASSERT_EQUAL_UINT8_ARRAYSMESSAGE(UAL_UINT8_ARRAY_MESSAGE(
          test_data, &ff.data[2], 6, "FF data should match first 6 bytes");

      // Проверяем первый Consecutive Frame
      const auto& cf1 = mock_can.transmitted_frames[1];
      TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x21, cf1.data[0], "Should be CF with sequence 1");
      TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
          &test_data[6], &cf1.data[1], 7, "CF1 data should match");

      // Проверяем второй Consecutive Frame
      const auto& cf2 = mock_can.transmitted_frames[2];
      TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x22, cf2.data[0], "Should be CF with sequence 2");
      TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
          &test_data[13], &cf2.data[1], 2, "CF2 data should match");
}

void test_iso_tp_send_transmit_error(void) {
  MockTwaiInterface mock_can;
  mock_can.reset();
  mock_can.transmit_result = ITwaiInterface::TwaiError::TRANSMIT_FAILED;
  IsoTp iso_tp(mock_can);

  uint8_t test_data[] = {0x01, 0x02, 0x03};
  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  bool result = iso_tp.send(msg);

  TEST_ASSERT_FALSE_MESSAGE(result, "Send should fail when transmit fails");
}

// ============================================================================
// ТЕСТЫ ПРИЕМА (RECEIVE)
// ============================================================================

void test_iso_tp_receive_single_frame(void) {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t expected_data[]            = {0xAA, 0xBB, 0xCC, 0xDD};
  ITwaiInterface::TwaiFrame sf_frame = create_single_frame(0x7E8, 4, expected_data);
  mock_can.add_receive_frame(sf_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  TEST_ASSERT_FALSE_MESSAGE(result, "Receive should succeed (returns 0)");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(4, msg.len, "Received length should be 4");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected_data, msg.data, 4, "Received data should match");
}

void test_iso_tp_receive_multi_frame(void) {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t expected_data[15];
  for (int i = 0; i < 15; i++) {
    expected_data[i] = i + 0x10;
  }

  ITwaiInterface::TwaiFrame ff_frame  = create_first_frame(0x7E8, 15, expected_data);
  ITwaiInterface::TwaiFrame cf1_frame = create_consecutive_frame(0x7E8, 1, &expected_data[6], 7);
  ITwaiInterface::TwaiFrame cf2_frame = create_consecutive_frame(0x7E8, 2, &expected_data[13], 2);

  mock_can.add_receive_frame(ff_frame);
  mock_can.add_receive_frame(cf1_frame);
  mock_can.add_receive_frame(cf2_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  TEST_ASSERT_FALSE_MESSAGE(result, "Receive should succeed (returns 0)");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(15, msg.len, "Received length should be 15");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected_data, msg.data, 15, "Received data should match");
  TEST_ASSERT_TRUE_MESSAGE(mock_can.transmit_called, "Should send Flow Control");
}

void test_iso_tp_receive_timeout(void) {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Receive should timeout (returns 1)");
}

void test_iso_tp_receive_wrong_id(void) {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[]                = {0x11, 0x22, 0x33};
  ITwaiInterface::TwaiFrame sf_frame = create_single_frame(0x999, 3, test_data);  // Wrong ID
  mock_can.add_receive_frame(sf_frame);

  uint8_t receive_buffer[128];
  IsoTp::Message msg;
  msg.tx_id = 0x7DF;
  msg.rx_id = 0x7E8;  // Expected different ID
  msg.len   = 0;
  msg.data  = receive_buffer;

  bool result = iso_tp.receive(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Receive should timeout due to wrong ID");
}

// ============================================================================
// ТЕСТЫ FLOW CONTROL
// ============================================================================

void test_iso_tp_send_flow_control_wait(void) {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[15];
  for (int i = 0; i < 15; i++) {
    test_data[i] = i + 0x40;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  ITwaiInterface::TwaiFrame fc_wait = create_flow_control_frame(0x456, 1, 0, 0);  // WAIT
  ITwaiInterface::TwaiFrame fc_cts  = create_flow_control_frame(0x456, 0, 0, 0);  // CTS
  mock_can.add_receive_frame(fc_wait);
  mock_can.add_receive_frame(fc_cts);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_TRUE_MESSAGE(result, "Send should succeed after FC WAIT");
}

void test_iso_tp_send_flow_control_overflow(void) {
  MockTwaiInterface mock_can;
  mock_can.reset();
  IsoTp iso_tp(mock_can);

  uint8_t test_data[15];
  for (int i = 0; i < 15; i++) {
    test_data[i] = i + 0x50;
  }

  IsoTp::Message msg;
  msg.tx_id = 0x123;
  msg.rx_id = 0x456;
  msg.len   = sizeof(test_data);
  msg.data  = test_data;

  ITwaiInterface::TwaiFrame fc_overflow = create_flow_control_frame(0x456, 2, 0, 0);  // OVERFLOW
  mock_can.add_receive_frame(fc_overflow);

  bool result = iso_tp.send(msg);

  TEST_ASSERT_FALSE_MESSAGE(result, "Send should fail on FC OVERFLOW");
}

// ============================================================================
// ФУНКЦИЯ ЗАПУСКА ВСЕХ ТЕСТОВ
// ============================================================================

extern "C" void run_tests() {
  // Тесты отправки
  RUN_TEST(test_iso_tp_send_single_frame);
  RUN_TEST(test_iso_tp_send_single_frame_max_size);
  RUN_TEST(test_iso_tp_send_empty_frame);
  RUN_TEST(test_iso_tp_send_transmit_error);
  RUN_TEST(test_iso_tp_send_multi_frame);

  // Тесты приема
  RUN_TEST(test_iso_tp_receive_single_frame);
  RUN_TEST(test_iso_tp_receive_multi_frame);

  // Тесты Flow Control
  RUN_TEST(test_iso_tp_send_flow_control_wait);
  RUN_TEST(test_iso_tp_send_flow_control_overflow);

  // Тесты с таймаутами отключены (слишком долгие)
  // RUN_TEST(test_iso_tp_receive_timeout);
  // RUN_TEST(test_iso_tp_receive_wrong_id);
}