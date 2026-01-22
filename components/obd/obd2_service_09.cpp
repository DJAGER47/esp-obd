#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esp_log.h"
#include "obd2.h"

static const char* const TAG = "OBD2_VIN";

/**
 * @brief Получает список поддерживаемых PID в Service 09 (диапазон 01-20)
 *
 * Возвращает битовую маску, где каждый бит указывает на поддержку соответствующего PID.
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_09_PID_00
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID
 */
std::optional<uint32_t> OBD2::supportedPIDs_Service09() {
  ResponseType response;
  if (ProcessPid(SERVICE_09, SERVICE_09_SUPPORTED_PIDS_01_20, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return std::nullopt;
}

/**
 * @brief Получает количество сообщений VIN в PID 02
 *
 * Только для ISO 9141-2, ISO 14230-4 и SAE J1850.
 * Обычно значение будет 5.
 *
 * @return std::optional<uint8_t> Количество сообщений VIN
 */
std::optional<uint8_t> OBD2::vinMessageCount() {
  ResponseType response;
  if (ProcessPid(SERVICE_09, SERVICE_09_VIN_MESSAGE_COUNT, response)) {
    return response[A];
  }
  return std::nullopt;
}

/**
 * @brief Получает идентификационный номер автомобиля (VIN)
 *
 * Эта функция отправляет запрос 0902 для получения VIN и обрабатывает ответ.
 * VIN - это 17-символьный код, который уникально идентифицирует автомобиль.
 *
 * @param vin_buffer Буфер для хранения VIN
 * @param buffer_size Размер буфера (должен быть не менее 18 байт для 17 символов + нулевой терминатор)
 * @return bool True если VIN успешно получен, иначе False
 */
bool OBD2::getVIN(char* vin_buffer, size_t buffer_size) {
  if (!vin_buffer || buffer_size < 18) {
    return false;
  }

  log_print("Getting VIN...\n");

  // Отправляем запрос для получения VIN (Service 09, PID 02)
  QueryPid(0x09, 0x02);

  uint8_t payload[128];
  IsoTp::Message msg{tx_id_, rx_id_, 0, payload};
  if (iso_tp_.receive(msg, sizeof(payload))) {
    if (msg.len >= 3 && msg.data[0] == 0x7F) {
      log_print("OBD2 negative response received: service=0x%02X, pid=0x%02X\n", msg.data[0], msg.data[1]);
      return false;
    }

    // Проверяем, что это правильный ответ на запрос VIN
    if (msg.len >= 4 && msg.data[0] == 0x49 && msg.data[1] == 0x02) {
      // Очищаем буфер
      memset(vin_buffer, 0, buffer_size);

      // Пропускаем заголовок и обрабатываем данные
      // Формат ответа: 49 02 01 [VIN данные в ASCII]
      size_t data_start = 4;  // Пропускаем 49 02 01
      size_t vin_index  = 0;

      for (size_t i = data_start; i < msg.len && vin_index < 17; i++) {
        if (msg.data[i] >= 0x20 && msg.data[i] <= 0x7E) {  // Проверяем, что это печатаемый ASCII символ
          vin_buffer[vin_index++] = static_cast<char>(msg.data[i]);
        }
      }

      if (vin_index == 17) {
        vin_buffer[17] = '\0';  // Гарантируем нулевое завершение строки
        log_print("VIN: %s\n", vin_buffer);
        return true;
      } else {
        log_print("Invalid VIN length: %d\n", vin_index);
      }
    }
  }

  log_print("No VIN response\n");
  vin_buffer[0] = '\0';  // Пустая строка в случае ошибки
  return false;
}

/**
 * @brief Получает количество сообщений Calibration ID в PID 04
 *
 * Только для ISO 9141-2, ISO 14230-4 и SAE J1850.
 * Будет кратно 4 (4 сообщения необходимы для каждого ID).
 *
 * @return std::optional<uint8_t> Количество сообщений Calibration ID
 */
std::optional<uint8_t> OBD2::calibrationIdMessageCount() {
  ResponseType response;
  if (ProcessPid(SERVICE_09, SERVICE_09_CALIB_ID_MESSAGE_COUNT, response)) {
    return response[A];
  }
  return std::nullopt;
}

/**
 * @brief Получает идентификатор калибровки (Calibration ID)
 *
 * До 16 ASCII символов. Несколько CALID могут быть выведены (по 16 байт каждый).
 * Неиспользуемые байты данных будут сообщены как нулевые байты (0x00).
 *
 * @param calib_buffer Буфер для хранения Calibration ID
 * @param buffer_size Размер буфера
 * @return bool True если Calibration ID успешно получен, иначе False
 */
bool OBD2::getCalibrationId(char* calib_buffer, size_t buffer_size) {
  if (!calib_buffer || buffer_size < 2) {
    return false;
  }

  QueryPid(SERVICE_09, SERVICE_09_CALIBRATION_ID);

  uint8_t payload[128];
  IsoTp::Message msg{tx_id_, rx_id_, 0, payload};
  if (iso_tp_.receive(msg, sizeof(payload))) {
    if (msg.len >= 3 && msg.data[0] == 0x7F) {
      log_print("OBD2 negative response received: service=0x%02X, pid=0x%02X\n", msg.data[0], msg.data[1]);
      return false;
    }

    // Проверяем, что это правильный ответ на запрос Calibration ID
    if (msg.len >= 4 && msg.data[0] == 0x49 && msg.data[1] == 0x04) {
      // Очищаем буфер
      memset(calib_buffer, 0, buffer_size);

      // Пропускаем заголовок и обрабатываем данные
      // Формат ответа: 49 04 [Calibration ID данные в ASCII]
      size_t data_start  = 3;  // Пропускаем 49 04
      size_t calib_index = 0;

      for (size_t i = data_start; i < msg.len && calib_index < buffer_size - 1; i++) {
        if (msg.data[i] >= 0x20 && msg.data[i] <= 0x7E) {  // Проверяем, что это печатаемый ASCII символ
          calib_buffer[calib_index++] = static_cast<char>(msg.data[i]);
        }
      }

      if (calib_index > 0) {
        calib_buffer[calib_index] = '\0';  // Гарантируем нулевое завершение строки
        log_print("Calibration ID: %s\n", calib_buffer);
        return true;
      }
    }
  }

  log_print("No Calibration ID response\n");
  calib_buffer[0] = '\0';  // Пустая строка в случае ошибки
  return false;
}

/**
 * @brief Получает количество сообщений CVN в PID 06
 *
 * Только для ISO 9141-2, ISO 14230-4 и SAE J1850.
 *
 * @return std::optional<uint8_t> Количество сообщений CVN
 */
std::optional<uint8_t> OBD2::cvnMessageCount() {
  ResponseType response;
  if (ProcessPid(SERVICE_09, SERVICE_09_CVN_MESSAGE_COUNT, response)) {
    return response[A];
  }
  return std::nullopt;
}

/**
 * @brief Получает номера верификации калибровки (Calibration Verification Numbers)
 *
 * Несколько CVN могут быть выведены (по 4 байта каждый).
 * Количество CVN и CALID должно совпадать.
 * Необработанные данные, дополненные нулевыми символами (0x00).
 * Обычно отображается как шестнадцатеричная строка.
 *
 * @param cvn_buffer Буфер для хранения CVN
 * @param buffer_size Размер буфера (максимальное количество CVN)
 * @param count Указатель для хранения количества полученных CVN
 * @return bool True если CVN успешно получены, иначе False
 */
bool OBD2::getCalibrationVerificationNumbers(uint32_t* cvn_buffer, size_t buffer_size, size_t* count) {
  if (!cvn_buffer || buffer_size == 0 || !count) {
    return false;
  }

  QueryPid(SERVICE_09, SERVICE_09_CALIBRATION_VERIFICATION_NUMBERS);

  uint8_t payload[128];
  IsoTp::Message msg{tx_id_, rx_id_, 0, payload};
  if (iso_tp_.receive(msg, sizeof(payload))) {
    if (msg.len >= 3 && msg.data[0] == 0x7F) {
      log_print("OBD2 negative response received: service=0x%02X, pid=0x%02X\n", msg.data[0], msg.data[1]);
      return false;
    }

    // Проверяем, что это правильный ответ на запрос CVN
    if (msg.len >= 4 && msg.data[0] == 0x49 && msg.data[1] == 0x06) {
      // Пропускаем заголовок и обрабатываем данные
      // Формат ответа: 49 06 [CVN данные в формате 4 байта каждый]
      size_t data_start = 3;  // Пропускаем 49 06

      *count = 0;
      // CVN состоит из 4 байт каждый
      for (size_t i = data_start; i + 3 < msg.len && *count < buffer_size; i += 4) {
        cvn_buffer[*count] = (msg.data[i] << 24) | (msg.data[i + 1] << 16) | (msg.data[i + 2] << 8) | msg.data[i + 3];
        (*count)++;
      }

      if (*count > 0) {
        log_print("CVNs count: %d\n", *count);
        return true;
      }
    }
  }

  log_print("No CVN response\n");
  *count = 0;
  return false;
}

/**
 * @brief Получает количество сообщений для отслеживания производительности в использовании
 *
 * Только для ISO 9141-2, ISO 14230-4 и SAE J1850.
 * 8 если требуется сообщить шестнадцать значений,
 * 9 если требуется сообщить восемнадцать значений,
 * 10 если требуется сообщить двадцать значений
 * (одно сообщение сообщает два значения, каждое из которых состоит из двух байтов).
 *
 * @return std::optional<uint8_t> Количество сообщений
 */
std::optional<uint8_t> OBD2::performanceTrackingMessageCount() {
  ResponseType response;
  if (ProcessPid(SERVICE_09, SERVICE_09_PERF_TRACK_MESSAGE_COUNT, response)) {
    return response[A];
  }
  return std::nullopt;
}

/**
 * @brief Получает данные отслеживания производительности для автомобилей с искровым зажиганием
 *
 * 4 или 5 сообщений, каждое содержащее 4 байта (два значения).
 *
 * @param tracking_buffer Буфер для хранения значений отслеживания
 * @param buffer_size Размер буфера (максимальное количество значений)
 * @param count Указатель для хранения количества полученных значений
 * @return bool True если данные успешно получены, иначе False
 */
bool OBD2::getPerformanceTrackingSparkIgnition(uint16_t* tracking_buffer, size_t buffer_size, size_t* count) {
  if (!tracking_buffer || buffer_size == 0 || !count) {
    return false;
  }

  QueryPid(SERVICE_09, SERVICE_09_PERF_TRACK_SPARK_IGNITION);

  uint8_t payload[128];
  IsoTp::Message msg{tx_id_, rx_id_, 0, payload};
  if (iso_tp_.receive(msg, sizeof(payload))) {
    if (msg.len >= 3 && msg.data[0] == 0x7F) {
      log_print("OBD2 negative response received: service=0x%02X, pid=0x%02X\n", msg.data[0], msg.data[1]);
      return false;
    }

    // Проверяем, что это правильный ответ на запрос отслеживания производительности
    if (msg.len >= 4 && msg.data[0] == 0x49 && msg.data[1] == 0x08) {
      // Пропускаем заголовок и обрабатываем данные
      // Формат ответа: 49 08 [данные отслеживания в формате 2 байта на значение]
      size_t data_start = 3;  // Пропускаем 49 08

      *count = 0;
      // Каждое значение состоит из 2 байт
      for (size_t i = data_start; i + 1 < msg.len && *count < buffer_size; i += 2) {
        tracking_buffer[*count] = (msg.data[i] << 8) | msg.data[i + 1];
        (*count)++;
      }

      if (*count > 0) {
        log_print("Spark ignition tracking values count: %d\n", *count);
        return true;
      }
    }
  }

  log_print("No spark ignition tracking response\n");
  *count = 0;
  return false;
}

/**
 * @brief Получает количество сообщений ECU name в PID 0A
 *
 * @return std::optional<uint8_t> Количество сообщений ECU name
 */
std::optional<uint8_t> OBD2::ecuNameMessageCount() {
  ResponseType response;
  if (ProcessPid(SERVICE_09, SERVICE_09_ECU_NAME_MESSAGE_COUNT, response)) {
    return response[A];
  }
  return std::nullopt;
}

/**
 * @brief Получает имя ECU
 *
 * ASCII-кодированное. Дополнено нулевыми символами (0x00) справа.
 *
 * @param ecu_buffer Буфер для хранения имени ECU
 * @param buffer_size Размер буфера
 * @return bool True если ECU name успешно получен, иначе False
 */
bool OBD2::getEcuName(char* ecu_buffer, size_t buffer_size) {
  if (!ecu_buffer || buffer_size < 2) {
    return false;
  }

  QueryPid(SERVICE_09, SERVICE_09_ECU_NAME);

  uint8_t payload[128];
  IsoTp::Message msg{tx_id_, rx_id_, 0, payload};
  if (iso_tp_.receive(msg, sizeof(payload))) {
    if (msg.len >= 3 && msg.data[0] == 0x7F) {
      log_print("OBD2 negative response received: service=0x%02X, pid=0x%02X\n", msg.data[0], msg.data[1]);
      return false;
    }

    // Проверяем, что это правильный ответ на запрос ECU name
    if (msg.len >= 4 && msg.data[0] == 0x49 && msg.data[1] == 0x0A) {
      // Очищаем буфер
      memset(ecu_buffer, 0, buffer_size);

      // Пропускаем заголовок и обрабатываем данные
      // Формат ответа: 49 0A [ECU name данные в ASCII]
      size_t data_start = 3;  // Пропускаем 49 0A
      size_t ecu_index  = 0;

      for (size_t i = data_start; i < msg.len && ecu_index < buffer_size - 1; i++) {
        if (msg.data[i] >= 0x20 && msg.data[i] <= 0x7E) {  // Проверяем, что это печатаемый ASCII символ
          ecu_buffer[ecu_index++] = static_cast<char>(msg.data[i]);
        }
      }

      if (ecu_index > 0) {
        ecu_buffer[ecu_index] = '\0';  // Гарантируем нулевое завершение строки
        log_print("ECU Name: %s\n", ecu_buffer);
        return true;
      }
    }
  }

  log_print("No ECU name response\n");
  ecu_buffer[0] = '\0';  // Пустая строка в случае ошибки
  return false;
}

/**
 * @brief Получает данные отслеживания производительности для автомобилей с воспламенением от сжатия
 *
 * 5 сообщений, каждое содержащее 4 байта (два значения).
 *
 * @param tracking_buffer Буфер для хранения значений отслеживания
 * @param buffer_size Размер буфера (максимальное количество значений)
 * @param count Указатель для хранения количества полученных значений
 * @return bool True если данные успешно получены, иначе False
 */
bool OBD2::getPerformanceTrackingCompressionIgnition(uint16_t* tracking_buffer, size_t buffer_size, size_t* count) {
  if (!tracking_buffer || buffer_size == 0 || !count) {
    return false;
  }

  QueryPid(SERVICE_09, SERVICE_09_PERF_TRACK_COMPRESSION_IGNITION);

  uint8_t payload[128];
  IsoTp::Message msg{tx_id_, rx_id_, 0, payload};
  if (iso_tp_.receive(msg, sizeof(payload))) {
    if (msg.len >= 3 && msg.data[0] == 0x7F) {
      log_print("OBD2 negative response received: service=0x%02X, pid=0x%02X\n", msg.data[0], msg.data[1]);
      return false;
    }

    // Проверяем, что это правильный ответ на запрос отслеживания производительности
    if (msg.len >= 4 && msg.data[0] == 0x49 && msg.data[1] == 0x0B) {
      // Пропускаем заголовок и обрабатываем данные
      // Формат ответа: 49 0B [данные отслеживания в формате 2 байта на значение]
      size_t data_start = 3;  // Пропускаем 49 0B

      *count = 0;
      // Каждое значение состоит из 2 байт
      for (size_t i = data_start; i + 1 < msg.len && *count < buffer_size; i += 2) {
        tracking_buffer[*count] = (msg.data[i] << 8) | msg.data[i + 1];
        (*count)++;
      }

      if (*count > 0) {
        log_print("Compression ignition tracking values count: %d\n", *count);
        return true;
      }
    }
  }

  log_print("No compression ignition tracking response\n");
  *count = 0;
  return false;
}
