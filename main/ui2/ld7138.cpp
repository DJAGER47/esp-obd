#include "ld7138.h"

#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "ld7138";

// Внутренняя структура дескриптора LD7138
typedef struct ld7138_s {
  ld7138_config_t config;
  spi_device_handle_t spi_handle;
  bool initialized;
} ld7138_t;

// Вспомогательные функции
static uint16_t rgb_to_565(uint8_t r, uint8_t g, uint8_t b);

// Реализация API функций
esp_err_t ld7138_init(const ld7138_config_t *config, ld7138_handle_t *handle) {
  if (config == NULL || handle == NULL) {
    ESP_LOGE(TAG, "Invalid arguments");
    return ESP_ERR_INVALID_ARG;
  }

  // Выделение памяти для дескриптора
  ld7138_t *ld7138 = (ld7138_t *)calloc(1, sizeof(ld7138_t));
  if (ld7138 == NULL) {
    ESP_LOGE(TAG, "No memory for LD7138 handle");
    return ESP_ERR_NO_MEM;
  }

  // Сохранение конфигурации
  memcpy(&ld7138->config, config, sizeof(ld7138_config_t));

  // Настройка GPIO
  gpio_config_t io_conf = {};
  io_conf.intr_type     = GPIO_INTR_DISABLE;
  io_conf.mode          = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask  = (1ULL << config->reset_pin) | (1ULL << config->dc_pin) | (1ULL << config->cs_pin);
  io_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en    = GPIO_PULLUP_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&io_conf));

  // Настройка пина подсветки, если указан
  if (config->bk_light_pin != GPIO_NUM_NC) {
    gpio_set_level(config->bk_light_pin, 1);
  }

  // Конфигурация SPI шины
  spi_bus_config_t buscfg = {};
  memset(&buscfg, 0, sizeof(buscfg));
  buscfg.mosi_io_num     = config->mosi_pin;
  buscfg.miso_io_num     = -1;  // MISO не используется
  buscfg.sclk_io_num     = config->sclk_pin;
  buscfg.quadwp_io_num   = -1;
  buscfg.quadhd_io_num   = -1;
  buscfg.max_transfer_sz = LD7138_WIDTH * LD7138_HEIGHT * 2 + 8;

  // Инициализация SPI шины
  esp_err_t ret = spi_bus_initialize(config->spi_host, &buscfg, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
    free(ld7138);
    return ret;
  }

  // Конфигурация SPI устройства
  spi_device_interface_config_t devcfg = {};
  memset(&devcfg, 0, sizeof(devcfg));
  devcfg.clock_speed_hz = (int)config->clock_speed;
  devcfg.mode           = 0;  // SPI mode 0
  devcfg.spics_io_num   = config->cs_pin;
  devcfg.queue_size     = 7;
  devcfg.flags          = SPI_DEVICE_NO_DUMMY;

  // Добавление устройства на шину SPI
  ret = spi_bus_add_device(config->spi_host, &devcfg, &ld7138->spi_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
    free(ld7138);
    return ret;
  }

  // Сброс дисплея
  ld7138_reset((ld7138_handle_t)ld7138);

  // Инициализация дисплея
  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x01_SOFTRES);
  vTaskDelay(pdMS_TO_TICKS(120));

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x02_DISPLAY_ON_OFF);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Выключить дисплей

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x03_DISPLAY_STANDBY_ON_OFF);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Запустить осциллятор

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x04_OSCILLATOR_SPEED);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x02);  // 90Hz

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x05_WRITE_DIRECTION);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Направление записи, порядок цветов RGB

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x06_SCAN_DIRECTION);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Направление сканирования

  // Установка окна дисплея
  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x07_SET_DISPLAY_WINDOW);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Xstart старшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Xstart младшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x07);  // Xend старшие биты (127)
  ld7138_write_data((ld7138_handle_t)ld7138, 0x7F);  // Xend младшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x03);  // Ystart старшие биты (60)
  ld7138_write_data((ld7138_handle_t)ld7138, 0x0C);  // Ystart младшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x05);  // Yend старшие биты (95)
  ld7138_write_data((ld7138_handle_t)ld7138, 0x0F);  // Yend младшие биты

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x08_IF_BUS_SEL);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x01);  // 8-битный интерфейс

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x09_DATA_MASKING);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x07);  // Показывать все данные

  // Установка окна данных
  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x0A_SET_DATA_WINDOW);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Xstart старшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Xstart младшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x07);  // Xend старшие биты (127)
  ld7138_write_data((ld7138_handle_t)ld7138, 0x7F);  // Xend младшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x03);  // Ystart старшие биты (60)
  ld7138_write_data((ld7138_handle_t)ld7138, 0x0C);  // Ystart младшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x05);  // Yend старшие биты (95)
  ld7138_write_data((ld7138_handle_t)ld7138, 0x0F);  // Yend младшие биты

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x0B_SET_ADDRESS);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Xstart старшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Xstart младшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Ystart старшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // Ystart младшие биты

  // Установка уровней тока
  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x0E_RGB_CURRENT_LEVEL);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x08);  // IRED старшие биты (135)
  ld7138_write_data((ld7138_handle_t)ld7138, 0x07);  // IRED младшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x03);  // IGREEN старшие биты (54)
  ld7138_write_data((ld7138_handle_t)ld7138, 0x06);  // IGREEN младшие биты
  ld7138_write_data((ld7138_handle_t)ld7138, 0x03);  // IBLUE старшие биты (50)
  ld7138_write_data((ld7138_handle_t)ld7138, 0x02);  // IBLUE младшие биты

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x0F_PEAK_CURRENT_LEVEL);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x05);  // Red
  ld7138_write_data((ld7138_handle_t)ld7138, 0x10);  // Green
  ld7138_write_data((ld7138_handle_t)ld7138, 0x23);  // Blue

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x1C_PRE_CHARGE_WIDTH);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x01);

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x1C_SET_PEAK_WIDTH);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x3F);  // Red
  ld7138_write_data((ld7138_handle_t)ld7138, 0x10);  // Green
  ld7138_write_data((ld7138_handle_t)ld7138, 0x3C);  // Blue

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x1E_SET_PEAK_DELAY);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x0F);

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x1F_SET_ROW_SCAN);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x30);

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x30_VCC_R_SELECT);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x11);

  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x3C_SET_VDD_SELECTION);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x00);  // 2.8V

  // Очистка дисплея
  ld7138_fill((ld7138_handle_t)ld7138, 0, 0, 0);

  // Включение дисплея
  ld7138_write_cmd((ld7138_handle_t)ld7138, LD7138_0x02_DISPLAY_ON_OFF);
  ld7138_write_data((ld7138_handle_t)ld7138, 0x01);  // Включить дисплей

  ld7138->initialized = true;
  *handle             = (ld7138_handle_t)ld7138;

  ESP_LOGI(TAG, "LD7138 initialized successfully");
  return ESP_OK;
}

esp_err_t ld7138_free(ld7138_handle_t handle) {
  if (handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  ld7138_t *ld7138 = (ld7138_t *)handle;

  if (ld7138->initialized) {
    spi_bus_remove_device(ld7138->spi_handle);
    spi_bus_free(ld7138->config.spi_host);
  }

  free(ld7138);
  return ESP_OK;
}

esp_err_t ld7138_reset(ld7138_handle_t handle) {
  if (handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  ld7138_t *ld7138 = (ld7138_t *)handle;

  gpio_set_level(ld7138->config.reset_pin, 0);
  vTaskDelay(pdMS_TO_TICKS(10));
  gpio_set_level(ld7138->config.reset_pin, 1);
  vTaskDelay(pdMS_TO_TICKS(120));

  return ESP_OK;
}

esp_err_t ld7138_display_on_off(ld7138_handle_t handle, bool on) {
  if (handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  ld7138_write_cmd(handle, LD7138_0x02_DISPLAY_ON_OFF);
  ld7138_write_data(handle, on ? 0x01 : 0x00);

  return ESP_OK;
}

esp_err_t ld7138_clear(ld7138_handle_t handle) {
  return ld7138_fill(handle, 0, 0, 0);
}

esp_err_t ld7138_fill(ld7138_handle_t handle, uint8_t r, uint8_t g, uint8_t b) {
  if (handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  // Установка окна на весь дисплей
  ld7138_set_window(handle, 0, 0, LD7138_WIDTH - 1, LD7138_HEIGHT - 1);

  // Подготовка данных для заполнения
  uint16_t color        = rgb_to_565(r, g, b);
  uint8_t color_data[2] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};

  // Отправка команды записи данных
  ld7138_write_cmd(handle, LD7138_0x0C_DATA_WRITE_READ);

  // Заполнение дисплея
  ld7138_t *ld7138 = (ld7138_t *)handle;
  gpio_set_level(ld7138->config.dc_pin, 1);  // Режим данных

  // Создание буфера для заполнения
  size_t pixel_count = LD7138_WIDTH * LD7138_HEIGHT;
  size_t buffer_size = pixel_count * 2;
  uint8_t *buffer    = (uint8_t *)malloc(buffer_size);
  if (buffer == NULL) {
    return ESP_ERR_NO_MEM;
  }

  // Заполнение буфера цветом
  for (size_t i = 0; i < pixel_count; i++) {
    buffer[i * 2]     = color_data[0];
    buffer[i * 2 + 1] = color_data[1];
  }

  // Отправка данных
  spi_transaction_t trans = {};
  memset(&trans, 0, sizeof(trans));
  trans.length    = buffer_size * 8;  // в битах
  trans.tx_buffer = buffer;

  spi_device_transmit(ld7138->spi_handle, &trans);

  // Освобождение буфера
  free(buffer);

  return ESP_OK;
}

esp_err_t ld7138_draw_pixel(ld7138_handle_t handle, uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
  if (handle == NULL || x >= LD7138_WIDTH || y >= LD7138_HEIGHT) {
    return ESP_ERR_INVALID_ARG;
  }

  // Установка окна для одного пикселя
  ld7138_set_window(handle, x, y, x, y);

  // Отправка команды записи данных
  ld7138_write_cmd(handle, LD7138_0x0C_DATA_WRITE_READ);

  // Отправка данных пикселя
  uint16_t color        = rgb_to_565(r, g, b);
  uint8_t color_data[2] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};
  ld7138_write_data_buffer(handle, color_data, 2);

  return ESP_OK;
}

esp_err_t ld7138_draw_line(
    ld7138_handle_t handle, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t r, uint8_t g, uint8_t b) {
  if (handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  // Алгоритм Брезенхэма для рисования линии
  int16_t dx  = abs(x1 - x0);
  int16_t sx  = x0 < x1 ? 1 : -1;
  int16_t dy  = abs(y1 - y0);
  int16_t sy  = y0 < y1 ? 1 : -1;
  int16_t err = (dx > dy ? dx : -dy) / 2;
  int16_t e2;

  while (1) {
    ld7138_draw_pixel(handle, x0, y0, r, g, b);

    if (x0 == x1 && y0 == y1)
      break;

    e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy) {
      err += dx;
      y0 += sy;
    }
  }

  return ESP_OK;
}

esp_err_t ld7138_draw_rect(
    ld7138_handle_t handle, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b) {
  if (handle == NULL || x >= LD7138_WIDTH || y >= LD7138_HEIGHT) {
    return ESP_ERR_INVALID_ARG;
  }

  // Проверка границ
  if (x + w > LD7138_WIDTH)
    w = LD7138_WIDTH - x;
  if (y + h > LD7138_HEIGHT)
    h = LD7138_HEIGHT - y;

  // Рисование четырех сторон прямоугольника
  ld7138_draw_line(handle, x, y, x + w - 1, y, r, g, b);                  // Верхняя сторона
  ld7138_draw_line(handle, x, y + h - 1, x + w - 1, y + h - 1, r, g, b);  // Нижняя сторона
  ld7138_draw_line(handle, x, y, x, y + h - 1, r, g, b);                  // Левая сторона
  ld7138_draw_line(handle, x + w - 1, y, x + w - 1, y + h - 1, r, g, b);  // Правая сторона

  return ESP_OK;
}

esp_err_t ld7138_fill_rect(
    ld7138_handle_t handle, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b) {
  if (handle == NULL || x >= LD7138_WIDTH || y >= LD7138_HEIGHT) {
    return ESP_ERR_INVALID_ARG;
  }

  // Проверка границ
  if (x + w > LD7138_WIDTH)
    w = LD7138_WIDTH - x;
  if (y + h > LD7138_HEIGHT)
    h = LD7138_HEIGHT - y;

  // Установка окна для прямоугольника
  ld7138_set_window(handle, x, y, x + w - 1, y + h - 1);

  // Отправка команды записи данных
  ld7138_write_cmd(handle, LD7138_0x0C_DATA_WRITE_READ);

  // Подготовка данных для заполнения
  uint16_t color        = rgb_to_565(r, g, b);
  uint8_t color_data[2] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};

  // Заполнение прямоугольника
  ld7138_t *ld7138 = (ld7138_t *)handle;
  gpio_set_level(ld7138->config.dc_pin, 1);  // Режим данных

  // Создание буфера для заполнения
  size_t pixel_count = w * h;
  size_t buffer_size = pixel_count * 2;
  uint8_t *buffer    = (uint8_t *)malloc(buffer_size);
  if (buffer == NULL) {
    return ESP_ERR_NO_MEM;
  }

  // Заполнение буфера цветом
  for (size_t i = 0; i < pixel_count; i++) {
    buffer[i * 2]     = color_data[0];
    buffer[i * 2 + 1] = color_data[1];
  }

  // Отправка данных
  spi_transaction_t trans = {};
  memset(&trans, 0, sizeof(trans));
  trans.length    = buffer_size * 8;  // в битах
  trans.tx_buffer = buffer;

  spi_device_transmit(ld7138->spi_handle, &trans);

  // Освобождение буфера
  free(buffer);

  return ESP_OK;
}

esp_err_t ld7138_draw_circle(
    ld7138_handle_t handle, uint16_t x0, uint16_t y0, uint16_t radius, uint8_t r, uint8_t g, uint8_t b) {
  if (handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  // Алгоритм окружности средней точки
  int16_t x           = radius;
  int16_t y           = 0;
  int16_t radiusError = 1 - x;

  while (x >= y) {
    // Рисование 8 точек окружности
    ld7138_draw_pixel(handle, x0 - y, y0 + x, r, g, b);  // 11 часов
    ld7138_draw_pixel(handle, x0 + y, y0 + x, r, g, b);  // 1 час
    ld7138_draw_pixel(handle, x0 - x, y0 + y, r, g, b);  // 10 часов
    ld7138_draw_pixel(handle, x0 + x, y0 + y, r, g, b);  // 2 часа
    ld7138_draw_pixel(handle, x0 - x, y0 - y, r, g, b);  // 8 часов
    ld7138_draw_pixel(handle, x0 + x, y0 - y, r, g, b);  // 4 часа
    ld7138_draw_pixel(handle, x0 - y, y0 - x, r, g, b);  // 7 часов
    ld7138_draw_pixel(handle, x0 + y, y0 - x, r, g, b);  // 5 часов

    y++;
    if (radiusError < 0) {
      radiusError += 2 * y + 1;
    } else {
      x--;
      radiusError += 2 * (y - x + 1);
    }
  }

  return ESP_OK;
}

esp_err_t ld7138_draw_image(
    ld7138_handle_t handle, const uint8_t *img, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  if (handle == NULL || img == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  // Проверка границ
  if (x >= LD7138_WIDTH || y >= LD7138_HEIGHT || x + w > LD7138_WIDTH || y + h > LD7138_HEIGHT) {
    return ESP_ERR_INVALID_ARG;
  }

  // Установка окна для изображения
  ld7138_set_window(handle, x, y, x + w - 1, y + h - 1);

  // Отправка команды записи данных
  ld7138_write_cmd(handle, LD7138_0x0C_DATA_WRITE_READ);

  // Подготовка буфера для преобразования RGB888 в RGB565
  uint16_t img_size      = w * h;
  uint8_t *rgb565_buffer = (uint8_t *)malloc(img_size * 2);
  if (rgb565_buffer == NULL) {
    return ESP_ERR_NO_MEM;
  }

  // Преобразование RGB888 в RGB565
  for (uint16_t i = 0; i < img_size; i++) {
    uint16_t idx = i * 3;
    uint8_t r    = img[idx];
    uint8_t g    = img[idx + 1];
    uint8_t b    = img[idx + 2];

    uint16_t rgb565          = rgb_to_565(r, g, b);
    rgb565_buffer[i * 2]     = rgb565 >> 8;
    rgb565_buffer[i * 2 + 1] = rgb565 & 0xFF;
  }

  // Отправка данных
  ld7138_write_data_buffer(handle, rgb565_buffer, img_size * 2);

  // Освобождение буфера
  free(rgb565_buffer);

  return ESP_OK;
}

// Вспомогательные функции
esp_err_t ld7138_write_cmd(ld7138_handle_t handle, uint8_t cmd) {
  if (handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  ld7138_t *ld7138 = (ld7138_t *)handle;

  gpio_set_level(ld7138->config.dc_pin, 0);  // Режим команды

  spi_transaction_t trans = {};
  memset(&trans, 0, sizeof(trans));
  trans.length     = 8;  // 8 бит
  trans.flags      = SPI_TRANS_USE_TXDATA;
  trans.tx_data[0] = cmd;

  return spi_device_transmit(ld7138->spi_handle, &trans);
}

esp_err_t ld7138_write_data(ld7138_handle_t handle, uint8_t data) {
  if (handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  ld7138_t *ld7138 = (ld7138_t *)handle;

  gpio_set_level(ld7138->config.dc_pin, 1);  // Режим данных

  spi_transaction_t trans = {};
  memset(&trans, 0, sizeof(trans));
  trans.length     = 8;  // 8 бит
  trans.flags      = SPI_TRANS_USE_TXDATA;
  trans.tx_data[0] = data;

  return spi_device_transmit(ld7138->spi_handle, &trans);
}

esp_err_t ld7138_write_data_buffer(ld7138_handle_t handle, const uint8_t *data, size_t len) {
  if (handle == NULL || data == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  ld7138_t *ld7138 = (ld7138_t *)handle;

  gpio_set_level(ld7138->config.dc_pin, 1);  // Режим данных

  spi_transaction_t trans = {};
  memset(&trans, 0, sizeof(trans));
  trans.length    = len * 8;  // в битах
  trans.tx_buffer = data;

  return spi_device_transmit(ld7138->spi_handle, &trans);
}

void ld7138_set_window(ld7138_handle_t handle, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  // Установка окна для записи данных
  ld7138_write_cmd(handle, LD7138_0x0A_SET_DATA_WINDOW);

  // Xstart
  ld7138_write_data(handle, (x0 >> 4) & 0x07);
  ld7138_write_data(handle, x0 & 0x0F);

  // Xend
  ld7138_write_data(handle, (x1 >> 4) & 0x07);
  ld7138_write_data(handle, x1 & 0x0F);

  // Ystart (добавляем 60, так как дисплей использует строки 60-95)
  y0 += 60;
  ld7138_write_data(handle, (y0 >> 4) & 0x07);
  ld7138_write_data(handle, y0 & 0x0F);

  // Yend (добавляем 60, так как дисплей использует строки 60-95)
  y1 += 60;
  ld7138_write_data(handle, (y1 >> 4) & 0x07);
  ld7138_write_data(handle, y1 & 0x0F);

  // Команда для начала записи данных
  ld7138_write_cmd(handle, LD7138_0x0C_DATA_WRITE_READ);
}

static uint16_t rgb_to_565(uint8_t r, uint8_t g, uint8_t b) {
  // Преобразование RGB888 в RGB565
  // Формат: RRRRR GGGGGG BBBBB (5 бит R, 6 бит G, 5 бит B)
  // Первый байт: RRRRR GGG, второй байт: GGG BBBBB
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
