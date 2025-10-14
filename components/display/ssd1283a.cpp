#include "ssd1283a.h"

const char* SSD1283A::TAG_ = "SSD1283A";

SSD1283A::SSD1283A(gpio_num_t mosi,
                   gpio_num_t sclk,
                   gpio_num_t cs,
                   gpio_num_t dc,
                   gpio_num_t rst,
                   spi_host_device_t spi_host) :
    mosi_(mosi),
    sclk_(sclk),
    cs_(cs),
    dc_(dc),
    rst_(rst),
    spi_host_(spi_host),
    spi_handle_(nullptr),
    rotation_(2),
    width_(SSD1283A_WIDTH),
    height_(SSD1283A_HEIGHT),
    inversion_bit_(0) {
  memset(buffer_, 0, sizeof(buffer_));
}

bool SSD1283A::init() {
  ESP_LOGI(TAG_, "Initializing SSD1283A display");

  // Конфигурация SPI
  spi_bus_config_t bus_cfg = {.mosi_io_num     = mosi_,
                              .miso_io_num     = -1,  // MISO не используется
                              .sclk_io_num     = sclk_,
                              .quadwp_io_num   = -1,
                              .quadhd_io_num   = -1,
                              .max_transfer_sz = SSD1283A_BUFFER_SIZE + 8,
                              .flags           = 0,
                              .intr_flags      = 0};

  // Инициализация SPI шины
  esp_err_t ret = spi_bus_initialize(spi_host_, &bus_cfg, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG_, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
    return false;
  }

  // Конфигурация SPI устройства
  spi_device_interface_config_t dev_cfg = {.command_bits     = 0,
                                           .address_bits     = 0,
                                           .dummy_bits       = 0,
                                           .mode             = 0,  // Mode 0
                                           .duty_cycle_pos   = 128,
                                           .cs_ena_pretrans  = 1,
                                           .cs_ena_posttrans = 1,
                                           .clock_speed_hz   = 10 * 1000 * 1000,  // 10 MHz
                                           .input_delay_ns   = 0,
                                           .spics_io_num     = cs_,
                                           .flags            = SPI_DEVICE_NO_DUMMY,
                                           .queue_size       = 7,
                                           .pre_cb           = nullptr,
                                           .post_cb          = nullptr};

  // Добавление устройства на шину
  ret = spi_bus_add_device(spi_host_, &dev_cfg, &spi_handle_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG_, "Failed to add SPI device: %s", esp_err_to_name(ret));
    return false;
  }

  // Настройка GPIO
  gpio_config_t io_conf = {};
  io_conf.intr_type     = GPIO_INTR_DISABLE;
  io_conf.mode          = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask  = (1ULL << dc_) | (1ULL << rst_);
  io_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en    = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);

  // Сброс и инициализация дисплея
  reset();
  setup();

  ESP_LOGI(TAG_, "SSD1283A display initialized successfully");
  return true;
}

void SSD1283A::reset() {
  ESP_LOGI(TAG_, "Resetting display");

  gpio_set_level(rst_, 0);
  vTaskDelay(pdMS_TO_TICKS(10));
  gpio_set_level(rst_, 1);
  vTaskDelay(pdMS_TO_TICKS(10));
}

void SSD1283A::setup() {
  ESP_LOGI(TAG_, "Setting up display");

  writeCommand(SSD1283A_DISPLAY_OFF);
  writeCommand(SSD1283A_RESET);
  vTaskDelay(pdMS_TO_TICKS(5));

  // Правильная инициализация для SSD1283A на основе временного файла
  const uint16_t ssd1283a_init_values[] = {
      0x10,   0x2F8E,  // Start OSC
      0x11,   0x000C,  // Power control
      0x07,   0x0021,  // Display control
      0x28,   0x0006,  // Analog circuit set
      0x28,   0x0005,  // Analog circuit set
      0x27,   0x057F,  // LCD drive AC control
      0x29,   0x89A1,  // LCD drive waveform control
      0x00,   0x0001,  // Start oscillation
      0xFFFF, 100,  // Задержка (используем 0xFFFF как маркер задержки)
      0x29,   0x80B0,  // LCD drive waveform control
      0xFFFF, 30,      // Задержка
      0x29,   0xFFFE,  // LCD drive waveform control
      0x07,   0x0223,  // Display control
      0xFFFF, 30,      // Задержка
      0x07,   0x0233,  // Display control
      0x01,   0x2183,  // Driver output control
      0x03,   0x6830,  // Entry mode
      0x2F,   0xFFFF,  // RAM write data mask
      0x2C,   0x8000,  // RAM address set
      0x27,   0x0570,  // LCD drive AC control
      0x02,   0x0300,  // Power control
      0x0B,   0x580C,  // Frame cycle control
      0x12,   0x0609,  // Power control
      0x13,   0x3100,  // Power control
  };

  // Отправка инициализационной последовательности
  for (size_t i = 0; i < sizeof(ssd1283a_init_values) / sizeof(uint16_t); i += 2) {
    uint16_t cmd  = ssd1283a_init_values[i];
    uint16_t data = ssd1283a_init_values[i + 1];

    if (cmd == 0xFFFF) {
      // Задержка
      vTaskDelay(pdMS_TO_TICKS(data));
    } else {
      writeCommand(cmd);
      writeData(data & 0xFF);
      writeData((data >> 8) & 0xFF);
    }
  }

  display();

  writeCommand(SSD1283A_DISPLAY_ON);

  ESP_LOGI(TAG_, "Display setup completed");
}

void SSD1283A::writeCommand(uint8_t cmd) {
  gpio_set_level(dc_, 0);  // Command mode

  spi_transaction_t trans = {.length = 8, .tx_buffer = &cmd, .rx_buffer = nullptr};

  spi_device_transmit(spi_handle_, &trans);
}

void SSD1283A::writeData(uint8_t data) {
  gpio_set_level(dc_, 1);  // Data mode

  spi_transaction_t trans = {.length = 8, .tx_buffer = &data, .rx_buffer = nullptr};

  spi_device_transmit(spi_handle_, &trans);
}

void SSD1283A::writeData(const uint8_t* data, size_t len) {
  gpio_set_level(dc_, 1);  // Data mode

  spi_transaction_t trans = {.length = len * 8, .tx_buffer = data, .rx_buffer = nullptr};

  spi_device_transmit(spi_handle_, &trans);
}

void SSD1283A::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x < 0 || x >= SSD1283A_WIDTH || y < 0 || y >= SSD1283A_HEIGHT) {
    return;
  }

  uint16_t index = y * SSD1283A_WIDTH + x;
  buffer_[index] = color;
}

void SSD1283A::on() {
  writeCommand(SSD1283A_DISPLAY_ON);
}

void SSD1283A::off() {
  writeCommand(SSD1283A_DISPLAY_OFF);
}

void SSD1283A::setContrast(uint8_t contrast) {
  writeCommand(SSD1283A_CONTRAST);
  writeCommand(contrast);
}

void SSD1283A::setRotation(uint8_t rotation) {
  rotation_ = rotation & 3;
  width_    = (rotation_ & 1) ? SSD1283A_HEIGHT : SSD1283A_WIDTH;
  height_   = (rotation_ & 1) ? SSD1283A_WIDTH : SSD1283A_HEIGHT;

  // Отправка команд для установки вращения
  writeCommand(0x01);  // Driver Output Control
  switch (rotation_) {
    case 0:
      writeData(inversion_bit_ | 0x2183);
      writeData((inversion_bit_ | 0x2183) >> 8);
      writeCommand(0x03);  // Entry Mode
      writeData(0x6830);
      writeData(0x6830 >> 8);
      break;
    case 1:
      writeData(inversion_bit_ | 0x2283);
      writeData((inversion_bit_ | 0x2283) >> 8);
      writeCommand(0x03);  // Entry Mode
      writeData(0x6808);
      writeData(0x6808 >> 8);
      break;
    case 2:
      writeData(inversion_bit_ | 0x2183);
      writeData((inversion_bit_ | 0x2183) >> 8);
      writeCommand(0x03);  // Entry Mode
      writeData(0x6800);
      writeData(0x6800 >> 8);
      break;
    case 3:
      writeData(inversion_bit_ | 0x2283);
      writeData((inversion_bit_ | 0x2283) >> 8);
      writeCommand(0x03);  // Entry Mode
      writeData(0x6838);
      writeData(0x6838 >> 8);
      break;
  }

  setWindowAddress(0, 0, width_ - 1, height_ - 1);
}

void SSD1283A::invertDisplay(bool invert) {
  inversion_bit_ = invert ? 0x0800 : 0x0000;
  setRotation(rotation_);
}

void SSD1283A::setWindowAddress(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  setWindowAddressInternal(x1, y1, x2, y2);
}

void SSD1283A::setWindowAddressInternal(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  switch (rotation_) {
    case 0:
      writeCommand(0x44);
      writeData(x2 + 2);
      writeData(x1 + 2);
      writeCommand(0x45);
      writeData(y2 + 2);
      writeData(y1 + 2);
      writeCommand(0x21);
      writeData(y1 + 2);
      writeData(x1 + 2);
      break;
    case 1:
      writeCommand(0x44);
      writeData(SSD1283A_HEIGHT - y1 + 1);
      writeData(SSD1283A_HEIGHT - y2 + 1);
      writeCommand(0x45);
      writeData(SSD1283A_WIDTH - x1 - 1);
      writeData(SSD1283A_WIDTH - x2 - 1);
      writeCommand(0x21);
      writeData(SSD1283A_WIDTH - x1 - 1);
      writeData(SSD1283A_HEIGHT - y1 + 1);
      break;
    case 2:
      writeCommand(0x44);
      writeData(SSD1283A_WIDTH - x1 + 1);
      writeData(SSD1283A_WIDTH - x2 + 1);
      writeCommand(0x45);
      writeData(SSD1283A_HEIGHT - y1 + 1);
      writeData(SSD1283A_HEIGHT - y2 + 1);
      writeCommand(0x21);
      writeData(SSD1283A_HEIGHT - y1 + 1);
      writeData(SSD1283A_WIDTH - x1 + 1);
      break;
    case 3:
      writeCommand(0x44);
      writeData(y2 + 2);
      writeData(y1 + 2);
      writeCommand(0x45);
      writeData(x2);
      writeData(x1);
      writeCommand(0x21);
      writeData(x1);
      writeData(y1 + 2);
      break;
  }
  writeCommand(0x22);
}

void SSD1283A::display() {
  // Устанавливаем окно для всего дисплея
  setWindowAddress(0, 0, width_ - 1, height_ - 1);

  // Отправляем весь буфер на дисплей
  writeData((uint8_t*)buffer_, width_ * height_ * 2);
}