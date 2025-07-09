#include <stdio.h>

#include "CANDriver.hpp"
#include "PIDParser.hpp"
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
    driver.processMessages();
    parser.printTripData();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}