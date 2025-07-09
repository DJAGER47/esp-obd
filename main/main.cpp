#include <stdio.h>

#include "CANDriver.hpp"
#include "PIDParser.hpp"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void initUART() {
  uart_config_t uart_config = {.baud_rate = 115200,
                               .data_bits = UART_DATA_8_BITS,
                               .parity    = UART_PARITY_DISABLE,
                               .stop_bits = UART_STOP_BITS_1,
                               .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
  uart_param_config(UART_NUM_0, &uart_config);
  uart_set_pin(UART_NUM_0,
               UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE);
  uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);
}

extern "C" void app_main() {
  ESP_LOGI("APP", "Starting application");

  initUART();
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