#include <cctype>
#include <cinttypes>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iso_tp.h"
#include "obd2.h"

static const char* TAG = "OBD2";

void OBD2::log_print(const char* format, ...) {
  if (OBD_DEBUG) {
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_INFO, TAG, format, args);
    va_end(args);
  }
}

void OBD2::log_print_buffer(uint32_t id, uint8_t* buffer, uint16_t len) {
  if (OBD_DEBUG) {
    char log_buffer[256];
    int offset = 0;

    offset += snprintf(
        log_buffer + offset, sizeof(log_buffer) - offset, "Buffer: %" PRIX32 " [%d] ", id, len);

    for (uint16_t i = 0; i < len && offset < sizeof(log_buffer) - 4; i++) {
      offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "%02X ", buffer[i]);
    }

    log_print("%s", log_buffer);
  }
}

OBD2::OBD2(IIsoTp& driver, uint16_t tx_id, uint16_t rx_id) :
    tx_id_(tx_id),
    rx_id_(rx_id),
    iso_tp_(driver) {}

/* Create a PID query command string and send the command
 * uint8_t service       - The diagnostic service ID. 01 is "Show current data"
 * uint8_t pid          - The Parameter ID (PID) from the service
 */
void OBD2::queryPID(uint8_t service, uint8_t pid) {
  log_print("Service: %d PID: %d", service, pid);
  uint8_t data[8]{2, service, pid, 0x00, 0x00, 0x00, 0x00, 0x00};
  IsoTp::Message msg{tx_id_, rx_id_, 3, data};
  log_print("Sending the following command/query");
  log_print_buffer(msg.tx_id, msg.data, msg.len);
  iso_tp_.send(msg);
}

/* Queries OBD2 for a specific type of vehicle telemetry data

 Inputs:
 -------
  * uint8_t service          - The diagnostic service ID. 01 is "Show current
 data"
  * uint16_t pid             - The Parameter ID (PID) from the service
  * uint8_t numExpectedBytes - Number of valid bytes from the response to
 process
  * double scaleFactor        - Amount to scale the response by
  * double bias               - Amount to bias the response by

 Return:
 -------
  * double - The PID value if successfully received, else 0.0
*/
bool OBD2::processPID(uint8_t service, uint16_t pid, ResponseType& response) {
  queryPID(service, pid);

  uint8_t payload[128];
  IsoTp::Message msg{tx_id_, rx_id_, 0, payload};
  if (iso_tp_.receive(msg, sizeof(payload))) {
    if (msg.len >= 3 && msg.data[0] == 0x7F) {
      log_print("OBD2 negative response received: service=0x%02X, error=0x%02X",
                msg.data[1],
                msg.data[2]);
    }

    const uint8_t response_service = service + 0x40;
    if ((payload[1] == response_service) && (payload[2] == pid)) {
      std::copy(response.begin(), response.end(), payload + 3);
      return true;
    }
  }
  return false;

  // else if (payload[1] == ERROR) {
  // }
}

// std::function<double(void)> calculator = selectCalculator(pid);
// if (!calculator) {
//   return conditionResponse(numExpectedBytes, scaleFactor, bias);
// } else {
//   return calculator();
// }