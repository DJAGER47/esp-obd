#pragma once

#include <cstdint>

class TripCalculator {
 private:
  float tripDistance_     = 0;  // Пробег за поездку (км)
  float tripFuelUsed_     = 0;  // Расход топлива за поездку (л)
  uint32_t tripStartTime_ = 0;  // Время начала поездки (секунды)
  bool tripInProgress_    = false;

 public:
  // Начать новую поездку
  void startTrip(uint32_t currentTime) {
    tripDistance_   = 0;
    tripFuelUsed_   = 0;
    tripStartTime_  = currentTime;
    tripInProgress_ = true;
  }

  // Завершить поездку
  void endTrip() {
    tripInProgress_ = false;
  }

  // Обновить данные поездки
  void updateTrip(float distance, float fuel) {
    if (tripInProgress_) {
      tripDistance_ += distance;
      tripFuelUsed_ += fuel;
    }
  }

  // Получить пробег за поездку
  float getTripDistance() const {
    return tripDistance_;
  }

  // Получить расход топлива за поездку
  float getTripFuelUsed() const {
    return tripFuelUsed_;
  }

  // Получить время поездки
  uint32_t getTripDuration(uint32_t currentTime) const {
    return tripInProgress_ ? (currentTime - tripStartTime_) : 0;
  }

  // Проверить активность поездки
  bool isTripInProgress() const {
    return tripInProgress_;
  }
};