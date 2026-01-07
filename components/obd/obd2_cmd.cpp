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

static const char* const TAG = "OBD2";

/**
 * @brief Выводит отладочную информацию в лог (если включен режим отладки)
 *
 * @param format Форматная строка для вывода
 * @param ... Аргументы для форматной строки
 */
void OBD2::log_print(const char* format, ...) {
  if (OBD_DEBUG) {
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_INFO, TAG, format, args);
    va_end(args);
  }
}

/**
 * @brief Выводит содержимое буфера в лог (если включен режим отладки)
 *
 * @param id Идентификатор буфера
 * @param buffer Указатель на буфер данных
 * @param len Длина буфера в байтах
 */
void OBD2::log_print_buffer(uint32_t id, uint8_t* buffer, uint16_t len) {
  if (OBD_DEBUG) {
    char log_buffer[256];
    int offset = 0;

    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "Buffer: %" PRIX32 " [%d] ", id, len);

    for (uint16_t i = 0; i < len && offset < sizeof(log_buffer) - 4; i++) {
      offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "%02X ", buffer[i]);
    }

    log_print("%s", log_buffer);
  }
  log_print("\n");
}

/**
 * @brief Конструктор класса OBD2
 *
 * @param driver Ссылка на драйвер ISO-TP
 * @param tx_id CAN ID для передачи
 * @param rx_id CAN ID для приема
 */
OBD2::OBD2(IIsoTp& driver, uint16_t tx_id, uint16_t rx_id) :
    tx_id_(tx_id),
    rx_id_(rx_id),
    iso_tp_(driver) {}

/**
 * @brief Формирует и отправляет запрос PID
 *
 * @param service ID диагностического сервиса (01 - "Показать текущие данные")
 * @param pid Parameter ID (PID) из сервиса
 */
void OBD2::queryPID(uint8_t service, uint8_t pid) {
  log_print("Service: %d PID: %d\n", service, pid);
  uint8_t data[8]{service, pid, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  IsoTp::Message msg{tx_id_, rx_id_, 3, data};
  log_print("Sending the following command/query\n");
  log_print_buffer(msg.tx_id, msg.data, msg.len);
  iso_tp_.send(msg);
}

/**
 * @brief Запрашивает и обрабатывает данные телеметрии автомобиля
 *
 * @param service ID диагностического сервиса
 * @param pid Parameter ID (PID)
 * @param[out] response Буфер для записи ответа
 * @return bool True если данные успешно получены и обработаны, иначе False
 */
bool OBD2::processPID(uint8_t service, uint16_t pid, ResponseType& response) {
  int attempt_count      = 0;
  const int max_attempts = 3;

  while (attempt_count < max_attempts) {
    attempt_count++;

    queryPID(service, pid);

    uint8_t payload[128];
    IsoTp::Message msg{tx_id_, rx_id_, 0, payload};

    if (iso_tp_.receive(msg, sizeof(payload))) {
      // Проверяем на отрицательный ответ
      if ((msg.len >= 3) && (msg.data[0] == 0x7F) && (msg.data[1] == pid)) {
        const NegativeResponseCode error_code = static_cast<NegativeResponseCode>(msg.data[2]);
        auto str                              = getErrorDescription(error_code);

        ESP_LOGW(TAG,
                 "OBD2 negative response received: service=0x%02X, pid=0x%02X, Error=0x%02X, %s",
                 msg.data[0],
                 msg.data[1],
                 msg.data[2],
                 str);

        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;

        // auto strategy_info = error_handler_->handleNegativeResponse(service, pid, error_code);

        // switch (strategy_info.strategy) {
        //   case HandlingStrategy::RETRY_IMMEDIATELY:
        //     continue;

        //   case HandlingStrategy::RETRY_WITH_DELAY:
        //   case HandlingStrategy::RETRY_WITH_BACKOFF:
        //     vTaskDelay(pdMS_TO_TICKS(strategy_info.delay_ms));
        //     continue;

        //   case HandlingStrategy::SKIP_SERVICE:
        //   case HandlingStrategy::ABORT_OPERATION:
        //     return false;

        //   default:
        //     break;
        // }
      }

      // Проверяем положительный ответ
      const uint8_t response_service = service + 0x40;
      if ((msg.data[0] == response_service) && (msg.data[1] == pid)) {
        size_t data_len = msg.len - 2;
        if (data_len > response.size()) {
          ESP_LOGW(TAG, "processPID: trim data");
          data_len = response.size();
        }
        std::copy(msg.data + 2, msg.data + 2 + data_len, response.begin());
        return true;
      }
    } else {
      // Таймаут приема
      // NegativeResponseCode timeout_code = NegativeResponseCode::BUSY_REPEAT_REQUEST;

      // auto strategy_info = error_handler_->handleNegativeResponse(service, pid, timeout_code, context);

      // if (strategy_info.strategy == HandlingStrategy::RETRY_WITH_DELAY ||
      //     strategy_info.strategy == HandlingStrategy::RETRY_WITH_BACKOFF) {
      //   vTaskDelay(pdMS_TO_TICKS(strategy_info.delay_ms));
      //   continue;
      // }
    }
  }

  return false;
}

const char* OBD2::getErrorDescription(NegativeResponseCode error_code) const {
  switch (error_code) {
    case NegativeResponseCode::GENERAL_REJECT:
      return "General reject";
    case NegativeResponseCode::SERVICE_NOT_SUPPORTED:
      return "Service not supported";
    case NegativeResponseCode::SUB_FUNCTION_NOT_SUPPORTED:
      return "Sub-function not supported";
    case NegativeResponseCode::INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT:
      return "Invalid message length/format";
    case NegativeResponseCode::RESPONSE_TOO_LONG:
      return "Response too long";
    case NegativeResponseCode::BUSY_REPEAT_REQUEST:
      return "Busy-repeat request";
    case NegativeResponseCode::CONDITIONS_NOT_CORRECT:
      return "Conditions not correct";
    case NegativeResponseCode::REQUEST_SEQUENCE_ERROR:
      return "Request sequence error";
    case NegativeResponseCode::NO_RESPONSE_FROM_SUBNET_COMPONENT:
      return "No response from subnet component";
    case NegativeResponseCode::FAILURE_PREVENTS_EXECUTION_OF_REQUESTED_ACTION:
      return "Failure prevents execution of requested action";
    case NegativeResponseCode::REQUEST_OUT_OF_RANGE:
      return "Request out of range";
    case NegativeResponseCode::SECURITY_ACCESS_DENIED:
      return "Security access denied";
    case NegativeResponseCode::INVALID_KEY:
      return "Invalid key";
    case NegativeResponseCode::EXCEEDED_NUMBER_OF_ATTEMPTS:
      return "Exceeded number of attempts";
    case NegativeResponseCode::REQUIRED_TIME_DELAY_NOT_EXPIRED:
      return "Required time delay has not expired";
    case NegativeResponseCode::UPLOAD_DOWNLOAD_NOT_ACCEPTED:
      return "Upload/download not accepted";
    case NegativeResponseCode::TRANSFER_DATA_SUSPENDED:
      return "Transfer data suspended";
    case NegativeResponseCode::GENERAL_PROGRAMMING_FAILURE:
      return "Programming failure";
    case NegativeResponseCode::WRONG_BLOCK_SEQUENCE_NUMBER:
      return "Wrong block sequence counter";
    case NegativeResponseCode::REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING:
      return "Request received - response pending";
    case NegativeResponseCode::SUB_FUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION:
      return "Sub function not supported in active session";
    case NegativeResponseCode::SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION:
      return "Service not supported in active session";
    case NegativeResponseCode::RPM_TOO_HIGH:
      return "RPM too high";
    case NegativeResponseCode::RPM_TOO_LOW:
      return "RPM too low";
    case NegativeResponseCode::ENGINE_IS_RUNNING:
      return "Engine is running";
    case NegativeResponseCode::ENGINE_IS_NOT_RUNNING:
      return "Engine is not running";
    case NegativeResponseCode::ENGINE_RUN_TIME_TOO_LOW:
      return "Engine run time too low";
    case NegativeResponseCode::TEMPERATURE_TOO_HIGH:
      return "Temperature too high";
    case NegativeResponseCode::TEMPERATURE_TOO_LOW:
      return "Temperature too low";
    case NegativeResponseCode::VEHICLE_SPEED_TOO_HIGH:
      return "Speed too high";
    case NegativeResponseCode::VEHICLE_SPEED_TOO_LOW:
      return "Speed too low";
    case NegativeResponseCode::THROTTLE_PEDAL_TOO_HIGH:
      return "Throttle pedal too high";
    case NegativeResponseCode::THROTTLE_PEDAL_TOO_LOW:
      return "Throttle pedal too low";
    case NegativeResponseCode::TRANSMISSION_RANGE_NOT_IN_NEUTRAL:
      return "Transmission range not in neutral";
    case NegativeResponseCode::TRANSMISSION_RANGE_NOT_IN_GEAR:
      return "Transmission range not in gear";
    case NegativeResponseCode::BRAKE_SWITCHES_NOT_CLOSED:
      return "Brake switches not closed";
    case NegativeResponseCode::SHIFTER_LEVER_NOT_IN_PARK:
      return "Shifter lever not in park";
    case NegativeResponseCode::TORQUE_CONVERTER_CLUTCH_LOCKED:
      return "Torque converter clutch locked";
    case NegativeResponseCode::VOLTAGE_TOO_HIGH:
      return "Voltage too high";
    case NegativeResponseCode::VOLTAGE_TOO_LOW:
      return "Voltage too low";
    case NegativeResponseCode::MANUFACTURER_SPECIFIC_CONDITIONS_NOT_CORRECT:
      return "Manufacturer specific conditions not correct (0xF0-0xFE)";
    default:
      return "Unknown error code";
  }
}

bool OBD2::isTemporaryError(NegativeResponseCode error_code) const {
  switch (error_code) {
    case NegativeResponseCode::BUSY_REPEAT_REQUEST:
    case NegativeResponseCode::CONDITIONS_NOT_CORRECT:
    case NegativeResponseCode::REQUEST_SEQUENCE_ERROR:
    case NegativeResponseCode::FAILURE_PREVENTS_EXECUTION_OF_REQUESTED_ACTION:
    case NegativeResponseCode::REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING:
    case NegativeResponseCode::RPM_TOO_HIGH:
    case NegativeResponseCode::RPM_TOO_LOW:
    case NegativeResponseCode::ENGINE_IS_RUNNING:
    case NegativeResponseCode::ENGINE_IS_NOT_RUNNING:
    case NegativeResponseCode::ENGINE_RUN_TIME_TOO_LOW:
    case NegativeResponseCode::TEMPERATURE_TOO_HIGH:
    case NegativeResponseCode::TEMPERATURE_TOO_LOW:
    case NegativeResponseCode::VEHICLE_SPEED_TOO_HIGH:
    case NegativeResponseCode::VEHICLE_SPEED_TOO_LOW:
    case NegativeResponseCode::THROTTLE_PEDAL_TOO_HIGH:
    case NegativeResponseCode::THROTTLE_PEDAL_TOO_LOW:
    case NegativeResponseCode::TRANSMISSION_RANGE_NOT_IN_NEUTRAL:
    case NegativeResponseCode::TRANSMISSION_RANGE_NOT_IN_GEAR:
    case NegativeResponseCode::BRAKE_SWITCHES_NOT_CLOSED:
    case NegativeResponseCode::SHIFTER_LEVER_NOT_IN_PARK:
    case NegativeResponseCode::TORQUE_CONVERTER_CLUTCH_LOCKED:
    case NegativeResponseCode::VOLTAGE_TOO_HIGH:
    case NegativeResponseCode::VOLTAGE_TOO_LOW:
      return true;

    default:
      return false;
  }
}
