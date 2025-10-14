#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "io.h"
#include "iso_tp.h"
#include "lvgl.h"
#include "lvgl_port.h"
#include "obd2.h"
#include "ssd1283a.h"
#include "twai_driver.h"

// Глобальный экземпляр CAN драйвера
// TwaiDriver can_driver(CAN_TX_PIN, CAN_RX_PIN, 500);  // TX, RX, 500 кбит/с

// Глобальный экземпляр дисплея SSD1283A MOSI, SCLK, CS, DC, RST
// SSD1283A display(LCD_MOSI_PIN, LCD_SCLK_PIN, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, SPI2_HOST);

// Функция для нового потока
void iso_tp_task(void *pvParameters) {
  // Создаем экземпляр IsoTp, передавая ссылку на CAN драйвер
  IPhyInterface &phy_interface = can_driver;
  IsoTp iso_tp(phy_interface);
  OBD2 obd(iso_tp);

  // Здесь можно добавить логику работы с IsoTp
  // Например, отправка и прием сообщений
  while (true) {
    // Добавьте здесь логику работы с ISO-TP
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Функция для демонстрации LVGL
void lvgl_demo_task(void *pvParameters) {
  // Создаем простой интерфейс
  lv_obj_t *scr = lv_scr_act();

  // Создаем метку
  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text(label, "OBD2 Display");
  lv_obj_set_style_text_color(label, lv_color_hex(0x00ff00), LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

  // Создаем контейнер для данных
  lv_obj_t *cont = lv_obj_create(scr);
  lv_obj_set_size(cont, 120, 80);
  lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_border_color(cont, lv_color_hex(0x00ff00), LV_PART_MAIN);
  lv_obj_set_style_border_width(cont, 2, LV_PART_MAIN);

  // Создаем метки для данных
  lv_obj_t *rpm_label = lv_label_create(cont);
  lv_label_set_text(rpm_label, "RPM: 0");
  lv_obj_set_style_text_color(rpm_label, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_align(rpm_label, LV_ALIGN_TOP_LEFT, 5, 5);

  lv_obj_t *speed_label = lv_label_create(cont);
  lv_label_set_text(speed_label, "Speed: 0 km/h");
  lv_obj_set_style_text_color(speed_label, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_align(speed_label, LV_ALIGN_TOP_LEFT, 5, 25);

  lv_obj_t *temp_label = lv_label_create(cont);
  lv_label_set_text(temp_label, "Temp: 0°C");
  lv_obj_set_style_text_color(temp_label, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_align(temp_label, LV_ALIGN_TOP_LEFT, 5, 45);

  // Обновляем данные каждые 2 секунды
  while (true) {
    static int rpm   = 800;
    static int speed = 0;
    static int temp  = 90;

    // Симуляция изменения данных
    rpm += (esp_random() % 200) - 100;
    if (rpm < 800)
      rpm = 800;
    if (rpm > 6000)
      rpm = 6000;

    speed += (esp_random() % 10) - 5;
    if (speed < 0)
      speed = 0;
    if (speed > 200)
      speed = 200;

    temp += (esp_random() % 3) - 1;
    if (temp < 70)
      temp = 70;
    if (temp > 110)
      temp = 110;

    // Обновляем метки
    lv_label_set_text_fmt(rpm_label, "RPM: %d", rpm);
    lv_label_set_text_fmt(speed_label, "Speed: %d km/h", speed);
    lv_label_set_text_fmt(temp_label, "Temp: %d°C", temp);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
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

  // Создание задачи LVGL
  // lvgl_port_create_task();

  // Создание задачи для демонстрации LVGL
  // xTaskCreate(lvgl_demo_task, "lvgl_demo_task", 4096, NULL, 4, NULL);

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