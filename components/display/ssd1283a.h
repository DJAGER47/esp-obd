#pragma once

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/task.h>

#include <cstring>

// Команды SSD1283A
#define SSD1283A_DISPLAY_OFF         0xAE
#define SSD1283A_DISPLAY_ON          0xAF
#define SSD1283A_DISPLAY_START_LINE  0x40
#define SSD1283A_PAGE_ADDRESS        0xB0
#define SSD1283A_COLUMN_ADDRESS_HIGH 0x10
#define SSD1283A_COLUMN_ADDRESS_LOW  0x00
#define SSD1283A_ADC_SELECT          0xA0
#define SSD1283A_DISPLAY_NORMAL      0xA6
#define SSD1283A_DISPLAY_REVERSE     0xA7
#define SSD1283A_DISPLAY_ALL_POINTS  0xA5
#define SSD1283A_BIAS_SELECT         0xA2
#define SSD1283A_READ_MODIFY_WRITE   0xE0
#define SSD1283A_END                 0xEE
#define SSD1283A_RESET               0xE2
#define SSD1283A_COM_OUTPUT          0xC0
#define SSD1283A_CONTRAST            0x81
#define SSD1283A_STATIC_CONTROL      0xAC
#define SSD1283A_DUTY_CYCLE          0xA8
#define SSD1283A_POWER_CONTROL       0x2F

// Цветовые команды для RGB режимов
#define SSD1283A_SET_COLOR_MODE      0x3A
#define SSD1283A_COLOR_MODE_65K      0x05  // 16-битный цвет (5-6-5)
#define SSD1283A_COLOR_MODE_262K     0x06  // 18-битный цвет (6-6-6)

// Размеры дисплея
#define SSD1283A_WIDTH               130
#define SSD1283A_HEIGHT              130
#define SSD1283A_BUFFER_SIZE         (SSD1283A_WIDTH * SSD1283A_HEIGHT * 2)  // 2 байта на пиксель (RGB565)

// Структура для 16-битного цвета (RGB565)
typedef union {
  struct {
    uint16_t blue : 5;
    uint16_t green : 6;
    uint16_t red : 5;
  };
  uint16_t value;
} rgb565_t;

class SSD1283A {
 public:
  /**
   * @brief Конструктор класса SSD1283A
   *
   * @param mosi MOSI пин
   * @param sclk SCLK пин
   * @param cs CS пин
   * @param dc DC (Data/Command) пин
   * @param rst RST пин
   * @param spi_host SPI хост (SPI2_HOST или SPI3_HOST)
   */
  SSD1283A(gpio_num_t mosi,
           gpio_num_t sclk,
           gpio_num_t cs,
           gpio_num_t dc,
           gpio_num_t rst,
           spi_host_device_t spi_host = SPI2_HOST);

  /**
   * @brief Инициализация дисплея
   *
   * @return true если инициализация прошла успешно, иначе false
   */
  bool init();

  /**
   * @brief Установка пикселя
   *
   * @param x координата X
   * @param y координата Y
   * @param color цвет (16-битный RGB565)
   */
  void drawPixel(int16_t x, int16_t y, uint16_t color);

  /**
   * @brief Включение дисплея
   */
  void on();

  /**
   * @brief Выключение дисплея
   */
  void off();

  /**
   * @brief Установка контрастности
   *
   * @param contrast значение контрастности (0-255)
   */
  void setContrast(uint8_t contrast);

  /**
   * @brief Получение ширины дисплея
   *
   * @return ширина в пикселях
   */
  int16_t getWidth() const {
    return width_;
  }

  /**
   * @brief Получение высоты дисплея
   *
   * @return высота в пикселях
   */
  int16_t getHeight() const {
    return height_;
  }

  /**
   * @brief Установка вращения дисплея
   *
   * @param rotation значение вращения (0-3)
   */
  void setRotation(uint8_t rotation);

  /**
   * @brief Инвертирование дисплея
   *
   * @param invert true для инвертирования, false для нормального режима
   */
  void invertDisplay(bool invert);

  /**
   * @brief Обновление дисплея (отправка буфера на дисплей)
   */
  void display();

  /**
   * @brief Получение указателя на буфер дисплея
   *
   * @return указатель на буфер дисплея
   */
  uint16_t* getBuffer() {
    return buffer_;
  }

 private:
  const gpio_num_t mosi_;
  const gpio_num_t sclk_;
  const gpio_num_t cs_;
  const gpio_num_t dc_;
  const gpio_num_t rst_;
  const spi_host_device_t spi_host_;
  spi_device_handle_t spi_handle_;

  // Параметры вращения
  uint8_t rotation_;
  uint16_t width_  = SSD1283A_WIDTH;
  uint16_t height_ = SSD1283A_HEIGHT;
  uint16_t inversion_bit_;

  // Буфер дисплея
  uint16_t buffer_[SSD1283A_WIDTH * SSD1283A_HEIGHT];

  static const char* TAG_;

  /**
   * @brief Отправка команды в дисплей
   *
   * @param cmd команда
   */
  void writeCommand(uint8_t cmd);

  /**
   * @brief Отправка данных в дисплей
   *
   * @param data данные
   */
  void writeData(uint8_t data);

  /**
   * @brief Отправка массива данных в дисплей
   *
   * @param data указатель на данные
   * @param len длина данных
   */
  void writeData(const uint8_t* data, size_t len);

  /**
   * @brief Сброс дисплея
   */
  void reset();

  /**
   * @brief Настройка дисплея
   */
  void setup();

  /**
   * @brief Установка окна адресации
   *
   * @param x1 начальная координата X
   * @param y1 начальная координата Y
   * @param x2 конечная координата X
   * @param y2 конечная координата Y
   */
  void setWindowAddress(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

  /**
   * @brief Установка окна адресации (внутренняя реализация)
   *
   * @param x1 начальная координата X
   * @param y1 начальная координата Y
   * @param x2 конечная координата X
   * @param y2 конечная координата Y
   */
  void setWindowAddressInternal(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
};