#pragma once
#include "twai_interface.h"

class IsoTp {
 public:
  // Static constants
  static const bool ISO_TP_DEBUG = true;

  static const uint8_t CAN_MAX_DLEN = 8;  // Not extended CAN

  // Flow Status given in FC frame
  static const uint8_t ISOTP_FC_CTS   = 0;  // clear to send
  static const uint8_t ISOTP_FC_WT    = 1;  // wait
  static const uint8_t ISOTP_FC_OVFLW = 2;  // overflow

  typedef enum {
    ISOTP_IDLE = 0,
    ISOTP_SEND,
    ISOTP_SEND_FF,
    ISOTP_SEND_CF,
    ISOTP_WAIT_FIRST_FC,
    ISOTP_WAIT_FC,
    ISOTP_WAIT_DATA,
    ISOTP_FINISHED,
    ISOTP_ERROR
  } isotp_states_t;

  // External structures for send and receive
  struct Message {
    uint32_t tx_id = 0;
    uint32_t rx_id = 0;
    uint16_t len   = 0;
    uint8_t *data  = nullptr;
  };

  IsoTp(ITwaiInterface &bus);
  bool send(Message &msg);
  bool receive(Message &msg);

 private:
  struct Message_t {
    uint32_t tx_id          = 0;
    uint32_t rx_id          = 0;
    isotp_states_t tp_state = ISOTP_IDLE;
    uint8_t fc_status       = ISOTP_FC_CTS;
    uint16_t seq_id         = 1;
    uint8_t blocksize       = 0;
    uint8_t min_sep_time    = 0;
    uint16_t len            = 0;
    uint8_t *Buffer;
  };

 private:
  // Timeout values
  static const uint32_t TIMEOUT_SESSION = 500;  // Timeout between successfull send and receive
  static const uint32_t TIMEOUT_FC      = 250;  // Timeout between FF and FC or Block CF and FC
  static const uint32_t TIMEOUT_CF      = 250;  // Timeout between CFs
  static const uint8_t MAX_FCWAIT_FRAME = 10;

  static const uint16_t MAX_MSGBUF = 128;  // Received Message Buffer. Depends on uC ressources!
                                           // Should be enough for our needs

  // N_PCI type values in bits 7-4 of N_PCI bytes
  static const uint8_t N_PCI_SF = 0x00;  // single frame
  static const uint8_t N_PCI_FF = 0x10;  // first frame
  static const uint8_t N_PCI_CF = 0x20;  // consecutive frame
  static const uint8_t N_PCI_FC = 0x30;  // flow control

  static const uint8_t FC_CONTENT_SZ = 3;  // flow control content size in byte (FS/BS/STmin)

  ITwaiInterface &_bus;
  ITwaiInterface::TwaiFrame rxFrame;
  uint16_t rest;
  uint8_t fc_wait_frames = 0;
  uint32_t wait_fc       = 0;
  uint32_t wait_cf       = 0;
  uint32_t wait_session  = 0;
  bool can_send(uint32_t id, uint8_t len, uint8_t *data);
  bool can_receive();
  bool send_fc(Message_t &msg);
  bool send_sf(Message_t &msg);
  bool send_ff(Message_t &msg);
  bool send_cf(Message_t &msg);

  void rcv_sf(Message_t &msg);
  bool rcv_ff(Message_t &msg);
  uint8_t rcv_cf(Message_t &msg);
  bool rcv_fc(Message_t &msg);

  static void fc_delay(uint8_t sep_time);

  void log_print(const char *format, ...);
  void log_print_buffer(uint32_t id, uint8_t *buffer, uint16_t len);
};