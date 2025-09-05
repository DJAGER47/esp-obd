#include "iso-tp.h"

#include <cinttypes>
#include <cstdint>

#include "esp_log.h"
#include "freertos/task.h"
#include "twai_errors.h"

uint32_t millis() {
  return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

static const char* const TAG = "ISO_TP";

IsoTp::IsoTp(ITwaiInterface& bus) :
    _bus(bus) {}

void IsoTp::log_print(const char* format, ...) {
#ifdef ISO_TP_DEBUG
  va_list args;
  va_start(args, format);
  esp_log_writev(ESP_LOG_INFO, TAG, format, args);
  va_end(args);
#endif
}

void IsoTp::log_print_buffer(uint32_t id, uint8_t* buffer, uint16_t len) {
#ifdef ISO_TP_DEBUG
  char log_buffer[128];
  int offset = 0;

  offset += snprintf(log_buffer + offset,
                     sizeof(log_buffer) - offset,
                     "Buffer: %" PRIX32 " [%d] ",
                     id,
                     len);

  for (uint16_t i = 0; i < len && offset < sizeof(log_buffer) - 4; i++) {
    offset += snprintf(
        log_buffer + offset, sizeof(log_buffer) - offset, "%02X ", buffer[i]);
  }

  ESP_LOGI(TAG, "%s", log_buffer);
#endif
}

uint8_t IsoTp::can_send(uint32_t id, uint8_t len, uint8_t* data) {
  log_print("Send CAN RAW Data:");
  log_print_buffer(id, data, len);

  twai_message_t message;
  message.identifier       = id;
  message.data_length_code = len;
  memcpy(message.data, data, len);
  message.flags = 0;  // Standard frame
  return (_bus.transmit(&message, 0) == TwaiError::OK) ? 0 : 1;
}

uint8_t IsoTp::can_receive() {
  twai_message_t message;
  if (_bus.receive(&message, 0) == TwaiError::OK) {
    rxId  = message.identifier;
    rxLen = message.data_length_code;
    memcpy(rxBuffer, message.data, rxLen);

    log_print("Received CAN RAW Data:");
    log_print_buffer(rxId, rxBuffer, rxLen);
    return true;
  }
  return false;
}

uint8_t IsoTp::send_fc(struct Message_t* msg) {
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // FC message high nibble = 0x3 , low nibble = FC Status
  TxBuf[0] = (N_PCI_FC | msg->fc_status);
  TxBuf[1] = msg->blocksize;
  /* fix wrong separation time values according spec */
  if ((msg->min_sep_time > 0x7F) &&
      ((msg->min_sep_time < 0xF1) || (msg->min_sep_time > 0xF9)))
    msg->min_sep_time = 0x7F;
  TxBuf[2] = msg->min_sep_time;
  return can_send(msg->tx_id, 8, TxBuf);
}

uint8_t IsoTp::send_sf(struct Message_t* msg)  // Send SF Message
{
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // SF message high nibble = 0x0 , low nibble = Length
  TxBuf[0] = (N_PCI_SF | msg->len);
  memcpy(TxBuf + 1, msg->Buffer, msg->len);
  //  return can_send(msg->tx_id,msg->len+1,TxBuf);// Add PCI length
  return can_send(msg->tx_id, 8, TxBuf);  // Always send full frame
}

uint8_t IsoTp::send_ff(struct Message_t* msg)  // Send FF
{
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  msg->seq_id      = 1;

  TxBuf[0] = (N_PCI_FF | ((msg->len & 0x0F00) >> 8));
  TxBuf[1] = (msg->len & 0x00FF);
  memcpy(TxBuf + 2, msg->Buffer, 6);      // Skip 2 Bytes PCI
  return can_send(msg->tx_id, 8, TxBuf);  // First Frame has full length
}

uint8_t IsoTp::send_cf(struct Message_t* msg)  // Send CF Message
{
  uint8_t TxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint16_t len     = 7;

  TxBuf[0] = (N_PCI_CF | (msg->seq_id & 0x0F));
  if (msg->len > 7)
    len = 7;
  else
    len = msg->len;
  memcpy(
      TxBuf + 1, msg->Buffer, len);  // Skip 1 Byte PCI
                                     // return can_send(msg->tx_id,len+1,TxBuf);
                                     // // Last frame is probably shorter
                                     // than 8 -> Signals last CF Frame
  return can_send(msg->tx_id, 8, TxBuf);  // Last frame is probably shorter
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

uint8_t IsoTp::rcv_sf(struct Message_t* msg) {
  /* get the SF_DL from the N_PCI byte */
  msg->len = rxBuffer[0] & 0x0F;
  /* copy the received data bytes */
  memcpy(msg->Buffer, rxBuffer + 1, msg->len);  // Skip PCI, SF uses len bytes
  msg->tp_state = ISOTP_FINISHED;

  return 0;
}

uint8_t IsoTp::rcv_ff(struct Message_t* msg) {
  msg->seq_id = 1;

  /* get the FF_DL */
  msg->len = (rxBuffer[0] & 0x0F) << 8;
  msg->len += rxBuffer[1];
  rest = msg->len;

  /* copy the first received data bytes */
  memcpy(
      msg->Buffer, rxBuffer + 2, 6);  // Skip 2 bytes PCI, FF must have 6 bytes!
  rest -= 6;                          // Restlength

  msg->tp_state = ISOTP_WAIT_DATA;

  log_print("First frame received with message length: %d", rest);
  log_print("Send flow controll.");
  log_print("ISO-TP state: %d", msg->tp_state);

  /* send our first FC frame with Target Address*/
  struct Message_t fc;
  fc.tx_id        = msg->tx_id;
  fc.fc_status    = ISOTP_FC_CTS;
  fc.blocksize    = 0;
  fc.min_sep_time = 0;
  return send_fc(&fc);
}

uint8_t IsoTp::rcv_cf(struct Message_t* msg) {
  // Handle Timeout
  // If no Frame within 250ms change State to ISOTP_IDLE
  uint32_t delta = millis() - wait_cf;

  if ((delta >= TIMEOUT_FC) && msg->seq_id > 1) {
    log_print("CF frame timeout during receive wait_cf=%lu delta=%lu",
              wait_cf,
              delta);

    msg->tp_state = ISOTP_IDLE;
    return 1;
  }
  wait_cf = millis();

  log_print("ISO-TP state: %d", msg->tp_state);
  log_print("CF received with message rest length: %d", rest);

  if (msg->tp_state != ISOTP_WAIT_DATA)
    return 0;

  if ((rxBuffer[0] & 0x0F) != (msg->seq_id & 0x0F)) {
    log_print("Got sequence ID: %d Expected: %d",
              rxBuffer[0] & 0x0F,
              msg->seq_id & 0x0F);
    msg->tp_state = ISOTP_IDLE;
    msg->seq_id   = 1;
    return 1;
  }

  if (rest <= 7)  // Last Frame
  {
    memcpy(msg->Buffer + 6 + 7 * (msg->seq_id - 1),
           rxBuffer + 1,
           rest);                    // 6 Bytes in FF +7
    msg->tp_state = ISOTP_FINISHED;  // per CF skip PCI
    log_print("Last CF received with seq. ID: %d", msg->seq_id);
  } else {
    log_print("CF received with seq. ID: %d", msg->seq_id);
    memcpy(msg->Buffer + 6 + 7 * (msg->seq_id - 1),
           rxBuffer + 1,
           7);  // 6 Bytes in FF +7
                // per CF
    rest -= 7;  // Got another 7 Bytes of Data;
  }

  msg->seq_id++;

  return 0;
}

uint8_t IsoTp::rcv_fc(struct Message_t* msg) {
  uint8_t retval = 0;

  if (msg->tp_state != ISOTP_WAIT_FC && msg->tp_state != ISOTP_WAIT_FIRST_FC)
    return 0;

  /* get communication parameters only from the first FC frame */
  if (msg->tp_state == ISOTP_WAIT_FIRST_FC) {
    msg->blocksize    = rxBuffer[1];
    msg->min_sep_time = rxBuffer[2];

    /* fix wrong separation time values according spec */
    if ((msg->min_sep_time > 0x7F) &&
        ((msg->min_sep_time < 0xF1) || (msg->min_sep_time > 0xF9)))
      msg->min_sep_time = 0x7F;
  }

  log_print("FC frame: FS %d, Blocksize %d, Min. separation Time %d",
            rxBuffer[0] & 0x0F,
            msg->blocksize,
            msg->min_sep_time);

  switch (rxBuffer[0] & 0x0F) {
    case ISOTP_FC_CTS:
      msg->tp_state = ISOTP_SEND_CF;
      break;

    case ISOTP_FC_WT:
      fc_wait_frames++;
      if (fc_wait_frames >= MAX_FCWAIT_FRAME) {
        log_print("FC wait frames exceeded.");
        fc_wait_frames = 0;
        msg->tp_state  = ISOTP_IDLE;
        retval         = 1;
      }
      log_print("Start waiting for next FC");
      break;

    case ISOTP_FC_OVFLW:
      log_print("Overflow in receiver side");
      // fall through
    default:
      msg->tp_state = ISOTP_IDLE;
      retval        = 1;
  }
  return retval;
}

uint8_t IsoTp::send(Message_t* msg) {
  uint8_t* origBuffer = msg->Buffer;
  uint16_t origLen    = msg->len;

  uint8_t bs     = false;
  uint32_t delta = 0;
  uint8_t retval = 0;

  msg->tp_state = ISOTP_SEND;

  while (msg->tp_state != ISOTP_IDLE && msg->tp_state != ISOTP_ERROR) {
    bs = false;
    log_print("ISO-TP State: %d", msg->tp_state);
    log_print("Length      : %d", msg->len);

    switch (msg->tp_state) {
      case ISOTP_IDLE:
        break;
      case ISOTP_SEND:
        if (msg->len <= 7) {
          log_print("Send SF");
          retval        = send_sf(msg);
          msg->tp_state = ISOTP_IDLE;
        } else {
          log_print("Send FF");
          if (!(retval = send_ff(msg)))  // FF complete
          {
            msg->Buffer += 6;
            msg->len -= 6;
            msg->tp_state  = ISOTP_WAIT_FIRST_FC;
            fc_wait_frames = 0;
            wait_fc        = millis();
          }
        }
        break;
      case ISOTP_WAIT_FIRST_FC:
        log_print("Wait first FC");
        delta = millis() - wait_fc;
        if (delta >= TIMEOUT_FC) {
          log_print("FC timeout during receive wait_fc=%lu delta=%lu",
                    wait_fc,
                    delta);
          msg->tp_state = ISOTP_IDLE;
          retval        = 1;
        }
        break;
      case ISOTP_WAIT_FC:
        log_print("Wait FC");
        break;
      case ISOTP_SEND_CF:
        log_print("Send CF");
        while (msg->len > 7 && !bs) {
          fc_delay(msg->min_sep_time);
          if (!(retval = send_cf(msg))) {
            log_print("Send Seq %d", msg->seq_id);
            if (msg->blocksize > 0) {
              log_print("Blocksize trigger %d", msg->seq_id % msg->blocksize);
              if (!(msg->seq_id % msg->blocksize)) {
                bs            = true;
                msg->tp_state = ISOTP_WAIT_FC;
                log_print(" yes");
              } else {
                log_print(" no");
              }
            }
            msg->seq_id++;
            if (msg->blocksize < 16)
              msg->seq_id %= 16;
            else
              msg->seq_id %= msg->blocksize;
            msg->Buffer += 7;
            msg->len -= 7;
            log_print("Length      : %d", msg->len);
          }
        }
        if (!bs) {
          fc_delay(msg->min_sep_time);
          log_print("Send last Seq %d", msg->seq_id);
          retval        = send_cf(msg);
          msg->tp_state = ISOTP_IDLE;
        }
        break;
      default:
        break;
    }

    if (msg->tp_state == ISOTP_WAIT_FIRST_FC ||
        msg->tp_state == ISOTP_WAIT_FC) {
      if (can_receive()) {
        log_print("Send branch:");
        if (rxId == msg->rx_id) {
          retval = rcv_fc(msg);
          memset(rxBuffer, 0, sizeof(rxBuffer));
          log_print("rxId OK!");
        }
      }
    }
  }
  msg->Buffer = origBuffer;
  msg->len    = origLen;

  return retval;
}

uint8_t IsoTp::receive(Message_t* msg) {
  uint8_t n_pci_type = 0;
  uint32_t delta     = 0;

  wait_session = millis();
  log_print("Start receive...");
  msg->tp_state = ISOTP_IDLE;

  while (msg->tp_state != ISOTP_FINISHED && msg->tp_state != ISOTP_ERROR) {
    delta = millis() - wait_session;
    if (delta >= TIMEOUT_SESSION) {
      log_print("ISO-TP Session timeout wait_session=%lu delta=%lu",
                wait_session,
                delta);
      return 1;
    }

    if (can_receive()) {
      if (rxId == msg->rx_id) {
        log_print("rxId OK!");
        n_pci_type = rxBuffer[0] & 0xF0;

        switch (n_pci_type) {
          case N_PCI_FC:
            log_print("FC");
            /* tx path: fc frame */
            rcv_fc(msg);
            break;

          case N_PCI_SF:
            log_print("SF");
            /* rx path: single frame */
            rcv_sf(msg);
            //		      msg->tp_state=ISOTP_FINISHED;
            break;

          case N_PCI_FF:
            log_print("FF");
            /* rx path: first frame */
            rcv_ff(msg);
            //		      msg->tp_state=ISOTP_WAIT_DATA;
            break;
            break;

          case N_PCI_CF:
            log_print("CF");
            /* rx path: consecutive frame */
            rcv_cf(msg);
            break;
        }
        memset(rxBuffer, 0, sizeof(rxBuffer));
      }
    }
  }
  log_print("ISO-TP message received:");
  log_print_buffer(msg->rx_id, msg->Buffer, msg->len);

  return 0;
}
