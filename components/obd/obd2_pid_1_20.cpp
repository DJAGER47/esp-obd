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
 * @brief Получает статус мониторинга с момента последнего сброса DTC
 *
 * Включает статус индикатора неисправности (MIL) и количество DTC.
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_01
 *
 * @return std::optional<uint32_t> Статус в битовом формате
 */
std::optional<uint32_t> OBD2::monitorStatus() {
  ResponseType response;
  if (processPID(SERVICE_01, MONITOR_STATUS_SINCE_DTC_CLEARED, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return std::nullopt;
}

/**
 * @brief Получает данные "замороженного" кадра DTC
 *
 * Содержит информацию о состоянии автомобиля в момент возникновения ошибки.
 * @see https://www.samarins.com/diagnose/freeze-frame.html
 *
 * @return std::optional<uint16_t> Различные параметры автомобиля
 */
std::optional<uint16_t> OBD2::freezeDTC() {
  ResponseType response;
  if (processPID(SERVICE_01, FREEZE_DTC, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return std::nullopt;
}

/**
 * @brief Получает статус топливной системы
 *
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_03
 *
 * @return std::optional<uint16_t> Статус в битовом формате
 */
std::optional<uint16_t> OBD2::fuelSystemStatus() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_SYSTEM_STATUS, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return std::nullopt;
}

/**
 * @brief Получает текущую нагрузку двигателя
 *
 * @return std::optional<float> Нагрузка двигателя в процентах [0-100%]
 */
std::optional<float> OBD2::engineLoad() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_LOAD, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает температуру охлаждающей жидкости двигателя
 *
 * @return std::optional<int16_t> Температура в градусах Цельсия
 */
std::optional<int16_t> OBD2::engineCoolantTemp() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_COOLANT_TEMP, response)) {
    return {static_cast<int16_t>(response[A]) - 40.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает краткосрочную коррекцию топливоподачи для банка 1
 *
 * @return std::optional<float> Коррекция топливоподачи в процентах [-100..99.2%]
 */
std::optional<float> OBD2::shortTermFuelTrimBank_1() {
  ResponseType response;
  if (processPID(SERVICE_01, SHORT_TERM_FUEL_TRIM_BANK_1, response)) {
    return {(response[A] * 100.0 / 128.0) - 100.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает долгосрочную коррекцию топливоподачи для банка 1
 *
 * @return std::optional<float> Коррекция топливоподачи в процентах [-100..99.2%]
 */
std::optional<float> OBD2::longTermFuelTrimBank_1() {
  ResponseType response;
  if (processPID(SERVICE_01, LONG_TERM_FUEL_TRIM_BANK_1, response)) {
    return {(response[A] * 100.0 / 128.0) - 100.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает краткосрочную коррекцию топливоподачи для банка 2
 *
 * @return std::optional<float> Коррекция топливоподачи в процентах [-100..99.2%]
 */
std::optional<float> OBD2::shortTermFuelTrimBank_2() {
  ResponseType response;
  if (processPID(SERVICE_01, SHORT_TERM_FUEL_TRIM_BANK_2, response)) {
    return {(response[A] * 100.0 / 128.0) - 100.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает долгосрочную коррекцию топливоподачи для банка 2
 *
 * @return std::optional<float> Коррекция топливоподачи в процентах [-100..99.2%]
 */
std::optional<float> OBD2::longTermFuelTrimBank_2() {
  ResponseType response;
  if (processPID(SERVICE_01, LONG_TERM_FUEL_TRIM_BANK_2, response)) {
    return {(response[A] * 100.0 / 128.0) - 100.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает давление топлива
 *
 * @return std::optional<uint16_t> Давление топлива в кПа
 */
std::optional<uint16_t> OBD2::fuelPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_PRESSURE, response)) {
    return {static_cast<uint16_t>(response[A]) * 3.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает абсолютное давление во впускном коллекторе
 *
 * @return std::optional<uint8_t> Давление в кПа
 */
std::optional<uint8_t> OBD2::manifoldPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, INTAKE_MANIFOLD_ABS_PRESSURE, response)) {
    return {response[A]};
  }
  return std::nullopt;
}

/**
 * @brief Получает обороты двигателя
 *
 * @return std::optional<float> Обороты двигателя в об/мин
 */
std::optional<float> OBD2::rpm() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_RPM, response)) {
    return {((response[A] << 8) | response[B]) / 4.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает скорость автомобиля
 *
 * @return std::optional<uint8_t> Скорость в км/ч
 */
std::optional<uint8_t> OBD2::kph() {
  ResponseType response;
  if (processPID(SERVICE_01, VEHICLE_SPEED, response)) {
    return response[A];
  }
  return std::nullopt;
}

/**
 * @brief Получает угол опережения зажигания
 *
 * @return std::optional<float> Угол опережения в градусах до ВМТ
 */
std::optional<float> OBD2::timingAdvance() {
  ResponseType response;
  if (processPID(SERVICE_01, TIMING_ADVANCE, response)) {
    return {response[A] / 2.0 - 64.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает температуру всасываемого воздуха
 *
 * @return std::optional<int16_t> Температура в градусах Цельсия
 */
std::optional<int16_t> OBD2::intakeAirTemp() {
  ResponseType response;
  if (processPID(SERVICE_01, INTAKE_AIR_TEMP, response)) {
    return {static_cast<int16_t>(response[A]) - 40.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает расход воздуха по датчику MAF
 *
 * @return std::optional<float> Расход воздуха в г/с
 */
std::optional<float> OBD2::mafRate() {
  ResponseType response;
  if (processPID(SERVICE_01, MAF_FLOW_RATE, response)) {
    return {((response[A] << 8) | response[B]) / 100.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает положение дроссельной заслонки
 *
 * @return std::optional<float> Положение в процентах [0-100%]
 */
std::optional<float> OBD2::throttle() {
  ResponseType response;
  if (processPID(SERVICE_01, THROTTLE_POSITION, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает статус системы вторичного воздуха
 *
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_12
 *
 * @return std::optional<uint8_t> Статус в битовом формате
 */
std::optional<uint8_t> OBD2::commandedSecAirStatus() {
  ResponseType response;
  if (processPID(SERVICE_01, COMMANDED_SECONDARY_AIR_STATUS, response)) {
    return response[A];
  }
  return std::nullopt;
}

/**
 * @brief Проверяет наличие кислородных датчиков (2 банка)
 *
 * Битовая маска: [A0..A3] - Банк 1, датчики 1-4; [A4..A7] - Банк 2...
 *
 * @return std::optional<uint8_t> Битовая маска присутствующих датчиков
 */
std::optional<uint8_t> OBD2::oxygenSensorsPresent_2banks() {
  ResponseType response;
  if (processPID(SERVICE_01, OXYGEN_SENSORS_PRESENT_2_BANKS, response)) {
    return response[A];
  }
  return std::nullopt;
}

/*
[this]() -> std::optional<float> {
        return response[A] / 200.0;
      };
constexpr std::optional<uint8_t> OXYGEN_SENSOR_1_A              = 20;  // 0x14 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_2_A              = 21;  // 0x15 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_3_A              = 22;  // 0x16 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_4_A              = 23;  // 0x17 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_5_A              = 24;  // 0x18 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_6_A              = 25;  // 0x19 - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_7_A              = 26;  // 0x1A - V %
constexpr std::optional<uint8_t> OXYGEN_SENSOR_8_A              = 27;  // 0x1B - V %
*/

/**
 * @brief Получает стандарт OBD, которому соответствует автомобиль
 *
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_1C
 *
 * @return std::optional<uint8_t> Код стандарта OBD
 */
std::optional<uint8_t> OBD2::obdStandards() {
  ResponseType response;
  if (processPID(SERVICE_01, OBD_STANDARDS, response)) {
    return response[A];
  }
  return std::nullopt;
}

/**
 * @brief Проверяет наличие кислородных датчиков (4 банка)
 *
 * Битовая маска: [A0..A7] == [B1S1, B1S2, B2S1, B2S2, B3S1, B3S2, B4S1, B4S2]
 *
 * @return std::optional<uint8_t> Битовая маска присутствующих датчиков
 */
std::optional<uint8_t> OBD2::oxygenSensorsPresent_4banks() {
  ResponseType response;
  if (processPID(SERVICE_01, OXYGEN_SENSORS_PRESENT_4_BANKS, response)) {
    return response[A];
  }
  return std::nullopt;
}

/**
 * @brief Проверяет статус Power Take Off (PTO)
 *
 * @return std::optional<bool> Статус PTO
 */
std::optional<bool> OBD2::auxInputStatus() {
  ResponseType response;
  if (processPID(SERVICE_01, AUX_INPUT_STATUS, response)) {
    return static_cast<bool>(response[A]);
  }
  return std::nullopt;
}

/**
 * @brief Получает время работы двигателя с момента запуска
 *
 * @return std::optional<uint16_t> Время в секундах
 */
std::optional<uint16_t> OBD2::runTime() {
  ResponseType response;
  if (processPID(SERVICE_01, RUN_TIME_SINCE_ENGINE_START, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return std::nullopt;
}
