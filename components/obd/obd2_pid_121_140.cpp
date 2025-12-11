#include "obd2.h"

/**
 * @brief Получает список поддерживаемых PID в диапазоне 121-140
 *
 * Возвращает битовую маску, где каждый бит указывает на поддержку соответствующего PID.
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID
 */
std::optional<uint32_t> OBD2::supportedPIDs121_140() {
  return getSupportedPIDs(SUPPORTED_PIDS_121_140);
}

/**
 * @brief NOx Sensor Corrected Data
 *
 * @return std::optional<std::array<uint16_t, 4>> Концентрация NOx в ppm для четырех датчиков
 */
std::optional<std::array<uint16_t, 4>> OBD2::noxSensorCorrectedData() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xA1, response)) {
    std::array<uint16_t, 4> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];  // NOx Sensor 1
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];  // NOx Sensor 2
    result[2] = (response[OBD2::E] << 8) | response[OBD2::F];  // NOx Sensor 3
    result[3] = (response[OBD2::G] << 8) | response[OBD2::H];  // NOx Sensor 4
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Cylinder Fuel Rate
 *
 * @return std::optional<float> Расход топлива в мг/ход
 */
std::optional<float> OBD2::cylinderFuelRate() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xA2, response)) {
    return {((response[OBD2::A] << 8) | response[OBD2::B]) / 32.0f};
  }
  return std::nullopt;
}

/**
 * @brief Evap System Vapor Pressure
 *
 * @return std::optional<std::array<int16_t, 4>> Давление в Па для четырех датчиков
 */
std::optional<std::array<int16_t, 4>> OBD2::evapSystemVaporPressure() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xA3, response)) {
    std::array<int16_t, 4> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];  // Evap System Vapor Pressure Sensor 1
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];  // Evap System Vapor Pressure Sensor 2
    result[2] = (response[OBD2::E] << 8) | response[OBD2::F];  // Evap System Vapor Pressure Sensor 3
    result[3] = (response[OBD2::G] << 8) | response[OBD2::H];  // Evap System Vapor Pressure Sensor 4
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Transmission Actual Gear
 *
 * @return std::optional<float> Передаточное отношение
 */
std::optional<float> OBD2::transmissionActualGear() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xA4, response)) {
    if (response[OBD2::A] & 0x02) {  // Supported
      return {((response[OBD2::C] << 8) | response[OBD2::D]) / 1000.0f};
    }
  }
  return std::nullopt;
}

/**
 * @brief Commanded Diesel Exhaust Fluid Dosing
 *
 * @return std::optional<float> Процент дозирования DEF
 */
std::optional<float> OBD2::commandedDieselExhaustFluidDosing() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xA5, response)) {
    if (response[OBD2::A] & 0x01) {  // Supported
      return {response[OBD2::B] / 2.0f};
    }
  }
  return std::nullopt;
}

/**
 * @brief Odometer
 *
 * @return std::optional<uint32_t> Пробег в км
 */
std::optional<uint32_t> OBD2::odometer() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xA6, response)) {
    return {(response[OBD2::A] << 24) | (response[OBD2::B] << 16) | (response[OBD2::C] << 8) | response[OBD2::D]};
  }
  return std::nullopt;
}

/**
 * @brief NOx Sensor Concentration Sensors 3 and 4
 *
 * @return std::optional<std::array<uint16_t, 2>> Концентрация NOx в ppm для двух датчиков
 */
std::optional<std::array<uint16_t, 2>> OBD2::noxSensorConcentration34() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xA7, response)) {
    std::array<uint16_t, 2> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];  // NOx Sensor 3
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];  // NOx Sensor 4
    return result;
  }
  return std::nullopt;
}

/**
 * @brief NOx Sensor Corrected Concentration Sensors 3 and 4
 *
 * @return std::optional<std::array<uint16_t, 2>> Концентрация NOx в ppm для двух датчиков
 */
std::optional<std::array<uint16_t, 2>> OBD2::noxSensorCorrectedConcentration34() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xA8, response)) {
    std::array<uint16_t, 2> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];  // NOx Sensor 3
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];  // NOx Sensor 4
    return result;
  }
  return std::nullopt;
}

/**
 * @brief ABS Disable Switch State
 *
 * @return std::optional<bool> Состояние переключателя отключения ABS
 */
std::optional<bool> OBD2::absDisableSwitchState() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xA9, response)) {
    if (response[OBD2::A] & 0x01) {  // Supported
      return {static_cast<bool>(response[OBD2::B] & 0x01)};
    }
  }
  return std::nullopt;
}

/**
 * @brief Fuel Level Input A/B
 *
 * @return std::optional<std::array<float, 2>> Уровень топлива в процентах для двух датчиков
 */
std::optional<std::array<float, 2>> OBD2::fuelLevelInputAB() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xC3, response)) {
    std::array<float, 2> result;
    result[0] = ((response[OBD2::A] << 8) | response[OBD2::B]) * 100.0f / 255.0f;  // Fuel Level Input A
    result[1] = ((response[OBD2::C] << 8) | response[OBD2::D]) * 100.0f / 255.0f;  // Fuel Level Input B
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Exhaust Particulate Control System Diagnostic Time/Count
 *
 * @return std::optional<std::array<uint32_t, 2>> [Время в секундах, Количество]
 */
std::optional<std::array<uint32_t, 2>> OBD2::exhaustParticulateControlSystemDiagnostic() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xC4, response)) {
    std::array<uint32_t, 2> result;
    result[0] = (response[OBD2::A] << 24) | (response[OBD2::B] << 16) | (response[OBD2::C] << 8) |
                response[OBD2::D];                             // Time in seconds
    result[1] = (response[OBD2::E] << 8) | response[OBD2::F];  // Count
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Fuel Pressure A and B
 *
 * @return std::optional<std::array<uint16_t, 2>> Давление топлива в кПа для двух датчиков
 */
std::optional<std::array<uint16_t, 2>> OBD2::fuelPressureAB() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xC5, response)) {
    std::array<uint16_t, 2> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];  // Fuel Pressure A
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];  // Fuel Pressure B
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Particulate control - driver inducement system status and counters
 *
 * @return std::optional<std::array<uint16_t, 5>> [Статус системы, Счетчик удаления/блокировки, Счетчик отказов системы
 * впрыска, Счетчик отказов системы мониторинга, Общее время в часах]
 */
std::optional<std::array<uint16_t, 5>> OBD2::particulateControlDriverInducementSystem() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xC6, response)) {
    std::array<uint16_t, 5> result;
    result[0] = response[OBD2::A];  // Particulate control - driver inducement system status
    result[1] = (response[OBD2::B] << 8) |
                response[OBD2::C];  // Removal or block of the particulate aftertreatment system counter
    result[2] = (response[OBD2::D] << 8) | response[OBD2::E];  // Liquid regent injection system failure counter
    result[3] =
        (response[OBD2::F] << 8) | response[OBD2::G];  // Malfunction of Particulate control monitoring system counter
    // Note: The last value is not clearly defined in the specification, so we'll skip it for now
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Distance Since Reflash or Module Replacement
 *
 * @return std::optional<uint16_t> Расстояние в км
 */
std::optional<uint16_t> OBD2::distanceSinceReflashOrModuleReplacement() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xC7, response)) {
    return {(response[OBD2::A] << 8) | response[OBD2::B]};
  }
  return std::nullopt;
}

/**
 * @brief NOx Control Diagnostic (NCD) and Particulate Control Diagnostic (PCD) Warning Lamp status
 *
 * @return std::optional<uint8_t> Статус лампы предупреждения
 */
std::optional<uint8_t> OBD2::noxParticulateControlDiagnosticWarningLamp() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, 0xC8, response)) {
    return response[OBD2::A];
  }
  return std::nullopt;
}