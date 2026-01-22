#include <array>
#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

/**
 * @brief Получает список поддерживаемых PID в диапазоне 21-40
 *
 * Возвращает битовую маску, где каждый бит указывает на поддержку соответствующего PID.
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_20
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID
 */
std::optional<uint32_t> OBD2::supportedPIDs_21_40() {
  return GetSupportedPids(SUPPORTED_PIDS_21_40);
}

/**
 * @brief Получает расстояние, пройденное с горящей лампой неисправности (MIL)
 *
 * @return std::optional<uint16_t> Расстояние в километрах
 */
std::optional<uint16_t> OBD2::distTravelWithMIL() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, DISTANCE_TRAVELED_WITH_MIL_ON, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return std::nullopt;
}

/**
 * @brief Получает давление в топливной рампе (относительно вакуума в коллекторе)
 *
 * @return std::optional<float> Давление в кПа
 */
std::optional<float> OBD2::fuelRailPressure() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, FUEL_RAIL_PRESSURE, response)) {
    return {((response[A] << 8) | response[B]) * 0.079};
  }
  return std::nullopt;
}

/**
 * @brief Получает давление в топливной рампе (дизель или непосредственный впрыск)
 *
 * @return std::optional<uint32_t> Давление в кПа
 */
std::optional<uint32_t> OBD2::fuelRailGuagePressure() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, FUEL_RAIL_GUAGE_PRESSURE, response)) {
    return {((response[A] << 8) | response[B]) * 10};
  }
  return std::nullopt;
}

/*
[this]() -> std::optional<float> {
        return response[A] / 200.0;
      };
constexpr std::optional<uint8_t> OXYGEN_SENSOR_1_B             = 36;  // 0x24 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_2_B             = 37;  // 0x25 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_3_B             = 38;  // 0x26 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_4_B             = 39;  // 0x27 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_5_B             = 40;  // 0x28 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_6_B             = 41;  // 0x29 - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_7_B             = 42;  // 0x2A - ratio V
constexpr std::optional<uint8_t> OXYGEN_SENSOR_8_B             = 43;  // 0x2B - ratio V
*/

/**
 * @brief Получает заданный процент рециркуляции выхлопных газов (EGR)
 *
 * @return std::optional<float> Процент EGR [0-100%]
 */
std::optional<float> OBD2::commandedEGR() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, COMMANDED_EGR, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает ошибку системы рециркуляции выхлопных газов (EGR)
 *
 * @return std::optional<float> Ошибка в процентах [-100..99.2%]
 */
std::optional<float> OBD2::egrError() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, EGR_ERROR, response)) {
    return {(response[A] * 100.0 / 128.0) - 100.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает заданный процент продувки испарительной системы
 *
 * @return std::optional<float> Процент продувки [0-100%]
 */
std::optional<float> OBD2::commandedEvapPurge() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, COMMANDED_EVAPORATIVE_PURGE, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает уровень топлива в баке
 *
 * @return std::optional<float> Уровень в процентах [0-100%]
 */
std::optional<float> OBD2::fuelLevel() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, FUEL_TANK_LEVEL_INPUT, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает количество прогреваний двигателя после сброса ошибок
 *
 * @return std::optional<uint8_t> Количество прогреваний
 */
std::optional<uint8_t> OBD2::warmUpsSinceCodesCleared() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, WARM_UPS_SINCE_CODES_CLEARED, response)) {
    return {response[A]};
  }
  return std::nullopt;
}

/**
 * @brief Получает расстояние, пройденное после сброса ошибок
 *
 * @return std::optional<uint16_t> Расстояние в километрах
 */
std::optional<uint16_t> OBD2::distSinceCodesCleared() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, DIST_TRAV_SINCE_CODES_CLEARED, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return std::nullopt;
}

/**
 * @brief Получает давление паров в системе улавливания паров топлива
 *
 * @return std::optional<float> Давление в Па
 */
std::optional<float> OBD2::evapSysVapPressure() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, EVAP_SYSTEM_VAPOR_PRESSURE, response)) {
    return {((response[A] << 8) | response[B]) / 4.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает абсолютное атмосферное давление
 *
 * @return std::optional<uint8_t> Давление в кПа
 */
std::optional<uint8_t> OBD2::absBaroPressure() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, ABS_BAROMETRIC_PRESSURE, response)) {
    return {response[A]};
  }
  return std::nullopt;
}

/*
[this]() -> std::optional<float> {
        return response[A] / 200.0;
      };
constexpr std::optional<uint8_t> OXYGEN_SENSOR_1_C             = 52;  // 0x34 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_2_C             = 53;  // 0x35 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_3_C             = 54;  // 0x36 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_4_C             = 55;  // 0x37 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_5_C             = 56;  // 0x38 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_6_C             = 57;  // 0x39 - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_7_C             = 58;  // 0x3A - ratio mA
constexpr std::optional<uint8_t> OXYGEN_SENSOR_8_C             = 59;  // 0x3B - ratio mA
*/

/**
 * @brief Получает температуру катализатора (банк 1, датчик 1)
 *
 * @return std::optional<float> Температура в градусах Цельсия
 */
std::optional<float> OBD2::catTempB1S1() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, CATALYST_TEMP_BANK_1_SENSOR_1, response)) {
    return {(((response[A] << 8) | response[B]) / 10.0) - 40.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает температуру катализатора (банк 2, датчик 1)
 *
 * @return std::optional<float> Температура в градусах Цельсия
 */
std::optional<float> OBD2::catTempB2S1() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, CATALYST_TEMP_BANK_2_SENSOR_1, response)) {
    return {(((response[A] << 8) | response[B]) / 10.0) - 40.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает температуру катализатора (банк 1, датчик 2)
 *
 * @return std::optional<float> Температура в градусах Цельсия
 */
std::optional<float> OBD2::catTempB1S2() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, CATALYST_TEMP_BANK_1_SENSOR_2, response)) {
    return {(((response[A] << 8) | response[B]) / 10.0) - 40.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает температуру катализатора (банк 2, датчик 2)
 *
 * @return std::optional<float> Температура в градусах Цельсия
 */
std::optional<float> OBD2::catTempB2S2() {
  ResponseType response;
  if (ProcessPid(SERVICE_01, CATALYST_TEMP_BANK_2_SENSOR_2, response)) {
    return {(((response[A] << 8) | response[B]) / 10.0) - 40.0};
  }
  return std::nullopt;
}