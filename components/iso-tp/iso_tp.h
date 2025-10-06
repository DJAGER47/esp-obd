#pragma once

#include <cstddef>
#include <cstdint>

#include "iso_tp_interface.h"
#include "phy_interface.h"

class IsoTp : public IIsoTp {
  // Single Frame       = SF
  // First Frame        = FF
  // Consecutive Frame  = CF
  // Flow control Frame = FC
 public:
  IsoTp(IPhyInterface &bus);
  bool send(Message &msg) override;
  bool receive(Message &msg, size_t size_buffer) override;

 private:
  typedef enum {
    ISOTP_IDLE = 0,
    ISOTP_SEND,
    ISOTP_SEND_FF,
    ISOTP_SEND_CF,
    ISOTP_WAIT_FIRST_FC,
    ISOTP_WAIT_FC,
    ISOTP_WAIT_DATA,
    ISOTP_FINISHED
  } isotp_states_t;

  struct Message_t {
    uint32_t tx_id          = 0;
    uint32_t rx_id          = 0;
    uint8_t *buffer         = nullptr;
    size_t len              = 0;
    size_t max_len          = 0;
    uint16_t seq_id         = 1;
    uint8_t fc_status       = ISOTP_FC_CTS;
    uint8_t blocksize       = 0;
    uint8_t min_sep_time    = 0;
    isotp_states_t tp_state = ISOTP_IDLE;
  };

  static const uint8_t CAN_MAX_DLEN = 8;  // Not extended CAN

  // Flow Status given in FC frame
  static const uint8_t ISOTP_FC_CTS   = 0;  // clear to send
  static const uint8_t ISOTP_FC_WT    = 1;  // wait
  static const uint8_t ISOTP_FC_OVFLW = 2;  // overflow

  // Timeout values
  static const uint32_t TIMEOUT_SESSION = 500;  // Timeout between successfull send and receive
  static const uint32_t TIMEOUT_FC      = 250;  // Timeout between FF and FC or Block CF and FC
  static const uint32_t TIMEOUT_CF      = 250;  // Timeout between CFs
  static const uint8_t MAX_FCWAIT_FRAME = 10;

  // N_PCI type values in bits 7-4 of N_PCI bytes
  static const uint8_t N_PCI_SF = 0x00;  // single frame
  static const uint8_t N_PCI_FF = 0x10;  // first frame
  static const uint8_t N_PCI_CF = 0x20;  // consecutive frame
  static const uint8_t N_PCI_FC = 0x30;  // flow control

  static const uint8_t FC_CONTENT_SZ = 3;  // flow control content size in byte (FS/BS/STmin)

  static const char *isotp_state_to_string(isotp_states_t state);
  static void fc_delay(uint8_t sep_time);
  static void log_print(const char *format, ...);
  static void log_print_buffer(uint32_t id, uint8_t *buffer, uint16_t len);

  void can_send(uint32_t id, uint8_t len, uint8_t *data);
  bool can_receive();

  void send_fc(const Message_t &msg);
  void send_sf(const Message_t &msg);
  void send_ff(const Message_t &msg);
  void send_cf(const Message_t &msg);

  void rcv_sf(Message_t &msg);
  void rcv_ff(Message_t &msg);
  void rcv_cf(Message_t &msg);
  bool rcv_fc(Message_t &msg);

  IPhyInterface &_bus;
  TwaiFrame rxFrame;

  uint8_t fc_wait_frames = 0;
  uint32_t wait_cf       = 0;
  uint32_t wait_session  = 0;
  uint16_t rest;
};
