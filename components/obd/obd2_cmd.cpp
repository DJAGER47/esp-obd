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
  queryPID(service, pid);

  uint8_t payload[128];
  IsoTp::Message msg{tx_id_, rx_id_, 0, payload};
  if (iso_tp_.receive(msg, sizeof(payload))) {
    if (msg.len >= 3 && msg.data[0] == 0x7F) {
      ESP_LOGW(TAG, "OBD2 negative response received: service=0x%02X, pid=0x%02X", msg.data[0], msg.data[1]);
    }

    const uint8_t response_service = service + 0x40;
    if ((msg.data[0] == response_service) && (msg.data[1] == pid)) {
      // Проверяем, что в ответе достаточно данных для копирования
      size_t data_len = msg.len - 2;  // Вычитаем 2 байта заголовка (service + pid)
      if (data_len > response.size()) {
        ESP_LOGW(TAG, "processPID: trim data");
        data_len = response.size();  // Ограничиваем размером response
      }
      std::copy(msg.data + 2, msg.data + 2 + data_len, response.begin());
      return true;
    }
  }
  return false;

  // else if (payload[1] == ERROR) {
  // }
}
