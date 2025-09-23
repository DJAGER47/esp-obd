#include "iso-tp.h"

#include <algorithm>
#include <cinttypes>
#include <cstdint>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint32_t millis() {
  return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

static const char* const TAG   = "ISO_TP";
static const bool ISO_TP_DEBUG = false;

const char* IsoTp::isotp_state_to_string(isotp_states_t state) {
  switch (state) {
    case ISOTP_IDLE:
      return "ISOTP_IDLE";
    case ISOTP_SEND:
      return "ISOTP_SEND";
    case ISOTP_SEND_FF:
      return "ISOTP_SEND_FF";
    case ISOTP_SEND_CF:
      return "ISOTP_SEND_CF";
    case ISOTP_WAIT_FIRST_FC:
      return "ISOTP_WAIT_FIRST_FC";
    case ISOTP_WAIT_FC:
      return "ISOTP_WAIT_FC";
    case ISOTP_WAIT_DATA:
      return "ISOTP_WAIT_DATA";
    case ISOTP_FINISHED:
      return "ISOTP_FINISHED";
    default:
      return "UNKNOWN_STATE";
  }
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

IsoTp::IsoTp(ITwaiInterface& bus) :
    _bus(bus) {}

void IsoTp::can_send(uint32_t id, uint8_t len, uint8_t* data) {
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
  _bus.transmit(message, 0);
}

bool IsoTp::can_receive() {
  if (_bus.receive(rxFrame, 0) == ITwaiInterface::TwaiError::OK) {
    log_print_buffer(rxFrame.id, rxFrame.data, rxFrame.data_length);
    return true;
  }
  return false;
}

void IsoTp::send_fc(const Message_t& msg) {
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // FC message high nibble = 0x3 , low nibble = FC Status
  TxBuf[0] = (N_PCI_FC | msg.fc_status);
  TxBuf[1] = msg.blocksize;
  /* fix wrong separation time values according spec */
  const bool wrong_sep_time =
      (msg.min_sep_time > 0x7F) && ((msg.min_sep_time < 0xF1) || (msg.min_sep_time > 0xF9));
  TxBuf[2] = wrong_sep_time ? 0x7F : msg.min_sep_time;
  can_send(msg.tx_id, 8, TxBuf);
}

void IsoTp::send_sf(const Message_t& msg) {
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // SF message high nibble = 0x0 , low nibble = Length
  TxBuf[0] = (N_PCI_SF | msg.len);
  memcpy(TxBuf + 1, msg.buffer, msg.len);
  //  return can_send(msg.tx_id,msg.len+1,TxBuf);// Add PCI length
  can_send(msg.tx_id, 8, TxBuf);  // Always send full frame
}

void IsoTp::send_ff(const Message_t& msg) {
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  TxBuf[0] = (N_PCI_FF | ((msg.len & 0x0F00) >> 8));
  TxBuf[1] = (msg.len & 0x00FF);
  memcpy(TxBuf + 2, msg.buffer, 6);  // Skip 2 Bytes PCI
  can_send(msg.tx_id, 8, TxBuf);     // First Frame has full length
}

void IsoTp::send_cf(const Message_t& msg) {
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint16_t len     = (msg.len > 7) ? 7 : msg.len;

  TxBuf[0] = (N_PCI_CF | (msg.seq_id & 0x0F));
  memcpy(TxBuf + 1, msg.buffer, len);  // Skip 1 Byte PCI
                                       // return can_send(msg.tx_id,len+1,TxBuf);
                                       // // Last frame is probably shorter
                                       // than 8 -> Signals last CF Frame
  can_send(msg.tx_id, 8, TxBuf);       // Last frame is probably shorter
                                       // than 8, pad with 00
}

void IsoTp::rcv_sf(Message_t& msg) {
  /* get the SF_DL from the N_PCI byte */
  msg.len = rxFrame.data[0] & 0x0F;

  uint16_t copy_len = msg.len;
  if (msg.len > msg.max_len) {
    log_print(
        "Warning: Buffer too small for SF (need %d, have %d), truncating", msg.len, msg.max_len);
    copy_len = msg.max_len;
  }

  /* copy the received data bytes */
  memcpy(msg.buffer, rxFrame.data + 1, copy_len);  // Skip PCI, SF uses len bytes
  msg.tp_state = ISOTP_FINISHED;
}

void IsoTp::rcv_ff(Message_t& msg) {
  msg.seq_id = 1;

  /* get the FF_DL */
  msg.len = (rxFrame.data[0] & 0x0F) << 8;
  msg.len += rxFrame.data[1];
  rest = msg.len;

  /* copy the first received data bytes */
  uint16_t copy_len = (msg.max_len > 6) ? 6 : msg.max_len;
  memcpy(msg.buffer, rxFrame.data + 2, copy_len);  // Skip 2 bytes PCI
  rest -= 6;                                       // Restlength

  msg.tp_state = ISOTP_WAIT_DATA;

  log_print("First frame received with message length: %d", rest);
  log_print("Send flow controll.");
  log_print("ISO-TP state: %s", IsoTp::isotp_state_to_string(msg.tp_state));

  /* send our first FC frame with Target Address*/
  Message_t fc;
  fc.tx_id        = msg.tx_id;
  fc.fc_status    = ISOTP_FC_CTS;
  fc.blocksize    = 0;
  fc.min_sep_time = 0;
  send_fc(fc);
}

void IsoTp::rcv_cf(Message_t& msg) {
  // Handle Timeout
  // If no Frame within 250ms change State to ISOTP_IDLE
  const uint32_t delta = millis() - wait_cf;

  if ((delta >= TIMEOUT_FC) && msg.seq_id > 1) {
    log_print("CF frame timeout during receive wait_cf=%lu delta=%lu", wait_cf, delta);

    msg.tp_state = ISOTP_IDLE;
    return;
  }
  wait_cf = millis();

  log_print("ISO-TP state: %s", IsoTp::isotp_state_to_string(msg.tp_state));
  log_print("CF received with message rest length: %d", rest);

  if (msg.tp_state != ISOTP_WAIT_DATA)
    return;

  const uint8_t received_seq_id = rxFrame.data[0] & 0x0F;
  const uint8_t expected_seq_id = msg.seq_id & 0x0F;

  if (received_seq_id != expected_seq_id) {
    if (received_seq_id < expected_seq_id) {
      // Дублированный кадр - игнорируем
      log_print("Duplicate CF ignored: Got sequence ID: %d Expected: %d",
                received_seq_id,
                expected_seq_id);
      return;
    } else {
      // Пропущен кадр - ошибка
      log_print("Missing CF detected: Got sequence ID: %d Expected: %d",
                received_seq_id,
                expected_seq_id);
      msg.tp_state = ISOTP_IDLE;
      msg.seq_id   = 1;
      return;
    }
  }

  const ssize_t offset         = 6 + 7 * (msg.seq_id - 1);
  const ssize_t tmp_space      = msg.max_len - offset;
  const size_t available_space = std::max(tmp_space, static_cast<ssize_t>(0));
  if (rest <= 7) {  // Last Frame
    uint16_t copy_len = (rest > available_space) ? available_space : rest;
    memcpy(msg.buffer + offset, rxFrame.data + 1, copy_len);  // 6 Bytes in FF + 7
    if (copy_len < rest) {
      log_print(
          "Warning: Truncated last CF frame (needed %d, had %d space)", rest, available_space);
    }
    msg.tp_state = ISOTP_FINISHED;  // per CF skip PCI
    log_print("Last CF received with seq. ID: %d", msg.seq_id);
  } else {
    uint16_t copy_len = (7 > available_space) ? available_space : 7;
    memcpy(msg.buffer + offset, rxFrame.data + 1, copy_len);
    if (copy_len < 7) {
      log_print("Warning: Truncated CF frame (needed 7, had %d space)", available_space);
    }
    rest -= 7;  // Got another 7 Bytes of Data;
    log_print("CF received with seq. ID: %d", msg.seq_id);
  }

  msg.seq_id++;
  return;
}

bool IsoTp::rcv_fc(Message_t& msg) {
  /* get communication parameters only from the first FC frame */
  if (msg.tp_state == ISOTP_WAIT_FIRST_FC) {  //??????????????????????????????????????????????????
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
      }
      log_print("Start waiting for next FC");
      break;

    case ISOTP_FC_OVFLW:
      log_print("Overflow in receiver side");
      // fall through
    default:
      return false;
  }
  return true;
}

bool IsoTp::send(Message& msg) {
  if ((msg.len > 4095) || ((msg.len > 0) && (msg.data == nullptr))) {
    return false;
  }

  Message_t internalMsg;
  internalMsg.tx_id  = msg.tx_id;
  internalMsg.rx_id  = msg.rx_id;
  internalMsg.len    = msg.len;
  internalMsg.buffer = msg.data;

  // Initialize other fields with default values
  internalMsg.tp_state     = ISOTP_SEND;
  internalMsg.fc_status    = ISOTP_FC_CTS;
  internalMsg.seq_id       = 1;
  internalMsg.blocksize    = 0;
  internalMsg.min_sep_time = 0;

  uint32_t wait_fc = 0;

  while (true) {
    log_print("\tISO-TP State: %s", isotp_state_to_string(internalMsg.tp_state));

    switch (internalMsg.tp_state) {
      case ISOTP_SEND:
        if (internalMsg.len <= 7) {
          log_print("Send SF");
          send_sf(internalMsg);
          return true;
        }

        log_print("Send FF");
        send_ff(internalMsg);
        internalMsg.seq_id = 1;
        internalMsg.buffer += 6;
        internalMsg.len -= 6;
        internalMsg.tp_state = ISOTP_WAIT_FIRST_FC;
        fc_wait_frames       = 0;
        wait_fc              = millis();
        break;

      case ISOTP_WAIT_FC:
      case ISOTP_WAIT_FIRST_FC: {
        if (can_receive() && (rxFrame.id == internalMsg.rx_id)) {
          if (!rcv_fc(internalMsg)) {
            return false;
          }
        }

        const uint32_t delta = millis() - wait_fc;
        if (delta >= TIMEOUT_FC) {
          log_print("FC timeout during receive wait_fc=%lu delta=%lu", wait_fc, delta);
          return false;
        }
      } break;

      case ISOTP_SEND_CF:
        if (internalMsg.len > 7) {
          fc_delay(internalMsg.min_sep_time);
          send_cf(internalMsg);
          log_print("Send Seq %d", internalMsg.seq_id);
          if (internalMsg.blocksize > 0) {
            log_print("Blocksize trigger %d", internalMsg.seq_id % internalMsg.blocksize);
            if (!(internalMsg.seq_id % internalMsg.blocksize)) {
              internalMsg.tp_state = ISOTP_WAIT_FC;
              wait_fc              = millis();
            }
          }
          internalMsg.seq_id++;
          if (internalMsg.blocksize < 16) {
            internalMsg.seq_id %= 16;
          } else {
            internalMsg.seq_id %= internalMsg.blocksize;
          }
          internalMsg.buffer += 7;
          internalMsg.len -= 7;
          log_print("Length      : %d", internalMsg.len);
        } else {
          fc_delay(internalMsg.min_sep_time);
          log_print("Send last Seq %d", internalMsg.seq_id);
          send_cf(internalMsg);
          return true;
        }
        break;

      case ISOTP_IDLE:  //??????????????????????????????????????????????????
      case ISOTP_SEND_FF:
      case ISOTP_WAIT_DATA:
      case ISOTP_FINISHED:
      default:
        break;
    }
  }
  return false;
}

bool IsoTp::receive(Message& msg, size_t size_buffer) {
  if (msg.data == nullptr || size_buffer == 0) {
    return false;
  }

  // Create internal Message_t structure
  Message_t internalMsg;
  internalMsg.tx_id   = msg.tx_id;
  internalMsg.rx_id   = msg.rx_id;
  internalMsg.len     = 0;  // Will be set during reception
  internalMsg.max_len = size_buffer;
  internalMsg.buffer  = msg.data;
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

  while (internalMsg.tp_state != ISOTP_FINISHED) {
    delta = millis() - wait_session;
    if (delta >= TIMEOUT_SESSION) {
      log_print("ISO-TP Session timeout wait_session=%lu delta=%lu", wait_session, delta);
      return false;
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
      }
    }
  }

  // Copy received data back to the external message structure
  msg.tx_id = internalMsg.tx_id;
  msg.rx_id = internalMsg.rx_id;
  msg.len   = internalMsg.len;
  // Note: msg.data points to the same buffer as internalMsg.buffer, so data is already there

  log_print("ISO-TP message received:");
  log_print_buffer(internalMsg.rx_id, internalMsg.buffer, internalMsg.len);

  return true;
}
