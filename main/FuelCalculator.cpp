#include "PIDParser.hpp"

float FuelCalculator::calculateInstant(float rpm,
                                       float speed,
                                       float injectionTime) {
  if (speed < 5.0f)
    return 0.0f;

  float fuelPerCycle = injectionTime * params_.injectorFlowRate / 3600000.0f;
  float fuelPerHour  = fuelPerCycle * rpm * params_.cylinders * 60.0f;
  return (fuelPerHour / speed) * 100.0f;
}

float FuelCalculator::calculateAverage() const {
  if (totalDistance_ < 0.1f)
    return 0.0f;
  return (totalFuelUsed_ / totalDistance_) * 100.0f;
}

void FuelCalculator::updateTotals(float fuelUsed, float distance) {
  totalFuelUsed_ += fuelUsed;
  totalDistance_ += distance;
}

void FuelCalculator::resetTrip() {
  totalFuelUsed_ = 0;
  totalDistance_ = 0;
}

float FuelCalculator::getTankCapacity() const {
  return params_.fuelTankCapacity;
}