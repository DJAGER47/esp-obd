#include <stdio.h>
#include <sys/lock.h>
#include <sys/param.h>
#include <unistd.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"
// #include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "io.h"
#include "iso_tp.h"
// #include "lvgl.h"
// #include "lvgl_port.h"
#include "obd2.h"
// #include "ssd1283a.h"
#include "twai_driver.h"

// Глобальный экземпляр CAN драйвера
// TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с

// Глобальный экземпляр дисплея SSD1283A MOSI, SCLK, CS, DC, RST
// SSD1283A display(LCD_MOSI_PIN, LCD_SCLK_PIN, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, SPI2_HOST);

// Функция для нового потока
void iso_tp_task(void *pvParameters) {
  // Создаем экземпляр IsoTp, передавая ссылку на CAN драйвер
  // IPhyInterface &phy_interface = can_driver;
  // IsoTp iso_tp(phy_interface);
  // OBD2 obd(iso_tp);

  // Здесь можно добавить логику работы с IsoTp
  // Например, отправка и прием сообщений
  while (true) {
    // Добавьте здесь логику работы с ISO-TP
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static const char *TAG = "OBD2";
extern "C" void app_main() {
  ESP_LOGI("APP", "Starting application");

  // Инициализация CAN драйвера
  // can_driver.InstallStart();

  // Инициализация дисплея SSD1283A
  // if (!display.init()) {
  //   ESP_LOGE(TAG, "Failed to initialize display");
  //   return;
  // }

  // Инициализация LVGL
  // if (!lvgl_port_init(&display)) {
  //   ESP_LOGE(TAG, "Failed to initialize LVGL");
  //   return;
  // }

  // // Установка таймера для LVGL
  // ESP_LOGI(TAG, "Install LVGL tick timer");
  // const esp_timer_create_args_t lvgl_tick_timer_args = {.callback = &example_increase_lvgl_tick,
  //                                                       .name     = "lvgl_tick"};
  // esp_timer_handle_t lvgl_tick_timer                 = NULL;
  // ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  // ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

  // // Создание задачи LVGL
  // ESP_LOGI(TAG, "Create LVGL task");
  // xTaskCreate(example_lvgl_port_task,
  //             "LVGL",
  //             EXAMPLE_LVGL_TASK_STACK_SIZE,
  //             NULL,
  //             EXAMPLE_LVGL_TASK_PRIORITY,
  //             NULL);

  // // Отображение UI OBD2
  // ESP_LOGI(TAG, "Display OBD2 UI");
  // // Lock the mutex due to the LVGL APIs are not thread-safe
  // _lock_acquire(&lvgl_api_lock);
  // example_lvgl_obd2_ui(lv_display_get_default());
  // _lock_release(&lvgl_api_lock);

  // Инициализация OBD2
  // if (!obd.begin(can_driver, true, 1000)) {
  //   ESP_LOGE(TAG, "Failed to initialize OBD2");
  //   return;
  // }

  // Создание нового потока для работы с ISO-TP
  // xTaskCreate(iso_tp_task, "iso_tp_task", 4096, NULL, 5, NULL);

  // Инициализация GPIO для светодиода
  gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

  while (true) {
    // Моргание светодиодом
    gpio_set_level(LED_GPIO, 1);  // Включить
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(LED_GPIO, 0);  // Выключить
    // twai_message_t message;
    // if (driver.receiveMessage(message)) {
    //   OBD2::PID pid = static_cast<OBD2::PID>(message.data[1]);
    //   parser.parsePID(pid, message);
    // }
    // parser.printTripData();

    // Пример использования OBD2 для запроса RPM двигателя
    // obd.queryPID(0x01, 0x0C);  // Сервис 01, PID 0C (RPM)
    // double rpm = obd.rpm();
    // ESP_LOGI(TAG, "Engine RPM: %.2f", rpm);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}