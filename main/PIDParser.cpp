#include "PIDParser.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

PIDParser::PIDParser() :
    fuelCalc_({55.0f, 0.74f, 4, 2.5f}) {}

void PIDParser::printTripData() const {
  float distance, fuelUsed;
  uint32_t duration;
  getTripData(distance, fuelUsed, duration);

  printf("Поездка: %.1f км, %.1f л, %.1f л/100км, %02lu:%02lu:%02lu\n",
         distance,
         fuelUsed,
         calculateTripConsumption(),
         duration / 3600,
         (duration % 3600) / 60,
         duration % 60);
}

float PIDParser::parseFuelConsumption(const twai_message_t& message) {
  float level    = OBD2::PidCalculator::calculateFuelLevel(&message.data[0]);
  float fuelUsed = fuelCalc_.getTankCapacity() * (level / 100.0f);
  fuelCalc_.updateTotals(fuelUsed, 0);
  if (engineRunning_) {
    tripCalc_.updateTrip(0, fuelUsed);
  }
  return fuelUsed;
}

float PIDParser::parseEngineRPM(const twai_message_t& message) {
  if (message.data_length_code < 4)
    return 0.0f;
  return static_cast<float>((message.data[2] << 8) | message.data[3]) / 4.0f;
}

float PIDParser::parseDistanceTraveled(const twai_message_t& message) {
  float distance = static_cast<float>((message.data[0] << 8) | message.data[1]);
  fuelCalc_.updateTotals(0, distance);
  if (engineRunning_) {
    tripCalc_.updateTrip(distance, 0);
  }
  return distance;
}

bool PIDParser::checkEngineRunning(float rpm) {
  engineRunning_ = rpm > RPM_THRESHOLD;
  return engineRunning_;
}

void PIDParser::getTripData(float& distance,
                            float& fuelUsed,
                            uint32_t& duration) const {
  uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
  distance             = tripCalc_.getTripDistance();
  fuelUsed             = tripCalc_.getTripFuelUsed();
  duration             = tripCalc_.getTripDuration(currentTime);
}

float PIDParser::calculateTripConsumption() const {
  float distance, fuelUsed;
  uint32_t duration;
  getTripData(distance, fuelUsed, duration);

  if (distance < 0.1f) {
    return 0.0f;
  }
  return (fuelUsed / distance) * 100.0f;
}

float PIDParser::parsePID(OBD2::PID pid, const twai_message_t& message) {
  if (message.data_length_code < 2)
    return 0.0f;

  switch (pid) {
    case OBD2::PID::FUEL_LEVEL:
      return parseFuelConsumption(message);
    case OBD2::PID::DISTANCE_TRAVELED:
      return parseDistanceTraveled(message);
    case OBD2::PID::RPM: {
      float rpm       = parseEngineRPM(message);
      bool wasRunning = engineRunning_;
      checkEngineRunning(rpm);

      if (engineRunning_ && !wasRunning) {
        tripCalc_.startTrip(xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
      } else if (!engineRunning_ && wasRunning) {
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
      return 0.0f;
  }
}