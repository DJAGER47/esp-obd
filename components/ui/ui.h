#ifndef UI_H
#define UI_H

#include <stdint.h>

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Пины для дисплея ST7789
#define ST7789_PIN_NUM_SCLK     GPIO_NUM_5
#define ST7789_PIN_NUM_MOSI     GPIO_NUM_6
#define ST7789_PIN_NUM_LCD_RST  GPIO_NUM_7
#define ST7789_PIN_NUM_LCD_DC   GPIO_NUM_8
#define ST7789_PIN_NUM_LCD_CS   GPIO_NUM_9
#define ST7789_PIN_NUM_BK_LIGHT GPIO_NUM_10

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class UI final {
 public:
  UI();

  // Инициализация
  esp_err_t init();

  // Управление экранами
  void switch_screen(int gpio_state);

  // Обновление данных
  void update_screen1();
  void update_screen2();

 private:
  // Разрешение дисплея
  static constexpr uint32_t ST7789_LCD_H_RES = 320;
  static constexpr uint32_t ST7789_LCD_V_RES = 170;
  // Размер буфера LVGL
  static constexpr uint32_t DISP_BUF_SIZE = (ST7789_LCD_H_RES * 10);

  // Дескрипторы для LCD и LVGL
  lv_display_t *display;
  esp_lcd_panel_handle_t panel_handle;
  lv_color_t *buf1;
  lv_color_t *buf2;

  // Экраны
  lv_obj_t *screen1;
  lv_obj_t *screen2;
  lv_obj_t *current_screen;

  // Элементы UI
  lv_obj_t *time_label;
  lv_obj_t *heap_label;
  lv_obj_t *spi_speed_label;

  // Переменные для времени
  uint32_t start_time;

  // Переменные для управления экранами
  int current_gpio_state;

  // Переменная для скорости SPI
  int freq_khz;

  // FreeRTOS таймер для LVGL
  TimerHandle_t lvgl_timer;

  // Приватные методы инициализации
  esp_err_t init_st7789();
  void init_lvgl();
  void create_ui();
  void create_ui_screen2();

  // Функция обратного вызова для отправки данных на дисплей
  static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

  // Статические задачи
  static void update_screen(void *arg);
  static void lvgl_task(void *arg);
};

#endif  // __cplusplus

#endif  // UI_H