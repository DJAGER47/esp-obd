#include "iso-tp.h"

#include <cinttypes>
#include <cstdint>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint32_t millis() {
  return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

static const char* const TAG = "ISO_TP";

IsoTp::IsoTp(ITwaiInterface& bus) :
    _bus(bus) {}

void IsoTp::log_print(const char* format, ...) {
  if (ISO_TP_DEBUG) {
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_INFO, TAG, format, args);
    va_end(args);
  }
}

void IsoTp::log_print_buffer(uint32_t id, uint8_t* buffer, uint16_t len) {
  if (ISO_TP_DEBUG) {
    char log_buffer[128];
    int offset = 0;

    offset += snprintf(
        log_buffer + offset, sizeof(log_buffer) - offset, "Buffer: %" PRIX32 " [%d] ", id, len);

    for (uint16_t i = 0; i < len && offset < sizeof(log_buffer) - 4; i++) {
      offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "%02X ", buffer[i]);
    }

    ESP_LOGI(TAG, "%s", log_buffer);
  }
}

bool IsoTp::can_send(uint32_t id, uint8_t len, uint8_t* data) {
  log_print("Send CAN RAW Data:");
  log_print_buffer(id, data, len);

  ITwaiInterface::TwaiFrame message;
  message.id          = id;
  message.is_extended = false;  // Standard frame
  message.is_rtr      = false;
  message.is_fd       = false;
  message.brs         = false;
  message.data_length = len;
  if (len <= 8) {  // Проверка на максимальный размер данных для классического CAN
    memcpy(message.data, data, len);
  }
  return (_bus.transmit(message, 0) == ITwaiInterface::TwaiError::OK);
}

bool IsoTp::can_receive() {
  if (_bus.receive(rxFrame, 0) == ITwaiInterface::TwaiError::OK) {
    log_print("Received CAN RAW Data:");
    log_print_buffer(rxFrame.id, rxFrame.data, rxFrame.data_length);
    return true;
  }
  return false;
}

bool IsoTp::send_fc(Message_t& msg) {
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // FC message high nibble = 0x3 , low nibble = FC Status
  TxBuf[0] = (N_PCI_FC | msg.fc_status);
  TxBuf[1] = msg.blocksize;
  /* fix wrong separation time values according spec */
  if ((msg.min_sep_time > 0x7F) && ((msg.min_sep_time < 0xF1) || (msg.min_sep_time > 0xF9)))
    msg.min_sep_time = 0x7F;
  TxBuf[2] = msg.min_sep_time;
  return can_send(msg.tx_id, 8, TxBuf);
}

bool IsoTp::send_sf(Message_t& msg)  // Send SF Message
{
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // SF message high nibble = 0x0 , low nibble = Length
  TxBuf[0] = (N_PCI_SF | msg.len);
  memcpy(TxBuf + 1, msg.Buffer, msg.len);
  //  return can_send(msg.tx_id,msg.len+1,TxBuf);// Add PCI length
  return can_send(msg.tx_id, 8, TxBuf);  // Always send full frame
}

bool IsoTp::send_ff(Message_t& msg)  // Send FF
{
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  msg.seq_id       = 1;

  TxBuf[0] = (N_PCI_FF | ((msg.len & 0x0F00) >> 8));
  TxBuf[1] = (msg.len & 0x00FF);
  memcpy(TxBuf + 2, msg.Buffer, 6);      // Skip 2 Bytes PCI
  return can_send(msg.tx_id, 8, TxBuf);  // First Frame has full length
}

bool IsoTp::send_cf(Message_t& msg)  // Send CF Message
{
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint16_t len     = 7;

  TxBuf[0] = (N_PCI_CF | (msg.seq_id & 0x0F));
  if (msg.len > 7)
    len = 7;
  else
    len = msg.len;
  memcpy(TxBuf + 1, msg.Buffer, len);    // Skip 1 Byte PCI
                                         // return can_send(msg.tx_id,len+1,TxBuf);
                                         // // Last frame is probably shorter
                                         // than 8 -> Signals last CF Frame
  return can_send(msg.tx_id, 8, TxBuf);  // Last frame is probably shorter
                                         // than 8, pad with 00
}

void IsoTp::fc_delay(uint8_t sep_time) {
  /*
   * 0x00 - 0x7F: 0 - 127ms
   * 0x80 - 0xF0: reserved
   * 0xF1 - 0xF9: 100us - 900us
   * 0xFA - 0xFF: reserved
   * default 0x7F, 127ms
   */
  if (sep_time <= 0x7F)
    vTaskDelay(pdMS_TO_TICKS(sep_time));
  else if ((sep_time >= 0xF1) && (sep_time <= 0xF9))
    esp_rom_delay_us((sep_time - 0xF0) * 100);
  else
    vTaskDelay(pdMS_TO_TICKS(0x7F));
}

void IsoTp::rcv_sf(Message_t& msg) {
  /* get the SF_DL from the N_PCI byte */
  msg.len = rxFrame.data[0] & 0x0F;
  /* copy the received data bytes */
  memcpy(msg.Buffer, rxFrame.data + 1, msg.len);  // Skip PCI, SF uses len bytes
  msg.tp_state = ISOTP_FINISHED;
}

bool IsoTp::rcv_ff(Message_t& msg) {
  msg.seq_id = 1;

  /* get the FF_DL */
  msg.len = (rxFrame.data[0] & 0x0F) << 8;
  msg.len += rxFrame.data[1];
  rest = msg.len;

  /* copy the first received data bytes */
  memcpy(msg.Buffer, rxFrame.data + 2, 6);  // Skip 2 bytes PCI, FF must have 6 bytes!
  rest -= 6;                                // Restlength

  msg.tp_state = ISOTP_WAIT_DATA;

  log_print("First frame received with message length: %d", rest);
  log_print("Send flow controll.");
  log_print("ISO-TP state: %d", msg.tp_state);

  /* send our first FC frame with Target Address*/
  Message_t fc;
  fc.tx_id        = msg.tx_id;
  fc.fc_status    = ISOTP_FC_CTS;
  fc.blocksize    = 0;
  fc.min_sep_time = 0;
  return send_fc(fc);
}

uint8_t IsoTp::rcv_cf(Message_t& msg) {
  // Handle Timeout
  // If no Frame within 250ms change State to ISOTP_IDLE
  uint32_t delta = millis() - wait_cf;

  if ((delta >= TIMEOUT_FC) && msg.seq_id > 1) {
    log_print("CF frame timeout during receive wait_cf=%lu delta=%lu", wait_cf, delta);

    msg.tp_state = ISOTP_IDLE;
    return 1;
  }
  wait_cf = millis();

  log_print("ISO-TP state: %d", msg.tp_state);
  log_print("CF received with message rest length: %d", rest);

  if (msg.tp_state != ISOTP_WAIT_DATA)
    return 0;

  if ((rxFrame.data[0] & 0x0F) != (msg.seq_id & 0x0F)) {
    log_print("Got sequence ID: %d Expected: %d", rxFrame.data[0] & 0x0F, msg.seq_id & 0x0F);
    msg.tp_state = ISOTP_IDLE;
    msg.seq_id   = 1;
    return 1;
  }

  if (rest <= 7)  // Last Frame
  {
    memcpy(msg.Buffer + 6 + 7 * (msg.seq_id - 1), rxFrame.data + 1,
           rest);                   // 6 Bytes in FF +7
    msg.tp_state = ISOTP_FINISHED;  // per CF skip PCI
    log_print("Last CF received with seq. ID: %d", msg.seq_id);
  } else {
    log_print("CF received with seq. ID: %d", msg.seq_id);
    memcpy(msg.Buffer + 6 + 7 * (msg.seq_id - 1),
           rxFrame.data + 1,
           7);  // 6 Bytes in FF +7
                // per CF
    rest -= 7;  // Got another 7 Bytes of Data;
  }

  msg.seq_id++;

  return 0;
}

bool IsoTp::rcv_fc(Message_t& msg) {
  bool retval = false;

  if (msg.tp_state != ISOTP_WAIT_FC && msg.tp_state != ISOTP_WAIT_FIRST_FC)
    return retval;

  /* get communication parameters only from the first FC frame */
  if (msg.tp_state == ISOTP_WAIT_FIRST_FC) {
    msg.blocksize    = rxFrame.data[1];
    msg.min_sep_time = rxFrame.data[2];

    /* fix wrong separation time values according spec */
    if ((msg.min_sep_time > 0x7F) && ((msg.min_sep_time < 0xF1) || (msg.min_sep_time > 0xF9)))
      msg.min_sep_time = 0x7F;
  }

  log_print("FC frame: FS %d, Blocksize %d, Min. separation Time %d",
            rxFrame.data[0] & 0x0F,
            msg.blocksize,
            msg.min_sep_time);

  switch (rxFrame.data[0] & 0x0F) {
    case ISOTP_FC_CTS:
      msg.tp_state = ISOTP_SEND_CF;
      break;

    case ISOTP_FC_WT:
      fc_wait_frames++;
      if (fc_wait_frames >= MAX_FCWAIT_FRAME) {
        log_print("FC wait frames exceeded.");
        fc_wait_frames = 0;
        msg.tp_state   = ISOTP_IDLE;
        retval         = true;
      }
      log_print("Start waiting for next FC");
      break;

    case ISOTP_FC_OVFLW:
      log_print("Overflow in receiver side");
      // fall through
    default:
      msg.tp_state = ISOTP_IDLE;
      retval       = false;
  }
  return retval;
}

bool IsoTp::send(Message& msg) {
  // Create internal Message_t structure
  Message_t internalMsg;
  internalMsg.tx_id  = msg.tx_id;
  internalMsg.rx_id  = msg.rx_id;
  internalMsg.len    = msg.len;
  internalMsg.Buffer = msg.data;
  // Initialize other fields with default values
  internalMsg.tp_state     = ISOTP_IDLE;
  internalMsg.fc_status    = ISOTP_FC_CTS;
  internalMsg.seq_id       = 1;
  internalMsg.blocksize    = 0;
  internalMsg.min_sep_time = 0;

  bool bs        = false;
  uint32_t delta = 0;
  bool retval    = false;

  internalMsg.tp_state = ISOTP_SEND;

  while (internalMsg.tp_state != ISOTP_IDLE && internalMsg.tp_state != ISOTP_ERROR) {
    bs = false;
    log_print("ISO-TP State: %d", internalMsg.tp_state);
    log_print("Length      : %d", internalMsg.len);

    switch (internalMsg.tp_state) {
      case ISOTP_IDLE:
        break;

      case ISOTP_SEND:
        if (internalMsg.len <= 7) {
          log_print("Send SF");
          retval               = send_sf(internalMsg);
          internalMsg.tp_state = ISOTP_IDLE;
        } else {
          log_print("Send FF");
          retval = send_ff(internalMsg);
          if (retval) {  // FF complete
            internalMsg.Buffer += 6;
            internalMsg.len -= 6;
            internalMsg.tp_state = ISOTP_WAIT_FIRST_FC;
            fc_wait_frames       = 0;
            wait_fc              = millis();
          }
        }
        break;

      case ISOTP_WAIT_FIRST_FC:
        log_print("Wait first FC");
        delta = millis() - wait_fc;
        if (delta >= TIMEOUT_FC) {
          log_print("FC timeout during receive wait_fc=%lu delta=%lu", wait_fc, delta);
          internalMsg.tp_state = ISOTP_IDLE;
          retval               = true;
        }
        break;

      case ISOTP_WAIT_FC:
        log_print("Wait FC");
        break;

      case ISOTP_SEND_CF:
        log_print("Send CF");
        while (internalMsg.len > 7 && !bs) {
          fc_delay(internalMsg.min_sep_time);
          retval = send_cf(internalMsg);
          if (retval) {
            log_print("Send Seq %d", internalMsg.seq_id);
            if (internalMsg.blocksize > 0) {
              log_print("Blocksize trigger %d", internalMsg.seq_id % internalMsg.blocksize);
              if (!(internalMsg.seq_id % internalMsg.blocksize)) {
                bs                   = true;
                internalMsg.tp_state = ISOTP_WAIT_FC;
                log_print(" yes");
              } else {
                log_print(" no");
              }
            }
            internalMsg.seq_id++;
            if (internalMsg.blocksize < 16)
              internalMsg.seq_id %= 16;
            else
              internalMsg.seq_id %= internalMsg.blocksize;
            internalMsg.Buffer += 7;
            internalMsg.len -= 7;
            log_print("Length      : %d", internalMsg.len);
          }
        }
        if (!bs) {
          fc_delay(internalMsg.min_sep_time);
          log_print("Send last Seq %d", internalMsg.seq_id);
          retval               = send_cf(internalMsg);
          internalMsg.tp_state = ISOTP_IDLE;
        }
        break;

      default:
        break;
    }

    if (internalMsg.tp_state == ISOTP_WAIT_FIRST_FC || internalMsg.tp_state == ISOTP_WAIT_FC) {
      if (can_receive()) {
        log_print("Send branch:");
        if (rxFrame.id == internalMsg.rx_id) {
          retval = rcv_fc(internalMsg);
          memset(rxFrame.data, 0, sizeof(rxFrame.data));
          log_print("rxId OK!");
        }
      }
    }
  }
  // Note: We don't restore internalMsg.Buffer and internalMsg.len as they are local copies

  return retval;
}

bool IsoTp::receive(Message& msg) {
  // Create internal Message_t structure
  Message_t internalMsg;
  internalMsg.tx_id  = msg.tx_id;
  internalMsg.rx_id  = msg.rx_id;
  internalMsg.len    = 0;  // Will be set during reception
  internalMsg.Buffer = msg.data;
  // Initialize other fields with default values
  internalMsg.tp_state     = ISOTP_IDLE;
  internalMsg.fc_status    = ISOTP_FC_CTS;
  internalMsg.seq_id       = 1;
  internalMsg.blocksize    = 0;
  internalMsg.min_sep_time = 0;

  uint8_t n_pci_type = 0;
  uint32_t delta     = 0;

  wait_session = millis();
  log_print("Start receive...");
  internalMsg.tp_state = ISOTP_IDLE;

  while (internalMsg.tp_state != ISOTP_FINISHED && internalMsg.tp_state != ISOTP_ERROR) {
    delta = millis() - wait_session;
    if (delta >= TIMEOUT_SESSION) {
      log_print("ISO-TP Session timeout wait_session=%lu delta=%lu", wait_session, delta);
      return 1;
    }

    if (can_receive()) {
      if (rxFrame.id == internalMsg.rx_id) {
        log_print("rxId OK!");
        n_pci_type = rxFrame.data[0] & 0xF0;

        switch (n_pci_type) {
          case N_PCI_FC:
            log_print("FC");  // flow control frame
            rcv_fc(internalMsg);
            break;

          case N_PCI_SF:
            log_print("SF");      // single frame
            rcv_sf(internalMsg);  // internalMsg.tp_state=ISOTP_FINISHED;
            break;

          case N_PCI_FF:
            log_print("FF");      // first frame
            rcv_ff(internalMsg);  // internalMsg.tp_state=ISOTP_WAIT_DATA;
            break;

          case N_PCI_CF:
            log_print("CF");  // consecutive frame
            rcv_cf(internalMsg);
            break;
        }
        memset(rxFrame.data, 0, sizeof(rxFrame.data));
      }
    }
  }

  // Copy received data back to the external message structure
  msg.tx_id = internalMsg.tx_id;
  msg.rx_id = internalMsg.rx_id;
  msg.len   = internalMsg.len;
  // Note: msg.data points to the same buffer as internalMsg.Buffer, so data is already there

  log_print("ISO-TP message received:");
  log_print_buffer(internalMsg.rx_id, internalMsg.Buffer, internalMsg.len);

  return 0;
}
