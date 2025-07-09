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

  float calculateInstant(float rpm, float speed, float injectionTime);
  float calculateAverage() const;
  void updateTotals(float fuelUsed, float distance);
  void resetTrip();
  float getTankCapacity() const;
};

class PIDParser {
 private:
  FuelCalculator fuelCalc_;
  TripCalculator tripCalc_;
  bool engineRunning_                  = false;
  static constexpr float RPM_THRESHOLD = 300.0f;

 public:
  PIDParser();

  float parseFuelConsumption(const twai_message_t& message);
  float parseEngineRPM(const twai_message_t& message);
  float parseDistanceTraveled(const twai_message_t& message);
  bool checkEngineRunning(float rpm);
  bool isEngineRunning() const;
  void getTripData(float& distance, float& fuelUsed, uint32_t& duration) const;
  float calculateTripConsumption() const;
  void printTripData() const;
  float parsePID(OBD2::PID pid, const twai_message_t& message);
};