#pragma once

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#ifdef __cplusplus
extern "C" {
#endif

// Команды LD7138
#define LD7138_CMD_SWRESET  0x01  // Software Reset
#define LD7138_CMD_SLPOUT   0x11  // Sleep Out
#define LD7138_CMD_NORON    0x13  // Normal Display Mode On
#define LD7138_CMD_INVOFF   0x20  // Display Inversion Off
#define LD7138_CMD_INVON    0x21  // Display Inversion On
#define LD7138_CMD_DISPOFF  0x28  // Display Off
#define LD7138_CMD_DISPON   0x29  // Display On
#define LD7138_CMD_CASET    0x2A  // Column Address Set
#define LD7138_CMD_RASET    0x2B  // Row Address Set
#define LD7138_CMD_RAMWR    0x2C  // Memory Write
#define LD7138_CMD_MADCTL   0x36  // Memory Data Access Control
#define LD7138_CMD_COLMOD   0x3A  // Interface Pixel Format

// Параметры для MADCTL
#define LD7138_MADCTL_MY    0x80  // Bottom to Top
#define LD7138_MADCTL_MX    0x40  // Right to Left
#define LD7138_MADCTL_MV    0x20  // Reverse Mode
#define LD7138_MADCTL_ML    0x10  // LCD Refresh Bottom to Top
#define LD7138_MADCTL_RGB   0x00  // RGB Order
#define LD7138_MADCTL_BGR   0x08  // BGR Order
#define LD7138_MADCTL_MH    0x04  // LCD Refresh Right to Left

// Параметры для COLMOD
#define LD7138_COLMOD_12BIT 0x03  // 12-bit/pixel
#define LD7138_COLMOD_16BIT 0x05  // 16-bit/pixel
#define LD7138_COLMOD_18BIT 0x06  // 18-bit/pixel

/**
 * @brief Инициализация дисплея LD7138
 *
 * @param io_handle Дескриптор интерфейса панели
 * @param panel_config Конфигурация панели
 * @param panel_handle Указатель для сохранения дескриптора панели
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t esp_lcd_new_panel_ld7138(const esp_lcd_panel_io_handle_t io_handle,
                                   const esp_lcd_panel_dev_config_t *panel_config,
                                   esp_lcd_panel_handle_t *panel_handle);

#ifdef __cplusplus
}
#endif