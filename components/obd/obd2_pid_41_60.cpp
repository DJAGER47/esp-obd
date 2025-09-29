#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "obd2.h"

/**
 * @brief Получает статус текущего цикла движения
 *
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_41
 * @return std::optional<uint32_t> Статус в битовом формате
 */
std::optional<uint32_t> OBD2::monitorDriveCycleStatus() {
  ResponseType response;
  if (processPID(SERVICE_01, MONITOR_STATUS_THIS_DRIVE_CYCLE, response)) {
    return {(response[A] << 24) | (response[B] << 16) | (response[C] << 8) | response[D]};
  }
  return std::nullopt;
}

/**
 * @brief Получает напряжение питания блока управления
 *
 * @return std::optional<float> Напряжение в вольтах
 */
std::optional<float> OBD2::ctrlModVoltage() {
  ResponseType response;
  if (processPID(SERVICE_01, CONTROL_MODULE_VOLTAGE, response)) {
    return {((response[A] << 8) | response[B]) / 1000.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает абсолютное значение нагрузки двигателя
 *
 * @return std::optional<float> Нагрузка в процентах [0-100%]
 */
std::optional<float> OBD2::absLoad() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_LOAD_VALUE, response)) {
    return {((response[A] << 8) | response[B]) * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает заданное соотношение воздух/топливо
 *
 * @return std::optional<float> Коэффициент эквивалентности
 */
std::optional<float> OBD2::commandedAirFuelRatio() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_AIR_COMMANDED_EQUIV_RATIO, response)) {
    return {((response[A] << 8) | response[B]) / 32768.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает относительное положение дроссельной заслонки
 *
 * @return std::optional<float> Положение в процентах [0-100%]
 */
std::optional<float> OBD2::relativeThrottle() {
  ResponseType response;
  if (processPID(SERVICE_01, RELATIVE_THROTTLE_POSITION, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает температуру окружающего воздуха
 *
 * @return std::optional<int16_t> Температура в градусах Цельсия
 */
std::optional<int16_t> OBD2::ambientAirTemp() {
  ResponseType response;
  if (processPID(SERVICE_01, AMBIENT_AIR_TEMP, response)) {
    return {static_cast<int16_t>(response[A]) - 40};
  }
  return std::nullopt;
}

/**
 * @brief Получает абсолютное положение дроссельной заслонки B
 *
 * @return std::optional<float> Положение в процентах [0-100%]
 */
std::optional<float> OBD2::absThrottlePosB() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_THROTTLE_POSITION_B, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает абсолютное положение дроссельной заслонки C
 *
 * @return std::optional<float> Положение в процентах [0-100%]
 */
std::optional<float> OBD2::absThrottlePosC() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_THROTTLE_POSITION_C, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает абсолютное положение дроссельной заслонки D
 *
 * @return std::optional<float> Положение в процентах [0-100%]
 */
std::optional<float> OBD2::absThrottlePosD() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_THROTTLE_POSITION_D, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает абсолютное положение дроссельной заслонки E
 *
 * @return std::optional<float> Положение в процентах [0-100%]
 */
std::optional<float> OBD2::absThrottlePosE() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_THROTTLE_POSITION_E, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает абсолютное положение дроссельной заслонки F
 *
 * @return std::optional<float> Положение в процентах [0-100%]
 */
std::optional<float> OBD2::absThrottlePosF() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_THROTTLE_POSITION_F, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает заданное положение исполнительного механизма дросселя
 *
 * @return std::optional<float> Положение в процентах [0-100%]
 */
std::optional<float> OBD2::commandedThrottleActuator() {
  ResponseType response;
  if (processPID(SERVICE_01, COMMANDED_THROTTLE_ACTUATOR, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает время работы с горящей лампой неисправности (MIL)
 *
 * @return std::optional<uint16_t> Время в минутах
 */
std::optional<uint16_t> OBD2::timeRunWithMIL() {
  ResponseType response;
  if (processPID(SERVICE_01, TIME_RUN_WITH_MIL_ON, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return std::nullopt;
}

/**
 * @brief Получает время с момента сброса ошибок
 *
 * @return std::optional<uint16_t> Время в минутах
 */
std::optional<uint16_t> OBD2::timeSinceCodesCleared() {
  ResponseType response;
  if (processPID(SERVICE_01, TIME_SINCE_CODES_CLEARED, response)) {
    return {(response[A] << 8) | response[B]};
  }
  return std::nullopt;
}

/*
constexpr std::optional<uint8_t> MAX_VALUES_EQUIV_V_I_PRESSURE = 79;  // 0x4F - ratio V mA kPa
[this]() -> std::optional<float> {
        return response[A];
      };
*/

/**
 * @brief Получает максимальное значение расхода воздуха по датчику MAF
 *
 * @return std::optional<uint16_t> Расход воздуха в г/с
 */
std::optional<uint16_t> OBD2::maxMafRate() {
  ResponseType response;
  if (processPID(SERVICE_01, MAX_MAF_RATE, response)) {
    return {response[A] * 10};
  }
  return std::nullopt;
}

/**
 * @brief Получает тип топлива
 *
 * @see https://en.wikipedia.org/wiki/OBD-II_PIDs#Fuel_Type_Coding
 * @return std::optional<uint8_t> Код типа топлива
 */
std::optional<uint8_t> OBD2::fuelType() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_TYPE, response)) {
    return {response[A]};
  }
  return std::nullopt;
}

/**
 * @brief Получает процент содержания этанола в топливе
 *
 * @return std::optional<float> Процент этанола [0-100%]
 */
std::optional<float> OBD2::ethanolPercent() {
  ResponseType response;
  if (processPID(SERVICE_01, ETHANOL_FUEL_PERCENT, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает абсолютное давление паров в системе улавливания
 *
 * @return std::optional<float> Давление в кПа
 */
std::optional<float> OBD2::absEvapSysVapPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, ABS_EVAP_SYS_VAPOR_PRESSURE, response)) {
    return {((response[A] << 8) | response[B]) / 200.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает давление паров в системе улавливания (альтернативный метод)
 *
 * @return std::optional<int32_t> Давление в Па
 */
std::optional<int32_t> OBD2::evapSysVapPressure2() {
  ResponseType response;
  if (processPID(SERVICE_01, EVAP_SYS_VAPOR_PRESSURE, response)) {
    return {static_cast<int32_t>((response[A] << 8) | response[B]) - 32767};
  }
  return std::nullopt;
}

/**
 * @brief Получает краткосрочную коррекцию вторичного кислородного датчика (банки 1 и 3)
 *
 * @return std::optional<float> Коррекция в процентах [-100.0 до +99.2%]
 */
std::optional<float> OBD2::shortTermSecOxyTrim13() {
  ResponseType response;
  if (processPID(SERVICE_01, SHORT_TERM_SEC_OXY_SENS_TRIM_1_3, response)) {
    return {(response[A] * (100.0 / 128.0)) - 100.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает долгосрочную коррекцию вторичного кислородного датчика (банки 1 и 3)
 *
 * @return std::optional<float> Коррекция в процентах [-100.0 до +99.2%]
 */
std::optional<float> OBD2::longTermSecOxyTrim13() {
  ResponseType response;
  if (processPID(SERVICE_01, LONG_TERM_SEC_OXY_SENS_TRIM_1_3, response)) {
    return {(response[A] * (100.0 / 128.0)) - 100.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает краткосрочную коррекцию вторичного кислородного датчика (банки 2 и 4)
 *
 * @return std::optional<float> Коррекция в процентах [-100.0 до +99.2%]
 */
std::optional<float> OBD2::shortTermSecOxyTrim24() {
  ResponseType response;
  if (processPID(SERVICE_01, SHORT_TERM_SEC_OXY_SENS_TRIM_2_4, response)) {
    return {(response[A] * (100.0 / 128.0)) - 100.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает долгосрочную коррекцию вторичного кислородного датчика (банки 2 и 4)
 *
 * @return std::optional<float> Коррекция в процентах [-100.0 до +99.2%]
 */
std::optional<float> OBD2::longTermSecOxyTrim24() {
  ResponseType response;
  if (processPID(SERVICE_01, LONG_TERM_SEC_OXY_SENS_TRIM_2_4, response)) {
    return {(response[A] * (100.0 / 128.0)) - 100.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает абсолютное давление в топливной рампе
 *
 * @return std::optional<uint32_t> Давление в кПа
 */
std::optional<uint32_t> OBD2::absFuelRailPressure() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_RAIL_ABS_PRESSURE, response)) {
    return {((response[A] << 8) | response[B]) * 10};
  }
  return std::nullopt;
}

/**
 * @brief Получает относительное положение педали акселератора
 *
 * @return std::optional<float> Положение в процентах [0-100%]
 */
std::optional<float> OBD2::relativePedalPos() {
  ResponseType response;
  if (processPID(SERVICE_01, RELATIVE_ACCELERATOR_PEDAL_POS, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает оставшийся ресурс гибридной батареи
 *
 * @return std::optional<float> Остаток ресурса в процентах [0-100%]
 */
std::optional<float> OBD2::hybridBatLife() {
  ResponseType response;
  if (processPID(SERVICE_01, HYBRID_BATTERY_REMAINING_LIFE, response)) {
    return {response[A] * 100.0 / 255.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает температуру моторного масла
 *
 * @return std::optional<int16_t> Температура в градусах Цельсия
 */
std::optional<int16_t> OBD2::oilTemp() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_OIL_TEMP, response)) {
    return {static_cast<int16_t>(response[A]) - 40.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает угол опережения впрыска топлива
 *
 * @return std::optional<float> Угол в градусах
 */
std::optional<float> OBD2::fuelInjectTiming() {
  ResponseType response;
  if (processPID(SERVICE_01, FUEL_INJECTION_TIMING, response)) {
    return {(((response[A] << 8) | response[B]) / 128.0) - 210};
  }
  return std::nullopt;
}

/**
 * @brief Получает расход топлива двигателем
 *
 * @return std::optional<float> Расход в литрах в час
 */
std::optional<float> OBD2::fuelRate() {
  ResponseType response;
  if (processPID(SERVICE_01, ENGINE_FUEL_RATE, response)) {
    return {((response[A] << 8) | response[B]) / 20.0};
  }
  return std::nullopt;
}

/**
 * @brief Получает стандарты выбросов, для которых спроектирован автомобиль
 *
 * @return std::optional<uint8_t> Код стандарта выбросов
 */
std::optional<uint8_t> OBD2::emissionRqmts() {
  ResponseType response;
  if (processPID(SERVICE_01, EMISSION_REQUIREMENTS, response)) {
    return {response[A]};
  }
  return std::nullopt;
}