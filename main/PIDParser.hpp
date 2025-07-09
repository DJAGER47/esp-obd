#pragma once

#include <cstdint>

#include "driver/twai.h"
#include "obd2_pid.h"

class FuelCalculator {
 private:
  struct VehicleParams {
    float fuelTankCapacity;  // Объем топливного бака (литры)
    float fuelDensity;       // Плотность топлива (кг/л)
    uint8_t cylinders;       // Количество цилиндров
    float injectorFlowRate;  // Производительность форсунки (л/ч)
  };

  VehicleParams params_;
  float totalFuelUsed_ = 0;  // Суммарный расход топлива (л)
  float totalDistance_ = 0;  // Суммарный пробег (км)

 public:
  explicit FuelCalculator(const VehicleParams& params) :
      params_(params) {}

  // Расчет мгновенного расхода (л/100км)
  float calculateInstant(float rpm, float speed, float injectionTime) {
    if (speed < 5.0f)
      return 0.0f;  // Игнорируем при стоянке

    // Расчет расхода за цикл
    float fuelPerCycle = injectionTime * params_.injectorFlowRate / 3600000.0f;
    // Расчет расхода в час
    float fuelPerHour = fuelPerCycle * rpm * params_.cylinders * 60.0f;
    // Пересчет в л/100км
    return (fuelPerHour / speed) * 100.0f;
  }

  // Расчет среднего расхода (л/100км)
  float calculateAverage() const {
    if (totalDistance_ < 0.1f)
      return 0.0f;
    return (totalFuelUsed_ / totalDistance_) * 100.0f;
  }

  // Обновление суммарных значений
  void updateTotals(float fuelUsed, float distance) {
    totalFuelUsed_ += fuelUsed;
    totalDistance_ += distance;
  }

  // Сброс данных поездки
  void resetTrip() {
    totalFuelUsed_ = 0;
    totalDistance_ = 0;
  }

  // Получение объема топливного бака
  float getTankCapacity() const {
    return params_.fuelTankCapacity;
  }
};

class PIDParser {
 private:
  FuelCalculator fuelCalc_;

 public:
  PIDParser() :
      fuelCalc_({55.0f, 0.74f, 4, 2.5f}) {}  // Пример параметров для авто

  // Парсинг PID 0x2F - Fuel Tank Level Input
  float parseFuelConsumption(const twai_message_t& message) {
    // Используем стандартную формулу из OBD2
    float level    = OBD2::PidCalculator::calculateFuelLevel(message.data[0]);
    float fuelUsed = fuelCalc_.getTankCapacity() * (level / 100.0f);
    fuelCalc_.updateTotals(fuelUsed, 0);
    return fuelUsed;
  }

  // Парсинг PID 0x31 - Distance Traveled Since Codes Cleared
  float parseDistanceTraveled(const twai_message_t& message) {
    // Получаем пройденное расстояние в км
    float distance =
        static_cast<float>((message.data[0] << 8) | message.data[1]);
    fuelCalc_.updateTotals(0, distance);
    return distance;
  }

  // Общий метод для парсинга любого PID
  float parsePID(OBD2::PID pid, const twai_message_t& message) {
    if (message.data_length_code < 2)
      return 0.0f;  // Минимум 2 байта данных

    switch (pid) {
      case OBD2::PID::FUEL_LEVEL:
        return parseFuelConsumption(message);
      case OBD2::PID::DISTANCE_TRAVELED:
        return parseDistanceTraveled(message);
      case OBD2::PID::RPM:
        if (message.data_length_code < 5)
          return 0.0f;
        return OBD2::PidCalculator::calculateRpm(message.data);
      case OBD2::PID::SPEED:
        return OBD2::PidCalculator::calculateSpeed(message.data);
      case OBD2::PID::COOLANT_TEMP:
        return OBD2::PidCalculator::calculateCoolantTemp(message.data);
      case OBD2::PID::ENGINE_LOAD:
        return OBD2::PidCalculator::calculateEngineLoad(message.data);
      case OBD2::PID::INTAKE_PRESSURE:
        return OBD2::PidCalculator::calculateIntakePressure(message.data);
      default:
        return 0.0f;  // Неизвестный PID
    }
  }
};