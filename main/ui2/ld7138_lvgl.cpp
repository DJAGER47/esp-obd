#include "ld7138_lvgl.h"

#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "ld7138_lvgl";

// Структура для хранения данных дисплея
typedef struct {
  ld7138_handle_t ld7138_handle;
  uint8_t *buf1;
  uint8_t *buf2;
  bool flushing;
} ld7138_lvgl_disp_t;

// Функция обратного вызова для отрисовки на дисплее
static void ld7138_lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

esp_err_t ld7138_lvgl_init(const ld7138_config_t *config, lv_display_t **disp_handle, ld7138_handle_t *ld7138_handle) {
  if (config == NULL || disp_handle == NULL || ld7138_handle == NULL) {
    ESP_LOGE(TAG, "Invalid arguments");
    return ESP_ERR_INVALID_ARG;
  }

  // Инициализация дисплея LD7138
  esp_err_t ret = ld7138_init(config, ld7138_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize LD7138: %s", esp_err_to_name(ret));
    return ret;
  }

  // Выделение памяти для данных дисплея
  ld7138_lvgl_disp_t *disp_data = (ld7138_lvgl_disp_t *)calloc(1, sizeof(ld7138_lvgl_disp_t));
  if (disp_data == NULL) {
    ESP_LOGE(TAG, "No memory for display data");
    ld7138_free(*ld7138_handle);
    return ESP_ERR_NO_MEM;
  }

  // Сохранение дескриптора LD7138
  disp_data->ld7138_handle = *ld7138_handle;
  disp_data->flushing      = false;

  // Выделение памяти для буферов LVGL
  size_t buffer_size = LD7138_WIDTH * LD7138_HEIGHT * sizeof(uint16_t);
  disp_data->buf1    = (uint8_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
  if (disp_data->buf1 == NULL) {
    ESP_LOGE(TAG, "No memory for LVGL buffer 1");
    free(disp_data);
    ld7138_free(*ld7138_handle);
    return ESP_ERR_NO_MEM;
  }

  disp_data->buf2 = (uint8_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
  if (disp_data->buf2 == NULL) {
    ESP_LOGE(TAG, "No memory for LVGL buffer 2");
    heap_caps_free(disp_data->buf1);
    free(disp_data);
    ld7138_free(*ld7138_handle);
    return ESP_ERR_NO_MEM;
  }

  // Создание дисплея LVGL
  lv_display_t *disp = lv_display_create(LD7138_WIDTH, LD7138_HEIGHT);
  if (disp == NULL) {
    ESP_LOGE(TAG, "Failed to create LVGL display");
    heap_caps_free(disp_data->buf1);
    heap_caps_free(disp_data->buf2);
    free(disp_data);
    ld7138_free(*ld7138_handle);
    return ESP_ERR_NO_MEM;
  }

  // Установка пользовательских данных
  lv_display_set_user_data(disp, disp_data);

  // Настройка буферов
  lv_display_set_buffers(disp, disp_data->buf1, disp_data->buf2, buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Установка функции обратного вызова для отрисовки
  lv_display_set_flush_cb(disp, ld7138_lvgl_flush_cb);

  // Установка цветового формата (RGB565)
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);

  // Сохранение дескриптора дисплея
  *disp_handle = disp;

  ESP_LOGI(TAG, "LD7138 LVGL initialized successfully");
  return ESP_OK;
}

esp_err_t ld7138_lvgl_free(lv_display_t *disp_handle, ld7138_handle_t ld7138_handle) {
  if (disp_handle == NULL || ld7138_handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  // Получение данных дисплея
  ld7138_lvgl_disp_t *disp_data = (ld7138_lvgl_disp_t *)lv_display_get_user_data(disp_handle);
  if (disp_data != NULL) {
    // Освобождение буферов
    if (disp_data->buf1 != NULL) {
      heap_caps_free(disp_data->buf1);
    }
    if (disp_data->buf2 != NULL) {
      heap_caps_free(disp_data->buf2);
    }
    free(disp_data);
  }

  // Удаление дисплея LVGL
  lv_display_delete(disp_handle);

  // Деинициализация LD7138
  ld7138_free(ld7138_handle);

  ESP_LOGI(TAG, "LD7138 LVGL deinitialized");
  return ESP_OK;
}

static void ld7138_lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  // Получение данных дисплея
  ld7138_lvgl_disp_t *disp_data = (ld7138_lvgl_disp_t *)lv_display_get_user_data(disp);
  if (disp_data == NULL || disp_data->flushing) {
    lv_display_flush_ready(disp);
    return;
  }

  disp_data->flushing = true;

  // Проверка границ области
  int16_t x1 = area->x1;
  int16_t y1 = area->y1;
  int16_t x2 = area->x2;
  int16_t y2 = area->y2;

  if (x1 < 0)
    x1 = 0;
  if (y1 < 0)
    y1 = 0;
  if (x2 >= LD7138_WIDTH)
    x2 = LD7138_WIDTH - 1;
  if (y2 >= LD7138_HEIGHT)
    y2 = LD7138_HEIGHT - 1;

  // Установка окна для записи данных (команда LD7138_0x0C_DATA_WRITE_READ уже вызывается внутри)
  ld7138_set_window(disp_data->ld7138_handle, x1, y1, x2, y2);

  // Отправка данных пикселей (команда записи уже отправлена в ld7138_set_window)
  size_t len = (x2 - x1 + 1) * (y2 - y1 + 1) * sizeof(uint16_t);
  ld7138_write_data_buffer(disp_data->ld7138_handle, px_map, len);

  disp_data->flushing = false;
  lv_display_flush_ready(disp);
}