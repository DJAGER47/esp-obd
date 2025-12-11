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
 * @brief Получает список поддерживаемых PID в диапазоне 81-100
 *
 * Возвращает битовую маску, где каждый бит указывает на поддержку соответствующего PID.
 *
 * @return std::optional<uint32_t> Битовая маска поддерживаемых PID
 */
std::optional<uint32_t> OBD2::supportedPIDs81_100() {
  return getSupportedPIDs(SUPPORTED_PIDS_81_100);
}

/**
 * @brief Engine run time for Auxiliary Emissions Control Device(AECD)
 *
 * @return std::optional<uint32_t> Время работы в секундах
 */
std::optional<uint32_t> OBD2::engineRunTimeAECD1_2() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, ENGINE_RUN_TIME_AECD_1_2, response)) {
    return {((response[OBD2::A] << 24) | (response[OBD2::B] << 16) | (response[OBD2::C] << 8) | response[OBD2::D])};
  }
  return std::nullopt;
}

/**
 * @brief Engine run time for Auxiliary Emissions Control Device(AECD)
 *
 * @return std::optional<uint32_t> Время работы в секундах
 */
std::optional<uint32_t> OBD2::engineRunTimeAECD3_4() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, ENGINE_RUN_TIME_AECD_3_4, response)) {
    return {((response[OBD2::A] << 24) | (response[OBD2::B] << 16) | (response[OBD2::C] << 8) | response[OBD2::D])};
  }
  return std::nullopt;
}

/**
 * @brief NOx sensor
 *
 * @return std::optional<std::array<uint16_t, 2>> Концентрация NOx в ppm для двух датчиков
 */
std::optional<std::array<uint16_t, 2>> OBD2::noxSensor() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, NOX_SENSOR, response)) {
    std::array<uint16_t, 2> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Manifold surface temperature
 *
 * @return std::optional<int16_t> Температура в градусах Цельсия
 */
std::optional<int16_t> OBD2::manifoldSurfaceTemp() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, MANIFOLD_SURFACE_TEMP, response)) {
    return {static_cast<int16_t>((response[OBD2::A] << 8) | response[OBD2::B]) - 40};
  }
  return std::nullopt;
}

/**
 * @brief NOx reagent system
 *
 * @return std::optional<float> Уровень реагента в процентах [0-100%]
 */
std::optional<float> OBD2::noxReagentSystem() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, NOX_REAGENT_SYSTEM, response)) {
    return {response[OBD2::A] * 100.0f / 255.0f};
  }
  return std::nullopt;
}

/**
 * @brief Particulate matter (PM) sensor
 *
 * @return std::optional<std::array<uint16_t, 3>> Данные PM сенсора: [масса в мкг/м3, количество света, температура в
 * °C]
 */
std::optional<std::array<uint16_t, 3>> OBD2::pmSensor() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, PM_SENSOR, response)) {
    std::array<uint16_t, 3> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];  // Масса
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];  // Количество света
    result[2] = (response[OBD2::E] << 8) | response[OBD2::F];  // Температура
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Intake manifold absolute pressure
 *
 * @return std::optional<uint16_t> Давление в кПа
 */
std::optional<uint16_t> OBD2::intakeManifoldAbsPressure() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, INTAKE_MANIFOLD_ABS_PRESSURE_81_100, response)) {
    return {(response[OBD2::A] << 8) | response[OBD2::B]};
  }
  return std::nullopt;
}

/**
 * @brief SCR Induce System
 *
 * @return std::optional<std::array<uint16_t, 5>> Состояние системы SCR: [состояние, время до следующей дозировки, время
 * с последней дозировки, время работы насоса, время работы клапана]
 */
std::optional<std::array<uint16_t, 5>> OBD2::scrInduceSystem() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, SCR_INDUCE_SYSTEM, response)) {
    std::array<uint16_t, 5> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];  // Состояние
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];  // Время до следующей дозировки
    result[2] = (response[OBD2::E] << 8) | response[OBD2::F];  // Время с последней дозировки
    result[3] = (response[OBD2::G] << 8) | response[OBD2::H];  // Время работы насоса
    // response[OBD2::H] не используется
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Run Time for AECD #11-#15
 *
 * @return std::optional<uint32_t> Время работы в секундах
 */
std::optional<uint32_t> OBD2::runTimeAECD11_15() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, RUN_TIME_AECD_11_15, response)) {
    return {((response[OBD2::A] << 24) | (response[OBD2::B] << 16) | (response[OBD2::C] << 8) | response[OBD2::D])};
  }
  return std::nullopt;
}

/**
 * @brief Run Time for AECD #16-#20
 *
 * @return std::optional<uint32_t> Время работы в секундах
 */
std::optional<uint32_t> OBD2::runTimeAECD16_20() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, RUN_TIME_AECD_16_20, response)) {
    return {((response[OBD2::A] << 24) | (response[OBD2::B] << 16) | (response[OBD2::C] << 8) | response[OBD2::D])};
  }
  return std::nullopt;
}

/**
 * @brief Diesel Aftertreatment
 *
 * @return std::optional<std::array<uint16_t, 7>> Состояние системы очистки выхлопа: [состояние, температура 1,
 * температура 2, температура 3, температура 4, давление 1, давление 2]
 */
std::optional<std::array<uint16_t, 7>> OBD2::dieselAftertreatment() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, DIESEL_AFTERTREATMENT, response)) {
    std::array<uint16_t, 7> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];  // Состояние
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];  // Температура 1
    result[2] = (response[OBD2::E] << 8) | response[OBD2::F];  // Температура 2
    result[3] = (response[OBD2::G] << 8) | response[OBD2::H];  // Температура 3
    // response[OBD2::H] не используется
    return result;
  }
  return std::nullopt;
}

/**
 * @brief O2 Sensor (Wide Range)
 *
 * @return std::optional<std::array<float, 2>> Напряжение датчиков O2 в вольтах
 */
std::optional<std::array<float, 2>> OBD2::o2SensorWideRange() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, O2_SENSOR_WIDE_RANGE, response)) {
    std::array<float, 2> result;
    result[0] = ((response[OBD2::A] << 8) | response[OBD2::B]) * 0.001f;  // Напряжение 1
    result[1] = ((response[OBD2::C] << 8) | response[OBD2::D]) * 0.001f;  // Напряжение 2
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Throttle Position G
 *
 * @return std::optional<float> Положение дроссельной заслонки в процентах [0-100%]
 */
std::optional<float> OBD2::throttlePositionG() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, THROTTLE_POSITION_G, response)) {
    return {response[OBD2::A] * 100.0f / 255.0f};
  }
  return std::nullopt;
}

/**
 * @brief Engine Friction - Percent Torque
 *
 * @return std::optional<int16_t> Процент крутящего момента [-125..125%]
 */
std::optional<int16_t> OBD2::engineFrictionPercentTorque() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, ENGINE_FRICTION_PERCENT_TORQUE, response)) {
    return {static_cast<int16_t>(response[OBD2::A]) - 125};
  }
  return std::nullopt;
}

/**
 * @brief PM Sensor Bank 1 & 2
 *
 * @return std::optional<std::array<uint16_t, 4>> Данные PM сенсоров: [масса 1, масса 2, температура 1, температура 2]
 */
std::optional<std::array<uint16_t, 4>> OBD2::pmSensorBank1_2() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, PM_SENSOR_BANK_1_2, response)) {
    std::array<uint16_t, 4> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];  // Масса 1
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];  // Масса 2
    result[2] = (response[OBD2::E] << 8) | response[OBD2::F];  // Температура 1
    result[3] = (response[OBD2::G] << 8) | response[OBD2::H];  // Температура 2
    return result;
  }
  return std::nullopt;
}

/**
 * @brief WWH-OBD Vehicle OBD System Information
 *
 * @return std::optional<uint16_t> Информация о системе OBD
 */
std::optional<uint16_t> OBD2::wwhObdVehicleInfo() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, WWH_OBD_VEHICLE_INFO_1, response)) {
    return {(response[OBD2::A] << 8) | response[OBD2::B]};
  }
  return std::nullopt;
}

/**
 * @brief WWH-OBD Vehicle OBD System Information
 *
 * @return std::optional<uint16_t> Информация о системе OBD
 */
std::optional<uint16_t> OBD2::wwhObdVehicleInfo2() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, WWH_OBD_VEHICLE_INFO_2, response)) {
    return {(response[OBD2::A] << 8) | response[OBD2::B]};
  }
  return std::nullopt;
}

/**
 * @brief Fuel System Control
 *
 * @return std::optional<uint16_t> Управление топливной системой
 */
std::optional<uint16_t> OBD2::fuelSystemControl() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, FUEL_SYSTEM_CONTROL, response)) {
    return {(response[OBD2::A] << 8) | response[OBD2::B]};
  }
  return std::nullopt;
}

/**
 * @brief WWH-OBD Vehicle OBD Counters support
 *
 * @return std::optional<uint16_t> Поддержка счетчиков OBD
 */
std::optional<uint16_t> OBD2::wwhObdCountersSupport() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, WWH_OBD_COUNTERS_SUPPORT, response)) {
    return {(response[OBD2::A] << 8) | response[OBD2::B]};
  }
  return std::nullopt;
}

/**
 * @brief NOx Warning And Inducement System
 *
 * @return std::optional<std::array<uint16_t, 4>> Состояние системы предупреждения NOx: [состояние, время до
 * индуцирования, время индуцирования, количество индуцирований]
 */
std::optional<std::array<uint16_t, 4>> OBD2::noxWarningInducementSystem() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, NOX_WARNING_INDUCTION_SYSTEM, response)) {
    std::array<uint16_t, 4> result;
    result[0] = (response[OBD2::A] << 8) | response[OBD2::B];  // Состояние
    result[1] = (response[OBD2::C] << 8) | response[OBD2::D];  // Время до индуцирования
    // response[OBD2::E] и response[OBD2::F] не используются
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Exhaust Gas Temperature Sensor
 *
 * @return std::optional<std::array<int16_t, 2>> Температура выхлопных газов в °C для двух датчиков
 */
std::optional<std::array<int16_t, 2>> OBD2::exhaustGasTempSensor() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, EXHAUST_GAS_TEMP_SENSOR_1, response)) {
    std::array<int16_t, 2> result;
    result[0] = static_cast<int16_t>((response[OBD2::A] << 8) | response[OBD2::B]) - 40;  // Температура 1
    result[1] = static_cast<int16_t>((response[OBD2::C] << 8) | response[OBD2::D]) - 40;  // Температура 2
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Exhaust Gas Temperature Sensor
 *
 * @return std::optional<std::array<int16_t, 2>> Температура выхлопных газов в °C для двух датчиков
 */
std::optional<std::array<int16_t, 2>> OBD2::exhaustGasTempSensor2() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, EXHAUST_GAS_TEMP_SENSOR_2, response)) {
    std::array<int16_t, 2> result;
    result[0] = static_cast<int16_t>((response[OBD2::A] << 8) | response[OBD2::B]) - 40;  // Температура 1
    result[1] = static_cast<int16_t>((response[OBD2::C] << 8) | response[OBD2::D]) - 40;  // Температура 2
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Hybrid/EV Vehicle System Data, Battery, Voltage
 *
 * @return std::optional<float> Напряжение батареи в вольтах
 */
std::optional<float> OBD2::hybridEvBatteryVoltage() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, HYBRID_EV_BATTERY_VOLTAGE, response)) {
    return {((response[OBD2::A] << 8) | response[OBD2::B]) * 0.001f};
  }
  return std::nullopt;
}

/**
 * @brief Diesel Exhaust Fluid Sensor Data
 *
 * @return std::optional<float> Уровень DEF в процентах [0-100%]
 */
std::optional<float> OBD2::dieselExhaustFluidSensor() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, DIESEL_EXHAUST_FLUID_SENSOR_DATA, response)) {
    return {response[OBD2::D] * 100.0f / 255.0f};
  }
  return std::nullopt;
}

/**
 * @brief O2 Sensor Data
 *
 * @return std::optional<std::array<float, 4>> Данные датчиков O2: [напряжение 1, напряжение 2, ток 1, ток 2]
 */
std::optional<std::array<float, 4>> OBD2::o2SensorData() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, O2_SENSOR_DATA_81_100, response)) {
    std::array<float, 4> result;
    result[0] = ((response[OBD2::A] << 8) | response[OBD2::B]) * 0.001f;  // Напряжение 1
    result[1] = ((response[OBD2::C] << 8) | response[OBD2::D]) * 0.001f;  // Напряжение 2
    result[2] = ((response[OBD2::E] << 8) | response[OBD2::F]) * 0.001f;  // Ток 1
    result[3] = ((response[OBD2::G] << 8) | response[OBD2::H]) * 0.001f;  // Ток 2
    return result;
  }
  return std::nullopt;
}

/**
 * @brief Engine Fuel Rate
 *
 * @return std::optional<float> Расход топлива в г/с
 */
std::optional<float> OBD2::engineFuelRate() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, ENGINE_FUEL_RATE_81_100, response)) {
    return {((response[OBD2::A] << 8) | response[OBD2::B]) * 0.05f};
  }
  return std::nullopt;
}

/**
 * @brief Engine Exhaust Flow Rate
 *
 * @return std::optional<float> Поток выхлопа в кг/ч
 */
std::optional<float> OBD2::engineExhaustFlowRate() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, ENGINE_EXHAUST_FLOW_RATE, response)) {
    return {((response[OBD2::A] << 8) | response[OBD2::B]) * 0.1f};
  }
  return std::nullopt;
}

/**
 * @brief Fuel System Percentage Use
 *
 * @return std::optional<std::array<float, 4>> Процент использования топлива: [дизель, бензин, CNG, LPG]
 */
std::optional<std::array<float, 4>> OBD2::fuelSystemPercentageUse() {
  OBD2::ResponseType response;
  if (processPID(SERVICE_01, FUEL_SYSTEM_PERCENTAGE_USE, response)) {
    std::array<float, 4> result;
    result[0] = response[OBD2::A] * 100.0f / 255.0f;  // Дизель
    result[1] = response[OBD2::B] * 100.0f / 255.0f;  // Бензин
    result[2] = response[OBD2::C] * 100.0f / 255.0f;  // CNG
    result[3] = response[OBD2::D] * 100.0f / 255.0f;  // LPG
    return result;
  }
  return std::nullopt;
}
