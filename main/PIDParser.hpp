#pragma once

#include <cstdint>

#include "TripCalculator.hpp"
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
  TripCalculator tripCalc_;
  bool engineRunning_ = false;
  static constexpr float RPM_THRESHOLD =
      300.0f;  // Порог оборотов для определения работы двигателя

 public:
  PIDParser() :
      fuelCalc_({55.0f, 0.74f, 4, 2.5f}) {}  // Пример параметров для авто

  // Парсинг PID 0x2F - Fuel Tank Level Input
  float parseFuelConsumption(const twai_message_t& message) {
    // Используем стандартную формулу из OBD2
    float level    = OBD2::PidCalculator::calculateFuelLevel(&message.data[0]);
    float fuelUsed = fuelCalc_.getTankCapacity() * (level / 100.0f);
    fuelCalc_.updateTotals(fuelUsed, 0);
    if (engineRunning_) {
      tripCalc_.updateTrip(0, fuelUsed);
    }
    return fuelUsed;
  }

  // Парсинг PID 0x0C - Engine RPM
  float parseEngineRPM(const twai_message_t& message) {
    if (message.data_length_code < 4)
      return 0.0f;
    // Формула: ((A*256)+B)/4 [об/мин]
    return static_cast<float>((message.data[2] << 8) | message.data[3]) / 4.0f;
  }

  // Парсинг PID 0x31 - Distance Traveled Since Codes Cleared
  float parseDistanceTraveled(const twai_message_t& message) {
    // Получаем пройденное расстояние в км
    float distance =
        static_cast<float>((message.data[0] << 8) | message.data[1]);
    fuelCalc_.updateTotals(0, distance);
    if (engineRunning_) {
      tripCalc_.updateTrip(distance, 0);
    }
    return distance;
  }

  // Определение состояния двигателя по оборотам
  bool checkEngineRunning(float rpm) {
    engineRunning_ = rpm > RPM_THRESHOLD;
    return engineRunning_;
  }

  // Получение текущего состояния двигателя
  bool isEngineRunning() const {
    return engineRunning_;
  }

  // Получение данных о текущей поездке
  void getTripData(float& distance, float& fuelUsed, uint32_t& duration) const {
    uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
    distance             = tripCalc_.getTripDistance();
    fuelUsed             = tripCalc_.getTripFuelUsed();
    duration             = tripCalc_.getTripDuration(currentTime);
  }

  // Расчет расхода топлива за поездку (л/100км)
  float calculateTripConsumption() const {
    float distance, fuelUsed;
    uint32_t duration;
    getTripData(distance, fuelUsed, duration);

    if (distance < 0.1f) {
      return 0.0f;
    }
    return (fuelUsed / distance) * 100.0f;
  }

  // Вывод данных о поездке через UART
  void printTripData() const {
    float distance, fuelUsed;
    uint32_t duration;
    getTripData(distance, fuelUsed, duration);

    printf("Поездка: %.1f км, %.1f л, %.1f л/100км, %02d:%02d:%02d\n",
           distance,
           fuelUsed,
           calculateTripConsumption(),
           duration / 3600,
           (duration % 3600) / 60,
           duration % 60);
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
      case OBD2::PID::RPM: {
        float rpm       = parseEngineRPM(message);
        bool wasRunning = engineRunning_;
        checkEngineRunning(rpm);

        // Управление состоянием поездки
        if (engineRunning_ && !wasRunning) {
          // Двигатель запустился - начинаем новую поездку
          tripCalc_.startTrip(xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
        } else if (!engineRunning_ && wasRunning) {
          // Двигатель остановился - завершаем поездку
          tripCalc_.endTrip();
        }

        return rpm;
      }
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