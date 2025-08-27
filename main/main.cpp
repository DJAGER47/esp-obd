#include <stdio.h>

#include "CANDriver.hpp"
#include "PIDParser.hpp"
#include "esp_log.h"

static const char *TAG = "OBD2";
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main() {
  ESP_LOGI("APP", "Starting application");

  CANDriver driver;
  PIDParser parser;

  if (driver.init()) {
    driver.start();
  }

  while (true) {
    twai_message_t message;
    if (driver.receiveMessage(message)) {
      OBD2::PID pid = static_cast<OBD2::PID>(message.data[1]);
      parser.parsePID(pid, message);
    }
    parser.printTripData();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}