#pragma once

#include <stdint.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#ifdef __cplusplus
extern "C" {
#endif

// Разрешение дисплея CFAL12836A0-088
#define LD7138_WIDTH                       128
#define LD7138_HEIGHT                      36

// Команды LD7138
#define LD7138_0x01_SOFTRES                (0x01)
#define LD7138_0x02_DISPLAY_ON_OFF         (0x02)
#define LD7138_0x03_DISPLAY_STANDBY_ON_OFF (0x03)
#define LD7138_0x04_OSCILLATOR_SPEED       (0x04)
#define LD7138_0x05_WRITE_DIRECTION        (0x05)
#define LD7138_0x06_SCAN_DIRECTION         (0x06)
#define LD7138_0x07_SET_DISPLAY_WINDOW     (0x07)
#define LD7138_0x08_IF_BUS_SEL             (0x08)
#define LD7138_0x09_DATA_MASKING           (0x09)
#define LD7138_0x0A_SET_DATA_WINDOW        (0x0A)
#define LD7138_0x0B_SET_ADDRESS            (0x0B)
#define LD7138_0x0C_DATA_WRITE_READ        (0x0C)
#define LD7138_0x0D_REGISTER_READ          (0x0D)
#define LD7138_0x0E_RGB_CURRENT_LEVEL      (0x0E)
#define LD7138_0x0F_PEAK_CURRENT_LEVEL     (0x0F)
#define LD7138_0x10_SCLK                   (0x10)
#define LD7138_0x1C_PRE_CHARGE_WIDTH       (0x1C)
#define LD7138_0x1C_SET_PEAK_WIDTH         (0x1D)
#define LD7138_0x1E_SET_PEAK_DELAY         (0x1E)
#define LD7138_0x1F_SET_ROW_SCAN           (0x1F)
#define LD7138_0x30_VCC_R_SELECT           (0x30)
#define LD7138_0x34_RGB_MODE               (0x34)
#define LD7138_0x3A_GAMMA_TUNE             (0x3A)
#define LD7138_0x3B_GAMMA_INIT             (0x3B)
#define LD7138_0x3C_SET_VDD_SELECTION      (0x3C)
#define LD7138_0x3E_TEST                   (0x3E)

// Структура для конфигурации LD7138
typedef struct {
  gpio_num_t sclk_pin;         // Пин для тактового сигнала SPI
  gpio_num_t mosi_pin;         // Пин для данных SPI
  gpio_num_t reset_pin;        // Пин для сброса
  gpio_num_t dc_pin;           // Пин для выбора команды/данных (RS)
  gpio_num_t cs_pin;           // Пин для выбора чипа
  gpio_num_t bk_light_pin;     // Пин для управления подсветкой (опционально)
  spi_host_device_t spi_host;  // SPI хост (SPI2_HOST, SPI3_HOST)
  uint32_t clock_speed;        // Скорость SPI
} ld7138_config_t;

// Структура дескриптора LD7138
typedef struct ld7138_s *ld7138_handle_t;

/**
 * @brief Инициализация дисплея LD7138
 *
 * @param config Конфигурация дисплея
 * @param handle Указатель для сохранения дескриптора
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_init(const ld7138_config_t *config, ld7138_handle_t *handle);

/**
 * @brief Деинициализация дисплея LD7138
 *
 * @param handle Дескриптор дисплея
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_free(ld7138_handle_t handle);

/**
 * @brief Сброс дисплея
 *
 * @param handle Дескриптор дисплея
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_reset(ld7138_handle_t handle);

/**
 * @brief Включение/выключение дисплея
 *
 * @param handle Дескриптор дисплея
 * @param on true для включения, false для выключения
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_display_on_off(ld7138_handle_t handle, bool on);

/**
 * @brief Очистка дисплея (заполнение черным цветом)
 *
 * @param handle Дескриптор дисплея
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_clear(ld7138_handle_t handle);

/**
 * @brief Заполнение дисплея указанным цветом
 *
 * @param handle Дескриптор дисплея
 * @param r Красный компонент (0-255)
 * @param g Зеленый компонент (0-255)
 * @param b Синий компонент (0-255)
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_fill(ld7138_handle_t handle, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Отрисовка пикселя
 *
 * @param handle Дескриптор дисплея
 * @param x Координата X
 * @param y Координата Y
 * @param r Красный компонент (0-255)
 * @param g Зеленый компонент (0-255)
 * @param b Синий компонент (0-255)
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_draw_pixel(ld7138_handle_t handle, uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Отрисовка линии
 *
 * @param handle Дескриптор дисплея
 * @param x0 Начальная координата X
 * @param y0 Начальная координата Y
 * @param x1 Конечная координата X
 * @param y1 Конечная координата Y
 * @param r Красный компонент (0-255)
 * @param g Зеленый компонент (0-255)
 * @param b Синий компонент (0-255)
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_draw_line(
    ld7138_handle_t handle, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Отрисовка прямоугольника
 *
 * @param handle Дескриптор дисплея
 * @param x Координата X левого верхнего угла
 * @param y Координата Y левого верхнего угла
 * @param w Ширина
 * @param h Высота
 * @param r Красный компонент (0-255)
 * @param g Зеленый компонент (0-255)
 * @param b Синий компонент (0-255)
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_draw_rect(
    ld7138_handle_t handle, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Отрисовка заполненного прямоугольника
 *
 * @param handle Дескриптор дисплея
 * @param x Координата X левого верхнего угла
 * @param y Координата Y левого верхнего угла
 * @param w Ширина
 * @param h Высота
 * @param r Красный компонент (0-255)
 * @param g Зеленый компонент (0-255)
 * @param b Синий компонент (0-255)
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_fill_rect(
    ld7138_handle_t handle, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Отрисовка круга
 *
 * @param handle Дескриптор дисплея
 * @param x0 Координата X центра
 * @param y0 Координата Y центра
 * @param radius Радиус
 * @param r Красный компонент (0-255)
 * @param g Зеленый компонент (0-255)
 * @param b Синий компонент (0-255)
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_draw_circle(
    ld7138_handle_t handle, uint16_t x0, uint16_t y0, uint16_t radius, uint8_t r, uint8_t g, uint8_t b);
/**
 * @brief Отрисовка изображения
 *
 * @param handle Дескриптор дисплея
 * @param img Указатель на данные изображения (RGB888)
 * @param x Координата X
 * @param y Координата Y
 * @param w Ширина изображения
 * @param h Высота изображения
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_draw_image(ld7138_handle_t handle, const uint8_t *img, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
 * @brief Установка окна для записи данных
 *
 * @param handle Дескриптор дисплея
 * @param x0 Начальная координата X
 * @param y0 Начальная координата Y
 * @param x1 Конечная координата X
 * @param y1 Конечная координата Y
 */
void ld7138_set_window(ld7138_handle_t handle, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Запись команды
 *
 * @param handle Дескриптор дисплея
 * @param cmd Команда
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_write_cmd(ld7138_handle_t handle, uint8_t cmd);

/**
 * @brief Запись данных
 *
 * @param handle Дескриптор дисплея
 * @param data Данные
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_write_data(ld7138_handle_t handle, uint8_t data);

/**
 * @brief Запись буфера данных
 *
 * @param handle Дескриптор дисплея
 * @param data Указатель на данные
 * @param len Размер данных
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_write_data_buffer(ld7138_handle_t handle, const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif