#pragma once

#include <cstdint>

#include "critical_section.h"

/**
 * @brief Класс для хранения параметров автомобиля с потокобезопасным доступом
 *
 * Все операции чтения и записи защищены критической секцией,
 * что обеспечивает консистентность данных при доступе из разных задач.
 */
class VehicleParams {
 public:
  /**
   * @brief Конструктор с инициализацией параметров по умолчанию
   */
  VehicleParams() :
      rpm_(0.0f),
      speed_(0),
      coolant_temp_(0),
      throttle_position_(0),
      engine_load_(0),
      intake_air_temp_(0),
      maf_rate_(0.0f),
      fuel_pressure_(0),
      timing_advance_(0.0f) {}

  // === Методы для оборотов двигателя (RPM) ===

  /**
   * @brief Установить обороты двигателя
   * @param rpm Обороты в об/мин
   */
  void setRpm(float rpm) {
    CriticalSection cs;
    rpm_ = rpm;
  }

  /**
   * @brief Получить обороты двигателя
   * @return Обороты в об/мин
   */
  float getRpm() const {
    CriticalSection cs;
    return rpm_;
  }

  // === Методы для скорости автомобиля ===

  /**
   * @brief Установить скорость автомобиля
   * @param speed Скорость в км/ч
   */
  void setSpeed(int speed) {
    CriticalSection cs;
    speed_ = speed;
  }

  /**
   * @brief Получить скорость автомобиля
   * @return Скорость в км/ч
   */
  int getSpeed() const {
    CriticalSection cs;
    return speed_;
  }

  // === Методы для температуры охлаждающей жидкости ===

  /**
   * @brief Установить температуру охлаждающей жидкости
   * @param temp Температура в °C
   */
  void setCoolantTemp(int temp) {
    CriticalSection cs;
    coolant_temp_ = temp;
  }

  /**
   * @brief Получить температуру охлаждающей жидкости
   * @return Температура в °C
   */
  int getCoolantTemp() const {
    CriticalSection cs;
    return coolant_temp_;
  }

  // === Методы для положения дроссельной заслонки ===

  /**
   * @brief Установить положение дроссельной заслонки
   * @param position Положение в процентах (0-100)
   */
  void setThrottlePosition(int position) {
    CriticalSection cs;
    throttle_position_ = position;
  }

  /**
   * @brief Получить положение дроссельной заслонки
   * @return Положение в процентах (0-100)
   */
  int getThrottlePosition() const {
    CriticalSection cs;
    return throttle_position_;
  }

  // === Методы для нагрузки двигателя ===

  /**
   * @brief Установить нагрузку двигателя
   * @param load Нагрузка в процентах (0-100)
   */
  void setEngineLoad(int load) {
    CriticalSection cs;
    engine_load_ = load;
  }

  /**
   * @brief Получить нагрузку двигателя
   * @return Нагрузка в процентах (0-100)
   */
  int getEngineLoad() const {
    CriticalSection cs;
    return engine_load_;
  }

  // === Методы для температуры впускного воздуха ===

  /**
   * @brief Установить температуру впускного воздуха
   * @param temp Температура в °C
   */
  void setIntakeAirTemp(int temp) {
    CriticalSection cs;
    intake_air_temp_ = temp;
  }

  /**
   * @brief Получить температуру впускного воздуха
   * @return Температура в °C
   */
  int getIntakeAirTemp() const {
    CriticalSection cs;
    return intake_air_temp_;
  }

  // === Методы для расхода воздуха (MAF) ===

  /**
   * @brief Установить расход воздуха
   * @param rate Расход в г/с
   */
  void setMafRate(float rate) {
    CriticalSection cs;
    maf_rate_ = rate;
  }

  /**
   * @brief Получить расход воздуха
   * @return Расход в г/с
   */
  float getMafRate() const {
    CriticalSection cs;
    return maf_rate_;
  }

  // === Методы для давления топлива ===

  /**
   * @brief Установить давление топлива
   * @param pressure Давление в кПа
   */
  void setFuelPressure(int pressure) {
    CriticalSection cs;
    fuel_pressure_ = pressure;
  }

  /**
   * @brief Получить давление топлива
   * @return Давление в кПа
   */
  int getFuelPressure() const {
    CriticalSection cs;
    return fuel_pressure_;
  }

  // === Методы для угла опережения зажигания ===

  /**
   * @brief Установить угол опережения зажигания
   * @param advance Угол в градусах
   */
  void setTimingAdvance(float advance) {
    CriticalSection cs;
    timing_advance_ = advance;
  }

  /**
   * @brief Получить угол опережения зажигания
   * @return Угол в градусах
   */
  float getTimingAdvance() const {
    CriticalSection cs;
    return timing_advance_;
  }

  // === Методы для массового обновления ===

  /**
   * @brief Структура для хранения основных параметров
   */
  struct BasicParams {
    float rpm;
    int speed;
    int coolant_temp;
  };

  /**
   * @brief Установить основные параметры одной операцией
   * @param params Структура с параметрами
   */
  void setBasicParams(const BasicParams& params) {
    CriticalSection cs;
    rpm_          = params.rpm;
    speed_        = params.speed;
    coolant_temp_ = params.coolant_temp;
  }

  /**
   * @brief Получить основные параметры одной операцией
   * @return Структура с параметрами
   */
  BasicParams getBasicParams() const {
    CriticalSection cs;
    return BasicParams{rpm_, speed_, coolant_temp_};
  }

  /**
   * @brief Сбросить все параметры в значения по умолчанию
   */
  void reset() {
    CriticalSection cs;
    rpm_               = 0.0f;
    speed_             = 0;
    coolant_temp_      = 0;
    throttle_position_ = 0;
    engine_load_       = 0;
    intake_air_temp_   = 0;
    maf_rate_          = 0.0f;
    fuel_pressure_     = 0;
    timing_advance_    = 0.0f;
  }

 private:
  // Основные параметры
  float rpm_;         ///< Обороты двигателя (об/мин)
  int speed_;         ///< Скорость автомобиля (км/ч)
  int coolant_temp_;  ///< Температура охлаждающей жидкости (°C)

  // Дополнительные параметры
  int throttle_position_;  ///< Положение дроссельной заслонки (%)
  int engine_load_;        ///< Нагрузка двигателя (%)
  int intake_air_temp_;    ///< Температура впускного воздуха (°C)
  float maf_rate_;         ///< Расход воздуха (г/с)
  int fuel_pressure_;      ///< Давление топлива (кПа)
  float timing_advance_;   ///< Угол опережения зажигания (градусы)
};