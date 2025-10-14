#include "lvgl_port.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static const char *TAG              = "LVGL_PORT";
static SSD1283A *display_instance   = nullptr;
static SemaphoreHandle_t lvgl_mutex = nullptr;

// Буфер для LVGL (используем двойную буферизацию для лучшей производительности)
static lv_color_t buf1[SSD1283A_WIDTH * 10];
static lv_color_t buf2[SSD1283A_WIDTH * 10];
static lv_display_t *disp;

// Функция обратного вызова для отрисовки
static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  if (!display_instance) {
    ESP_LOGE(TAG, "Display instance is null");
    lv_display_flush_ready(disp);
    return;
  }

  // Получаем указатель на буфер дисплея
  uint16_t *display_buf = display_instance->getBuffer();

  // Преобразуем указатель на данные в lv_color_t
  lv_color_t *color_map = (lv_color_t *)px_map;

  // Копируем данные из LVGL буфера в буфер дисплея
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      if (x >= 0 && x < SSD1283A_WIDTH && y >= 0 && y < SSD1283A_HEIGHT) {
        // LVGL использует 16-битный цвет (RGB565), наш дисплей тоже 16-битный
        lv_color_t lv_color =
            color_map[(y - area->y1) * (area->x2 - area->x1 + 1) + (x - area->x1)];

        // Преобразуем LVGL цвет в RGB565
        uint16_t pixel_color = lv_color_to_u16(lv_color);

        // Устанавливаем пиксель в буфере дисплея
        uint16_t index     = y * SSD1283A_WIDTH + x;
        display_buf[index] = pixel_color;
      }
    }
  }

  // Обновляем дисплей
  display_instance->display();

  lv_display_flush_ready(disp);
}

bool lvgl_port_init(SSD1283A *display) {
  if (!display) {
    ESP_LOGE(TAG, "Display pointer is null");
    return false;
  }

  display_instance = display;

  // Создаем мьютекс для LVGL
  lvgl_mutex = xSemaphoreCreateMutex();
  if (!lvgl_mutex) {
    ESP_LOGE(TAG, "Failed to create LVGL mutex");
    return false;
  }

  // Инициализация LVGL
  lv_init();

  // Создание дисплея
  disp = lv_display_create(SSD1283A_WIDTH, SSD1283A_HEIGHT);
  if (!disp) {
    ESP_LOGE(TAG, "Failed to create display");
    return false;
  }

  // Настройка дисплея
  lv_display_set_flush_cb(disp, disp_flush_cb);

  // Установка буферов для дисплея
  lv_display_set_buffers(disp, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Установка цветовой глубины 16 бит
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);

  ESP_LOGI(TAG, "LVGL initialized successfully");
  return true;
}

void lvgl_port_flush_ready() {
  // Эта функция вызывается из disp_flush_cb
  // Ничего дополнительно делать не нужно
}

void lvgl_port_delay(uint32_t delay_ms) {
  vTaskDelay(pdMS_TO_TICKS(delay_ms));
}

uint32_t lvgl_port_get_tick() {
  return xTaskGetTickCount();
}

void lvgl_port_task() {
  if (xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
    lv_timer_handler();
    xSemaphoreGive(lvgl_mutex);
  }
}

// Функция для создания задачи LVGL
void lvgl_port_create_task() {
  xTaskCreate(
      [](void *arg) {
        while (true) {
          lvgl_port_task();
          vTaskDelay(pdMS_TO_TICKS(1));
        }
      },
      "lvgl_task",
      4096,
      NULL,
      5,
      NULL);
}