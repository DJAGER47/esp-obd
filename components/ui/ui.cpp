#include "ui.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_lcd_panel_io_interface.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>
#include <inttypes.h>
#include <stdio.h>

static const char *TAG = "ui_class";

UI::UI()
    : display(nullptr), panel_handle(nullptr), buf1(nullptr), buf2(nullptr),
      screen1(nullptr), screen2(nullptr), current_screen(nullptr),
      time_label(nullptr), heap_label(nullptr), spi_speed_label(nullptr),
      start_time(0), current_gpio_state(0), freq_khz(0), lvgl_timer(nullptr) {}

esp_err_t UI::init() {
  ESP_LOGI(TAG, "Initializing UI");

  esp_err_t ret = init_st7789();
  if (ret != ESP_OK) {
    return ret;
  }

  init_lvgl();
  create_ui();
  create_ui_screen2();

  switch_screen(0);

  xTaskCreate(lvgl_task, "lvgl_task", 4096, this, 5, NULL);
  xTaskCreate(update_screen, "update_time", 4096, this, 4, NULL);
  return ESP_OK;
}

void UI::switch_screen(int gpio_state) {
  if (gpio_state == 0 && current_screen != screen1) {
    // Переключение на первый экран
    lv_screen_load(screen1);
    current_screen = screen1;
    ESP_LOGI(TAG, "Switched to screen 1 (time display)");
  } else if (gpio_state == 1 && current_screen != screen2) {
    // Переключение на второй экран
    lv_screen_load(screen2);
    current_screen = screen2;
    ESP_LOGI(TAG, "Switched to screen 2 (chip info)");
  }
}

void UI::update_screen1() {
  char time_str[15];

  // Вычисляем прошедшее время
  uint32_t elapsed_time =
      (xTaskGetTickCount() * portTICK_PERIOD_MS) / 1000 - start_time;
  uint32_t hours = elapsed_time / 3600;
  uint32_t minutes = (elapsed_time % 3600) / 60;
  uint32_t seconds = elapsed_time % 60;

  // Форматируем строку времени
  snprintf(time_str, sizeof(time_str), "%02" PRIu32 ":%02" PRIu32 ":%02" PRIu32,
           hours, minutes, seconds);

  // Обновляем текст метки
  if (time_label != NULL) {
    lv_label_set_text(time_label, time_str);
  }
}

void UI::update_screen2() {
  if (heap_label != NULL) {
    char heap_str[64];
    snprintf(heap_str, sizeof(heap_str), "Free heap: %" PRIu32 " bytes",
             esp_get_minimum_free_heap_size());
    lv_label_set_text(heap_label, heap_str);
  }
}

esp_err_t UI::init_st7789() {
  ESP_LOGI(TAG, "Initializing ST7789 LCD display");

  // Инициализация GPIO для подсветки
  gpio_config_t bk_gpio_config;
  memset(&bk_gpio_config, 0, sizeof(bk_gpio_config));
  bk_gpio_config.pin_bit_mask = 1ULL << ST7789_PIN_NUM_BK_LIGHT;
  bk_gpio_config.mode = GPIO_MODE_OUTPUT;
  bk_gpio_config.pull_up_en = GPIO_PULLUP_DISABLE;
  bk_gpio_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  bk_gpio_config.intr_type = GPIO_INTR_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

  // Включаем подсветку
  gpio_set_level(ST7789_PIN_NUM_BK_LIGHT, 1);

  // Конфигурация SPI шины
  spi_bus_config_t bus_config;
  memset(&bus_config, 0, sizeof(bus_config));
  bus_config.sclk_io_num = ST7789_PIN_NUM_SCLK;
  bus_config.mosi_io_num = ST7789_PIN_NUM_MOSI;
  bus_config.miso_io_num = -1; // MISO не используется
  bus_config.quadwp_io_num = -1;
  bus_config.quadhd_io_num = -1;
  bus_config.max_transfer_sz =
      ST7789_LCD_H_RES * ST7789_LCD_V_RES * sizeof(uint16_t);

  // Инициализация SPI шины
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO));

  // Конфигурация интерфейса панели
  esp_lcd_panel_io_spi_config_t io_config;
  memset(&io_config, 0, sizeof(io_config));
  io_config.dc_gpio_num = ST7789_PIN_NUM_LCD_DC;
  io_config.cs_gpio_num = ST7789_PIN_NUM_LCD_CS;
  io_config.pclk_hz = 80000000;
  io_config.lcd_cmd_bits = 8;
  io_config.lcd_param_bits = 8;
  io_config.spi_mode = 0;
  io_config.trans_queue_depth = 10;

  // Создание интерфейса панели
  esp_lcd_panel_io_handle_t io_handle = NULL;
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST,
                                           &io_config, &io_handle));
  // Получение spi_device_handle_t из io_handle
  // В структуре esp_lcd_panel_io_spi_t поле spi_dev находится сразу после поля
  // base
  spi_device_handle_t spi_dev = *(
      spi_device_handle_t *)((uint8_t *)io_handle + sizeof(esp_lcd_panel_io_t));
  esp_err_t ret = spi_device_get_actual_freq(spi_dev, &freq_khz);
  if (ret == ESP_OK) {
    freq_khz *= 1000; // Преобразуем кГц в Гц
  } else {
    ESP_LOGE(TAG, "Failed to get SPI2 actual freq: %s", esp_err_to_name(ret));
  }

  // Конфигурация панели ST7789
  esp_lcd_panel_dev_config_t panel_config;
  memset(&panel_config, 0, sizeof(panel_config));
  panel_config.reset_gpio_num = ST7789_PIN_NUM_LCD_RST;
  panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
  panel_config.data_endian = LCD_RGB_DATA_ENDIAN_LITTLE;
  panel_config.bits_per_pixel = 16;

  // Создание панели ST7789
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

  // Инициализация и сброс панели
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

  // Установка ориентации дисплея
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
  ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 35));

  // Исправление ориентации дисплея - меняем местами координаты X и Y
  ESP_LOGI(TAG, "Setting display orientation");
  ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
  ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));

  // Явно включаем дисплей
  ESP_LOGI(TAG, "Turning on the display");
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

  return ESP_OK;
}

void UI::init_lvgl() {
  ESP_LOGI(TAG, "Initializing LVGL");

  // Инициализация LVGL
  lv_init();

  // Выделение памяти для буферов LVGL
  buf1 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t),
                                        MALLOC_CAP_DMA);
  assert(buf1);
  buf2 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t),
                                        MALLOC_CAP_DMA);
  assert(buf2);

  // Создание дисплея LVGL
  display = lv_display_create(ST7789_LCD_H_RES, ST7789_LCD_V_RES);

  // Устанавливаем указатель на экземпляр класса как пользовательские данные
  lv_display_set_user_data(display, this);

  // Настройка буферов отрисовки
  ESP_LOGI(TAG, "Setting up LVGL display buffers: size=%d", DISP_BUF_SIZE);
  lv_display_set_buffers(display, buf1, buf2, DISP_BUF_SIZE,
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Установка функции обратного вызова для отправки данных на дисплей
  lv_display_set_flush_cb(display, lvgl_flush_cb);
}

void UI::create_ui() {
  ESP_LOGI(TAG, "Creating UI elements");

  // Создание первого экрана
  screen1 = lv_obj_create(NULL);

  // Создание красной рамки
  lv_obj_t *border1 = lv_obj_create(screen1);
  lv_obj_set_size(border1, ST7789_LCD_H_RES, ST7789_LCD_V_RES);
  lv_obj_set_pos(border1, 0, 0);
  lv_obj_set_style_border_width(border1, 5, LV_PART_MAIN);
  lv_obj_set_style_border_color(border1, lv_color_make(255, 0, 0),
                                LV_PART_MAIN);
  lv_obj_set_style_bg_opa(border1, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_pad_all(border1, 0, LV_PART_MAIN);

  lv_obj_t *border2 = lv_obj_create(screen1);
  lv_obj_set_size(border2, ST7789_LCD_H_RES - 20, ST7789_LCD_V_RES - 20);
  lv_obj_align(border2, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_border_width(border2, 5, LV_PART_MAIN);
  lv_obj_set_style_border_color(border2, lv_color_make(0, 255, 0),
                                LV_PART_MAIN);
  lv_obj_set_style_bg_opa(border2, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_pad_all(border2, 0, LV_PART_MAIN);

  lv_obj_t *border3 = lv_obj_create(screen1);
  lv_obj_set_size(border3, ST7789_LCD_H_RES - 40, ST7789_LCD_V_RES - 40);
  lv_obj_align(border3, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_border_width(border3, 5, LV_PART_MAIN);
  lv_obj_set_style_border_color(border3, lv_color_make(0, 0, 255),
                                LV_PART_MAIN);
  lv_obj_set_style_bg_opa(border3, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_pad_all(border3, 0, LV_PART_MAIN);

  // Создание контейнера для метки времени с фоном
  lv_obj_t *time_container = lv_obj_create(screen1);
  lv_obj_set_size(time_container, 150, 50);
  lv_obj_align(time_container, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(time_container, lv_color_make(255, 255, 255),
                            LV_PART_MAIN); // Белый фон
  lv_obj_set_style_bg_opa(time_container, LV_OPA_70,
                          LV_PART_MAIN); // Полупрозрачный
  lv_obj_set_style_border_width(time_container, 2, LV_PART_MAIN);
  lv_obj_set_style_border_color(time_container, lv_color_make(0, 0, 0),
                                LV_PART_MAIN); // Черная рамка
  lv_obj_set_style_radius(time_container, 5, LV_PART_MAIN); // Скругленные углы

  // Создание метки для отображения времени
  time_label = lv_label_create(time_container);
  lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(time_label, &lv_font_montserrat_24,
                             LV_PART_MAIN); // Увеличен размер шрифта
  lv_obj_set_style_text_color(time_label, lv_color_make(0, 0, 0),
                              LV_PART_MAIN); // Черный текст
  lv_label_set_text(time_label, "00:00:00");

  // Сохраняем начальное время
  start_time = (xTaskGetTickCount() * portTICK_PERIOD_MS) / 1000;
}

void UI::create_ui_screen2() {
  ESP_LOGI(TAG, "Creating second UI screen with chip info");

  // Получение информации о чипе
  esp_chip_info_t chip_info;
  uint32_t flash_size;
  esp_chip_info(&chip_info);
  esp_flash_get_size(NULL, &flash_size);

  // Создание второго экрана
  screen2 = lv_obj_create(NULL);

  // Создание фона
  lv_obj_t *bg = lv_obj_create(screen2);
  lv_obj_set_size(bg, ST7789_LCD_H_RES, ST7789_LCD_V_RES);
  lv_obj_set_pos(bg, 0, 0);
  lv_obj_set_style_bg_color(bg, lv_color_make(0, 0, 50),
                            LV_PART_MAIN); // Темно-синий фон
  lv_obj_set_style_border_width(bg, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(bg, 0, LV_PART_MAIN);

  // Заголовок
  lv_obj_t *title = lv_label_create(screen2);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_set_style_text_color(title, lv_color_make(255, 255, 255),
                              LV_PART_MAIN);
  lv_label_set_text(title, "CHIP INFO");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  // Информация о чипе
  char chip_str[256];
  snprintf(chip_str, sizeof(chip_str),
           "Chip: %s\nCores: %d\nRev: v%d.%d\nFlash: %" PRIu32
           "MB\nFeatures: %s%s%s%s",
           CONFIG_IDF_TARGET, chip_info.cores, chip_info.revision / 100,
           chip_info.revision % 100, flash_size / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi " : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT " : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE " : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? "802.15.4" : "");

  lv_obj_t *info_label = lv_label_create(screen2);
  lv_obj_set_style_text_font(info_label, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_text_color(info_label, lv_color_make(255, 255, 255),
                              LV_PART_MAIN);
  lv_label_set_text(info_label, chip_str);
  lv_obj_align(info_label, LV_ALIGN_CENTER, 0, 0);

  // Информация о свободной памяти
  char heap_str[64];
  snprintf(heap_str, sizeof(heap_str), "Free heap: %" PRIu32 " bytes",
           esp_get_minimum_free_heap_size());

  heap_label = lv_label_create(screen2);
  lv_obj_set_style_text_font(heap_label, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_set_style_text_color(heap_label, lv_color_make(255, 255, 0),
                              LV_PART_MAIN);
  lv_label_set_text(heap_label, heap_str);
  lv_obj_align(heap_label, LV_ALIGN_BOTTOM_MID, 0, -30);

  // Информация о скорости SPI2_HOST
  char spi_str[64];
  uint32_t spi_speed = (uint32_t)freq_khz;
  snprintf(spi_str, sizeof(spi_str), "SPI2 Speed: %" PRIu32 " Hz", spi_speed);

  spi_speed_label = lv_label_create(screen2);
  lv_obj_set_style_text_font(spi_speed_label, &lv_font_montserrat_12,
                             LV_PART_MAIN);
  lv_obj_set_style_text_color(spi_speed_label, lv_color_make(0, 255, 255),
                              LV_PART_MAIN);
  lv_label_set_text(spi_speed_label, spi_str);
  lv_obj_align(spi_speed_label, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void UI::lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area,
                       uint8_t *px_map) {
  // Получаем указатель на экземпляр из пользовательских данных дисплея
  UI *ui_instance = (UI *)lv_display_get_user_data(disp);
  if (ui_instance && ui_instance->panel_handle) {
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // Отправка данных на дисплей
    esp_err_t err =
        esp_lcd_panel_draw_bitmap(ui_instance->panel_handle, offsetx1, offsety1,
                                  offsetx2 + 1, offsety2 + 1, (void *)px_map);
    if (err != ESP_OK) {
      ESP_LOGI(TAG, "Failed to draw bitmap: %s", esp_err_to_name(err));
    }
  }
  lv_display_flush_ready(disp);
}

void UI::update_screen(void *arg) {
  UI *ui_instance = (UI *)arg;
  if (ui_instance) {
    while (1) {
      if (ui_instance->current_screen == ui_instance->screen1) {
        ui_instance->update_screen1();
      }

      if (ui_instance->current_screen == ui_instance->screen2) {
        ui_instance->update_screen2();
      }
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  } else {
    while (1) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

void UI::lvgl_task(void *arg) {
  while (1) {
    // Вызываем обработчик таймеров LVGL
    lv_timer_handler();
    // Увеличиваем счетчик тиков LVGL
    lv_tick_inc(10);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
