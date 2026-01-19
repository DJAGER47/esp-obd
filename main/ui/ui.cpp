#include "ui.h"

#include <inttypes.h>
#include <stdio.h>

#include <cstring>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_chip_info.h"
#include "esp_lcd_panel_io_interface.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vehicle_params.h"

static const char *TAG = "ui_class";

extern VehicleParams vehicle_params;

UI::UI(gpio_num_t sclk_pin,
       gpio_num_t mosi_pin,
       gpio_num_t lcd_rst_pin,
       gpio_num_t lcd_dc_pin,
       gpio_num_t lcd_cs_pin,
       gpio_num_t bk_light_pin) :
    st7789_pin_num_sclk(sclk_pin),
    st7789_pin_num_mosi(mosi_pin),
    st7789_pin_num_lcd_rst(lcd_rst_pin),
    st7789_pin_num_lcd_dc(lcd_dc_pin),
    st7789_pin_num_lcd_cs(lcd_cs_pin),
    st7789_pin_num_bk_light(bk_light_pin),
    display(nullptr),
    panel_handle(nullptr),
    buf1(nullptr),
    buf2(nullptr),
    current_screen(nullptr) {}

void UI::Init() {
  ESP_LOGI(TAG, "Initializing UI");

  if (init_st7789() != ESP_OK) {
    ESP_LOGI(TAG, "init_st7789 initialization failed. Restarting in 5 seconds...");
    esp_rom_delay_us(5000000);
    esp_restart();
  }

  init_lvgl();
  create_ui0();
  create_ui1();

  switch_screen(0);

  xTaskCreate(lvgl_task, "lvgl_task", 8192, this, 5, NULL);
  xTaskCreate(update_screen, "update_time", 8192, this, 4, NULL);
}

void UI::switch_screen(int num_screen) {
  if (num_screen == 0 && current_screen != screen0_elements.screen) {
    // Переключение на первый экран
    lv_screen_load(screen0_elements.screen);
    current_screen = screen0_elements.screen;
    // update_screen0(0.0, 0, 0);
    ESP_LOGI(TAG, "Switched to screen 1 (time display)");
  } else if (num_screen == 1 && current_screen != screen1_elements.screen) {
    // Переключение на второй экран
    lv_screen_load(screen1_elements.screen);
    current_screen = screen1_elements.screen;
    update_screen1();
    ESP_LOGI(TAG, "Switched to screen 2 (chip info)");
  }
}

void UI::update_screen0(float rpm, int speed, int coolant_temp) {
  if (screen0_elements.rpm_label != NULL) {
    char rpm_str[32];
    snprintf(rpm_str, sizeof(rpm_str), "RPM: %.1f", rpm);
    lv_label_set_text(screen0_elements.rpm_label, rpm_str);
  }

  if (screen0_elements.speed_label != NULL) {
    char speed_str[32];
    snprintf(speed_str, sizeof(speed_str), "Speed: %d km/h", speed);
    lv_label_set_text(screen0_elements.speed_label, speed_str);
  }

  if (screen0_elements.coolant_temp_label != NULL) {
    char coolant_str[32];
    snprintf(coolant_str, sizeof(coolant_str), "Coolant: %d°C", coolant_temp);
    lv_label_set_text(screen0_elements.coolant_temp_label, coolant_str);
  }
}

void UI::update_screen1() {
  if (screen1_elements.heap_label != NULL) {
    char heap_str[64];
    snprintf(heap_str, sizeof(heap_str), "Free heap: %" PRIu32 " bytes", esp_get_minimum_free_heap_size());
    lv_label_set_text(screen1_elements.heap_label, heap_str);
  }
}

esp_err_t UI::init_st7789() {
  ESP_LOGI(TAG, "Initializing ST7789 LCD display");

  if (st7789_pin_num_bk_light != GPIO_NUM_NC) {
    // Инициализация GPIO для подсветки
    gpio_config_t bk_gpio_config;
    memset(&bk_gpio_config, 0, sizeof(bk_gpio_config));
    bk_gpio_config.pin_bit_mask = 1ULL << st7789_pin_num_bk_light;
    bk_gpio_config.mode         = GPIO_MODE_OUTPUT;
    bk_gpio_config.pull_up_en   = GPIO_PULLUP_DISABLE;
    bk_gpio_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    bk_gpio_config.intr_type    = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    // Включаем подсветку
    gpio_set_level(st7789_pin_num_bk_light, 1);
  }

  // Конфигурация SPI шины
  spi_bus_config_t bus_config;
  memset(&bus_config, 0, sizeof(bus_config));
  bus_config.sclk_io_num     = st7789_pin_num_sclk;
  bus_config.mosi_io_num     = st7789_pin_num_mosi;
  bus_config.miso_io_num     = -1;  // MISO не используется
  bus_config.quadwp_io_num   = -1;
  bus_config.quadhd_io_num   = -1;
  bus_config.max_transfer_sz = ST7789_LCD_H_RES * ST7789_LCD_V_RES * sizeof(uint16_t);

  // Инициализация SPI шины
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO));

  // Конфигурация интерфейса панели
  esp_lcd_panel_io_spi_config_t io_config;
  memset(&io_config, 0, sizeof(io_config));
  io_config.dc_gpio_num       = st7789_pin_num_lcd_dc;
  io_config.cs_gpio_num       = st7789_pin_num_lcd_cs;
  io_config.pclk_hz           = 80000000;
  io_config.lcd_cmd_bits      = 8;
  io_config.lcd_param_bits    = 8;
  io_config.spi_mode          = 0;
  io_config.trans_queue_depth = 10;

  // Создание интерфейса панели
  esp_lcd_panel_io_handle_t io_handle = NULL;
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));

  // Конфигурация панели ST7789
  esp_lcd_panel_dev_config_t panel_config;
  memset(&panel_config, 0, sizeof(panel_config));
  panel_config.reset_gpio_num = st7789_pin_num_lcd_rst;
  panel_config.rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB;
  panel_config.data_endian    = LCD_RGB_DATA_ENDIAN_LITTLE;
  panel_config.bits_per_pixel = 16;

  // Создание панели ST7789
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

  // Инициализация и сброс панели
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

  // Установка ориентации дисплея
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
  // ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 35));

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
  buf1 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf1);
  buf2 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf2);

  // Создание дисплея LVGL
  display = lv_display_create(ST7789_LCD_H_RES, ST7789_LCD_V_RES);

  // Устанавливаем указатель на экземпляр класса как пользовательские данные
  lv_display_set_user_data(display, this);

  // Настройка буферов отрисовки
  ESP_LOGI(TAG, "Setting up LVGL display buffers: size=%d", DISP_BUF_SIZE);
  lv_display_set_buffers(display, buf1, buf2, DISP_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Установка функции обратного вызова для отправки данных на дисплей
  lv_display_set_flush_cb(display, lvgl_flush_cb);
}

void UI::create_ui0() {
  ESP_LOGI(TAG, "Creating UI elements");

  // Создание первого экрана
  screen0_elements.screen = lv_obj_create(NULL);

  // Создание меток для отображения данных OBD2
  screen0_elements.rpm_label = lv_label_create(screen0_elements.screen);
  lv_label_set_text(screen0_elements.rpm_label, "RPM: --");
  lv_obj_align(screen0_elements.rpm_label, LV_ALIGN_TOP_MID, 0, 20);

  screen0_elements.speed_label = lv_label_create(screen0_elements.screen);
  lv_label_set_text(screen0_elements.speed_label, "Speed: -- km/h");
  lv_obj_align(screen0_elements.speed_label, LV_ALIGN_TOP_MID, 0, 60);

  screen0_elements.coolant_temp_label = lv_label_create(screen0_elements.screen);
  lv_label_set_text(screen0_elements.coolant_temp_label, "Coolant: --°C");
  lv_obj_align(screen0_elements.coolant_temp_label, LV_ALIGN_TOP_MID, 0, 100);
}

void UI::create_ui1() {
  ESP_LOGI(TAG, "Creating second UI screen with chip info");

  // Создание второго экрана
  screen1_elements.screen = lv_obj_create(NULL);

  // Создание фона
  screen1_elements.bg = lv_obj_create(screen1_elements.screen);
  lv_obj_set_size(screen1_elements.bg, ST7789_LCD_H_RES, ST7789_LCD_V_RES);
  lv_obj_set_pos(screen1_elements.bg, 0, 0);
  lv_obj_set_style_bg_color(screen1_elements.bg, lv_color_make(0, 0, 0), LV_PART_MAIN);
  lv_obj_set_style_border_width(screen1_elements.bg, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(screen1_elements.bg, 0, LV_PART_MAIN);

  // Информация о свободной памяти
  char heap_str[64];
  snprintf(heap_str, sizeof(heap_str), "Free heap: %" PRIu32 " bytes", esp_get_minimum_free_heap_size());

  screen1_elements.heap_label = lv_label_create(screen1_elements.screen);
  lv_obj_set_style_text_font(screen1_elements.heap_label, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_set_style_text_color(screen1_elements.heap_label, lv_color_make(255, 255, 0), LV_PART_MAIN);
  lv_label_set_text(screen1_elements.heap_label, heap_str);
  lv_obj_align(screen1_elements.heap_label, LV_ALIGN_BOTTOM_MID, 0, -30);
}

void UI::lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  // Получаем указатель на экземпляр из пользовательских данных дисплея
  UI *ui_instance = (UI *)lv_display_get_user_data(disp);
  if (ui_instance && ui_instance->panel_handle) {
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // Отправка данных на дисплей
    esp_err_t err = esp_lcd_panel_draw_bitmap(
        ui_instance->panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, (void *)px_map);
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
      if (ui_instance->current_screen == ui_instance->screen0_elements.screen) {
        // Получаем параметры из vehicle_params и обновляем экран
        auto params = vehicle_params.getBasicParams();
        ui_instance->update_screen0(params.rpm, params.speed, params.coolant_temp);
      }

      if (ui_instance->current_screen == ui_instance->screen1_elements.screen) {
        ui_instance->update_screen1();
      }

      vTaskDelay(pdMS_TO_TICKS(100));
    }
  } else {
    while (1) {
      ESP_LOGE(TAG, "Error: update_screen");
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
