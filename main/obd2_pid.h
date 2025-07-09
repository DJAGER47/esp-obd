// obd2_pid.h
#pragma once

#include <cstdint>

/**
 * Стандартные PID OBD2 (SAE J1979)
 * Источник: https://github.com/OBDb/SAEJ1979
 */
namespace OBD2 {

static const uint8_t A = 3;
static const uint8_t B = 4;
static const uint8_t C = 5;
static const uint8_t D = 6;
static const uint8_t E = 7;

// Режимы работы
enum class Mode : uint8_t {
  SHOW_CURRENT = 0x01,  // Текущие данные
  SHOW_FREEZE  = 0x02,  // Данные freeze frame
  SHOW_DTC     = 0x03,  // Диагностические коды неисправностей
  CLEAR_DTC    = 0x04,  // Очистка DTC
  TEST_RESULTS = 0x05,  // Результаты тестов
  SHOW_PENDING = 0x07,  // Ожидающие DTC
  CONTROL      = 0x08,  // Управление
  VEHICLE_INFO = 0x09   // Информация о транспортном средстве
};

// Стандартные PID (Service 01)
enum class PID : uint8_t {
  SUPPORTED_PIDS    = 0x00,  // Поддерживаемые PID
  STATUS            = 0x01,  // Статус DTC
  FREEZE_DTC        = 0x02,  // Freeze DTC
  FUEL_STATUS       = 0x03,  // Состояние топливной системы
  ENGINE_LOAD       = 0x04,  // Нагрузка двигателя (%)
  COOLANT_TEMP      = 0x05,  // Температура охлаждающей жидкости (°C)
  SHORT_FUEL_TRIM_1 = 0x06,  // Короткая коррекция топлива банк 1 (%)
  LONG_FUEL_TRIM_1  = 0x07,  // Длинная коррекция топлива банк 1 (%)
  SHORT_FUEL_TRIM_2 = 0x08,  // Короткая коррекция топлива банк 2 (%)
  LONG_FUEL_TRIM_2  = 0x09,  // Длинная коррекция топлива банк 2 (%)
  FUEL_PRESSURE     = 0x0A,  // Давление топлива (kPa)
  INTAKE_PRESSURE   = 0x0B,  // Давление во впускном коллекторе (kPa)
  RPM               = 0x0C,  // Обороты двигателя (об/мин)
  SPEED             = 0x0D,  // Скорость автомобиля (км/ч)
  TIMING_ADVANCE    = 0x0E,  // Угол опережения зажигания (°)
  INTAKE_TEMP       = 0x0F,  // Температура впускного воздуха (°C)
  MAF_FLOW          = 0x10,  // Расход воздуха (г/с)
  THROTTLE_POS      = 0x11,  // Положение дроссельной заслонки (%)
  // ... другие PID
  FUEL_LEVEL        = 0x2F,  // Уровень топлива (%)
  DISTANCE_TRAVELED = 0x31   // Пройденное расстояние (км)
};

/**
 * Формулы для расчета значений PID
 * Источник: https://en.wikipedia.org/wiki/OBD-II_PIDs
 */
class PidCalculator {
 public:
  // Расчет оборотов двигателя (PID 0x0C) - использует байты A и B (3 и 4)
  static float calculateRpm(const uint8_t* data) {
    return (256.0f * data[A] + data[B]) / 4.0f;  // [об/мин]
  }

  // Расчет скорости (PID 0x0D) - использует байт A (3)
  static float calculateSpeed(const uint8_t* data) {
    return static_cast<float>(data[A]);  // [км/ч]
  }

  // Расчет температуры охлаждающей жидкости (PID 0x05) - использует байт A (3)
  static float calculateCoolantTemp(const uint8_t* data) {
    return data[A] - 40.0f;  // [°C]
  }

  // Расчет нагрузки двигателя (PID 0x04) - использует байт A
  static float calculateEngineLoad(const uint8_t* data) {
    return 100.0f * data[A] / 255.0f;  // [%]
  }

  // Расчет давления во впускном коллекторе (PID 0x0B) - использует байт A
  static float calculateIntakePressure(const uint8_t* data) {
    return static_cast<float>(data[A]);  // [kPa]
  }

  // Расчет уровня топлива (PID 0x2F) - использует байт A
  static float calculateFuelLevel(const uint8_t* data) {
    return 100.0f * data[A] / 255.0f;  // [%]
  }

  // Расчет пройденного расстояния (PID 0x31) - использует байты A и B
  static float calculateDistance(const uint8_t* data) {
    return 256.0f * data[A] + data[B];  // [км]
  }
};
}  // namespace OBD2