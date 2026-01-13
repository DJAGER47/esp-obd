#include "ld7138.h"

#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "ld7138";

// Структура для хранения данных панели LD7138
typedef struct {
  esp_lcd_panel_io_handle_t io;
  gpio_num_t reset_gpio_num;
  uint16_t x_res;
  uint16_t y_res;
  uint8_t bits_per_pixel;
  uint8_t madctl_val;
  bool reset_level;
} ld7138_panel_t;

// Структура-обёртка для панели
typedef struct {
  void *base;                   // Базовая структура панели (указатель на void)
  ld7138_panel_t *ld7138_data;  // Указатель на наши данные
} ld7138_panel_wrapper_t;

// Вспомогательные функции
static esp_err_t ld7138_write_cmd(esp_lcd_panel_io_handle_t io, uint8_t cmd);
static esp_err_t ld7138_write_data(esp_lcd_panel_io_handle_t io, const uint8_t *data, size_t len);
static esp_err_t ld7138_init_panel(esp_lcd_panel_handle_t panel);

// Реализация функций панели
static esp_err_t ld7138_del(esp_lcd_panel_handle_t panel);
static esp_err_t ld7138_reset(esp_lcd_panel_handle_t panel);
static esp_err_t ld7138_init(esp_lcd_panel_handle_t panel);
static esp_err_t ld7138_draw_bitmap(
    esp_lcd_panel_handle_t panel, int x1, int y1, int x2, int y2, const void *color_data);
static esp_err_t ld7138_invert_color(esp_lcd_panel_handle_t panel, bool invert_color);
static esp_err_t ld7138_mirror(esp_lcd_panel_handle_t panel, bool mirror_x, bool mirror_y);
static esp_err_t ld7138_swap_xy(esp_lcd_panel_handle_t panel, bool swap_axes);
static esp_err_t ld7138_set_gap(esp_lcd_panel_handle_t panel, int x_gap, int y_gap);
static esp_err_t ld7138_disp_on_off(esp_lcd_panel_handle_t panel, bool on_off);

esp_err_t esp_lcd_new_panel_ld7138(const esp_lcd_panel_io_handle_t io_handle,
                                   const esp_lcd_panel_dev_config_t *panel_config,
                                   esp_lcd_panel_handle_t *panel_handle) {
  ESP_LOGI(TAG, "Initializing LD7138 panel");

  if (panel_handle == NULL) {
    ESP_LOGE(TAG, "panel_handle cannot be NULL");
    return ESP_ERR_INVALID_ARG;
  }

  // Выделение памяти для структуры панели
  ld7138_panel_t *ld7138_panel = (ld7138_panel_t *)calloc(1, sizeof(ld7138_panel_t));
  if (ld7138_panel == NULL) {
    ESP_LOGE(TAG, "No memory for LD7138 panel");
    return ESP_ERR_NO_MEM;
  }

  // Сохранение параметров
  ld7138_panel->io             = io_handle;
  ld7138_panel->reset_gpio_num = (gpio_num_t)panel_config->reset_gpio_num;
  ld7138_panel->bits_per_pixel = panel_config->bits_per_pixel;
  ld7138_panel->reset_level    = panel_config->flags.reset_active_high;

  // Установка разрешений по умолчанию
  ld7138_panel->x_res = 320;
  ld7138_panel->y_res = 240;

  // Настройка MADCTL
  ld7138_panel->madctl_val = 0;

  // В ESP-IDF v5 структура esp_lcd_panel_t является непрозрачной,
  // но мы можем создать структуру-обёртку, которая содержит её.

  // Выделяем память для нашей структуры-обёртки
  ld7138_panel_wrapper_t *wrapper = (ld7138_panel_wrapper_t *)calloc(1, sizeof(ld7138_panel_wrapper_t));
  if (wrapper == NULL) {
    ESP_LOGE(TAG, "No memory for LD7138 panel wrapper");
    free(ld7138_panel);
    return ESP_ERR_NO_MEM;
  }

  // Сохраняем указатель на наши данные
  wrapper->ld7138_data = ld7138_panel;

  // В ESP-IDF v5, esp_lcd_panel_handle_t - это указатель на void,
  // поэтому мы можем использовать указатель на нашу структуру-обёртку
  *panel_handle = (esp_lcd_panel_handle_t)wrapper;
  ESP_LOGI(TAG, "LD7138 panel created successfully");

  return ESP_OK;
}

static esp_err_t ld7138_del(esp_lcd_panel_handle_t panel) {
  if (panel) {
    // Получаем указатель на нашу структуру-обёртку
    ld7138_panel_wrapper_t *wrapper = (ld7138_panel_wrapper_t *)panel;

    // Освобождаем память
    if (wrapper->ld7138_data) {
      free(wrapper->ld7138_data);
    }
    free(wrapper);
  }
  return ESP_OK;
}

static esp_err_t ld7138_reset(esp_lcd_panel_handle_t panel) {
  // Получаем указатель на нашу структуру-обёртку
  ld7138_panel_wrapper_t *wrapper = (ld7138_panel_wrapper_t *)panel;
  ld7138_panel_t *ld7138_panel    = wrapper->ld7138_data;

  if (ld7138_panel->reset_gpio_num != GPIO_NUM_NC) {
    gpio_reset_pin(ld7138_panel->reset_gpio_num);
    gpio_set_direction(ld7138_panel->reset_gpio_num, GPIO_MODE_OUTPUT);
    gpio_set_level(ld7138_panel->reset_gpio_num, ld7138_panel->reset_level ? 0 : 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(ld7138_panel->reset_gpio_num, ld7138_panel->reset_level ? 1 : 0);
    vTaskDelay(pdMS_TO_TICKS(120));
  }

  return ESP_OK;
}

static esp_err_t ld7138_init(esp_lcd_panel_handle_t panel) {
  ESP_LOGI(TAG, "Initializing LD7138 panel");
  return ld7138_init_panel(panel);
}

static esp_err_t ld7138_init_panel(esp_lcd_panel_handle_t panel) {
  // Получаем указатель на нашу структуру-обёртку
  ld7138_panel_wrapper_t *wrapper = (ld7138_panel_wrapper_t *)panel;
  ld7138_panel_t *ld7138_panel    = wrapper->ld7138_data;
  esp_lcd_panel_io_handle_t io    = ld7138_panel->io;

  // Последовательность инициализации LD7138
  ld7138_write_cmd(io, LD7138_CMD_SWRESET);
  vTaskDelay(pdMS_TO_TICKS(120));

  ld7138_write_cmd(io, LD7138_CMD_SLPOUT);
  vTaskDelay(pdMS_TO_TICKS(120));

  // Установка формата пикселей
  ld7138_write_cmd(io, LD7138_CMD_COLMOD);
  uint8_t colmod = LD7138_COLMOD_16BIT;
  ld7138_write_data(io, &colmod, 1);

  // Установка ориентации
  ld7138_write_cmd(io, LD7138_CMD_MADCTL);
  ld7138_write_data(io, &ld7138_panel->madctl_val, 1);

  ld7138_write_cmd(io, LD7138_CMD_NORON);
  vTaskDelay(pdMS_TO_TICKS(10));

  ld7138_write_cmd(io, LD7138_CMD_DISPON);
  vTaskDelay(pdMS_TO_TICKS(120));

  ESP_LOGI(TAG, "LD7138 panel initialized successfully");
  return ESP_OK;
}

static esp_err_t ld7138_draw_bitmap(
    esp_lcd_panel_handle_t panel, int x1, int y1, int x2, int y2, const void *color_data) {
  // Получаем указатель на нашу структуру-обёртку
  ld7138_panel_wrapper_t *wrapper = (ld7138_panel_wrapper_t *)panel;
  ld7138_panel_t *ld7138_panel    = wrapper->ld7138_data;
  esp_lcd_panel_io_handle_t io    = ld7138_panel->io;

  // Установка области для отрисовки
  ld7138_write_cmd(io, LD7138_CMD_CASET);
  uint8_t caset_data[4];
  caset_data[0] = (uint8_t)(x1 >> 8);
  caset_data[1] = (uint8_t)(x1 & 0xFF);
  caset_data[2] = (uint8_t)(x2 >> 8);
  caset_data[3] = (uint8_t)(x2 & 0xFF);
  ld7138_write_data(io, caset_data, 4);

  ld7138_write_cmd(io, LD7138_CMD_RASET);
  uint8_t raset_data[4];
  raset_data[0] = (uint8_t)(y1 >> 8);
  raset_data[1] = (uint8_t)(y1 & 0xFF);
  raset_data[2] = (uint8_t)(y2 >> 8);
  raset_data[3] = (uint8_t)(y2 & 0xFF);
  ld7138_write_data(io, raset_data, 4);

  // Запись данных пикселей
  ld7138_write_cmd(io, LD7138_CMD_RAMWR);

  size_t len = (x2 - x1 + 1) * (y2 - y1 + 1) * ld7138_panel->bits_per_pixel / 8;
  return esp_lcd_panel_io_tx_param(io, LD7138_CMD_RAMWR, color_data, len);
}

static esp_err_t ld7138_invert_color(esp_lcd_panel_handle_t panel, bool invert_color) {
  // Получаем указатель на нашу структуру-обёртку
  ld7138_panel_wrapper_t *wrapper = (ld7138_panel_wrapper_t *)panel;
  ld7138_panel_t *ld7138_panel    = wrapper->ld7138_data;
  esp_lcd_panel_io_handle_t io    = ld7138_panel->io;

  uint8_t cmd = invert_color ? LD7138_CMD_INVON : LD7138_CMD_INVOFF;
  return ld7138_write_cmd(io, cmd);
}

static esp_err_t ld7138_mirror(esp_lcd_panel_handle_t panel, bool mirror_x, bool mirror_y) {
  // Получаем указатель на нашу структуру-обёртку
  ld7138_panel_wrapper_t *wrapper = (ld7138_panel_wrapper_t *)panel;
  ld7138_panel_t *ld7138_panel    = wrapper->ld7138_data;
  esp_lcd_panel_io_handle_t io    = ld7138_panel->io;

  // Обновление MADCTL
  if (mirror_x) {
    ld7138_panel->madctl_val |= LD7138_MADCTL_MX;
  } else {
    ld7138_panel->madctl_val &= ~LD7138_MADCTL_MX;
  }

  if (mirror_y) {
    ld7138_panel->madctl_val |= LD7138_MADCTL_MY;
  } else {
    ld7138_panel->madctl_val &= ~LD7138_MADCTL_MY;
  }

  ld7138_write_cmd(io, LD7138_CMD_MADCTL);
  return ld7138_write_data(io, &ld7138_panel->madctl_val, 1);
}

static esp_err_t ld7138_swap_xy(esp_lcd_panel_handle_t panel, bool swap_axes) {
  // Получаем указатель на нашу структуру-обёртку
  ld7138_panel_wrapper_t *wrapper = (ld7138_panel_wrapper_t *)panel;
  ld7138_panel_t *ld7138_panel    = wrapper->ld7138_data;
  esp_lcd_panel_io_handle_t io    = ld7138_panel->io;

  // Обновление MADCTL
  if (swap_axes) {
    ld7138_panel->madctl_val |= LD7138_MADCTL_MV;
  } else {
    ld7138_panel->madctl_val &= ~LD7138_MADCTL_MV;
  }

  ld7138_write_cmd(io, LD7138_CMD_MADCTL);
  return ld7138_write_data(io, &ld7138_panel->madctl_val, 1);
}

static esp_err_t ld7138_set_gap(esp_lcd_panel_handle_t panel, int x_gap, int y_gap) {
  // LD7138 не поддерживает установку зазоров
  return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t ld7138_disp_on_off(esp_lcd_panel_handle_t panel, bool on_off) {
  // Получаем указатель на нашу структуру-обёртку
  ld7138_panel_wrapper_t *wrapper = (ld7138_panel_wrapper_t *)panel;
  ld7138_panel_t *ld7138_panel    = wrapper->ld7138_data;
  esp_lcd_panel_io_handle_t io    = ld7138_panel->io;

  uint8_t cmd = on_off ? LD7138_CMD_DISPON : LD7138_CMD_DISPOFF;
  return ld7138_write_cmd(io, cmd);
}

// Вспомогательные функции для записи команд и данных
static esp_err_t ld7138_write_cmd(esp_lcd_panel_io_handle_t io, uint8_t cmd) {
  return esp_lcd_panel_io_tx_param(io, cmd, NULL, 0);
}

static esp_err_t ld7138_write_data(esp_lcd_panel_io_handle_t io, const uint8_t *data, size_t len) {
  return esp_lcd_panel_io_tx_color(io, 0, data, len);
}