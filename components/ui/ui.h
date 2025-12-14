#pragma once

#include <stdint.h>

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "lvgl.h"
#include "phy_interface.h"

class UI final {
 public:
  UI(gpio_num_t sclk_pin     = GPIO_NUM_5,
     gpio_num_t mosi_pin     = GPIO_NUM_6,
     gpio_num_t lcd_rst_pin  = GPIO_NUM_7,
     gpio_num_t lcd_dc_pin   = GPIO_NUM_8,
     gpio_num_t lcd_cs_pin   = GPIO_NUM_9,
     gpio_num_t bk_light_pin = GPIO_NUM_10);

  // Инициализация
  esp_err_t init();

  // Управление экранами
  void switch_screen(int num_screen);
  void update_screen0(float rpm, int speed, int coolant_temp);
  void update_screen1();

  // Добавление CAN сообщения в очередь для отображения
  void addCanMessageToQueue(const TwaiFrame &frame);

  // Обработка сообщений из очереди и обновление экрана
  void processCanMessages();

 private:
  struct Screen0Elements {
    lv_obj_t *screen{nullptr};
    lv_obj_t *rpm_label{nullptr};
    lv_obj_t *speed_label{nullptr};
    lv_obj_t *coolant_temp_label{nullptr};
  };

  struct Screen1Elements {
    lv_obj_t *screen{nullptr};
    lv_obj_t *bg{nullptr};
    lv_obj_t *heap_label{nullptr};
  };

  const uint32_t size_can_labels       = 10;
  static const uint32_t can_queue_size = 20;  // Размер очереди для CAN сообщений

  // GPIO пины для дисплея ST7789
  const gpio_num_t st7789_pin_num_sclk;
  const gpio_num_t st7789_pin_num_mosi;
  const gpio_num_t st7789_pin_num_lcd_rst;
  const gpio_num_t st7789_pin_num_lcd_dc;
  const gpio_num_t st7789_pin_num_lcd_cs;
  const gpio_num_t st7789_pin_num_bk_light;

  // Разрешение дисплея
  static constexpr uint32_t ST7789_LCD_H_RES = 320;
  // static constexpr uint32_t ST7789_LCD_V_RES = 170;
  static constexpr uint32_t ST7789_LCD_V_RES = 240;

  // Размер буфера LVGL
  static constexpr uint32_t DISP_BUF_SIZE = (ST7789_LCD_H_RES * 10);

  // Дескрипторы для LCD и LVGL
  lv_display_t *display;
  esp_lcd_panel_handle_t panel_handle;
  lv_color_t *buf1;
  lv_color_t *buf2;

  // Экраны
  Screen0Elements screen0_elements;
  Screen1Elements screen1_elements;
  lv_obj_t *current_screen;

  // Приватные методы инициализации
  esp_err_t init_st7789();
  void init_lvgl();
  void create_ui0();
  void create_ui1();

  // Функция обратного вызова для отправки данных на дисплей
  static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

  // Статические задачи
  static void update_screen(void *arg);
  static void lvgl_task(void *arg);
};
