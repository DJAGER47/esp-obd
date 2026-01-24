/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"
#include "ld7138.h"

static const char *TAG = "lcd_panel.ld7138";

static esp_err_t panel_ld7138_del(esp_lcd_panel_t *panel);
static esp_err_t panel_ld7138_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_ld7138_init(esp_lcd_panel_t *panel);
static esp_err_t panel_ld7138_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end,
                                          const void *color_data);
static esp_err_t panel_ld7138_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_ld7138_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_ld7138_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_ld7138_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_ld7138_disp_on_off(esp_lcd_panel_t *panel, bool on_off);

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    ld7138_handle_t ld7138_handle;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    uint8_t fb_bits_per_pixel;
} ld7138_panel_t;

esp_err_t esp_lcd_new_panel_ld7138(const esp_lcd_panel_io_handle_t io, 
                                   const esp_lcd_panel_dev_config_t *panel_dev_config,
                                   esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    ld7138_panel_t *ld7138_panel = NULL;
    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    
    ld7138_panel = calloc(1, sizeof(ld7138_panel_t));
    ESP_GOTO_ON_FALSE(ld7138_panel, ESP_ERR_NO_MEM, err, TAG, "no mem for ld7138 panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    uint8_t fb_bits_per_pixel = 0;
    switch (panel_dev_config->bits_per_pixel) {
    case 16: // RGB565
        fb_bits_per_pixel = 16;
        break;
    case 18: // RGB666
        fb_bits_per_pixel = 24;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
        break;
    }

    ld7138_panel->io = io;
    ld7138_panel->fb_bits_per_pixel = fb_bits_per_pixel;
    ld7138_panel->reset_gpio_num = panel_dev_config->reset_gpio_num;
    ld7138_panel->reset_level = panel_dev_config->flags.reset_active_high;
    ld7138_panel->base.del = panel_ld7138_del;
    ld7138_panel->base.reset = panel_ld7138_reset;
    ld7138_panel->base.init = panel_ld7138_init;
    ld7138_panel->base.draw_bitmap = panel_ld7138_draw_bitmap;
    ld7138_panel->base.invert_color = panel_ld7138_invert_color;
    ld7138_panel->base.set_gap = panel_ld7138_set_gap;
    ld7138_panel->base.mirror = panel_ld7138_mirror;
    ld7138_panel->base.swap_xy = panel_ld7138_swap_xy;
    ld7138_panel->base.disp_on_off = panel_ld7138_disp_on_off;
    *ret_panel = &(ld7138_panel->base);
    ESP_LOGD(TAG, "new ld7138 panel @%p", ld7138_panel);

    return ESP_OK;

err:
    if (ld7138_panel) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(ld7138_panel);
    }
    return ret;
}

static esp_err_t panel_ld7138_del(esp_lcd_panel_t *panel)
{
    ld7138_panel_t *ld7138_panel = __containerof(panel, ld7138_panel_t, base);

    if (ld7138_panel->ld7138_handle) {
        ld7138_free(ld7138_panel->ld7138_handle);
    }
    if (ld7138_panel->reset_gpio_num >= 0) {
        gpio_reset_pin(ld7138_panel->reset_gpio_num);
    }
    ESP_LOGD(TAG, "del ld7138 panel @%p", ld7138_panel);
    free(ld7138_panel);
    return ESP_OK;
}

static esp_err_t panel_ld7138_reset(esp_lcd_panel_t *panel)
{
    ld7138_panel_t *ld7138_panel = __containerof(panel, ld7138_panel_t, base);

    // perform hardware reset
    if (ld7138_panel->reset_gpio_num >= 0) {
        gpio_set_level(ld7138_panel->reset_gpio_num, ld7138_panel->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(ld7138_panel->reset_gpio_num, !ld7138_panel->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Reset через драйвер LD7138 если он инициализирован
    if (ld7138_panel->ld7138_handle) {
        ESP_RETURN_ON_ERROR(ld7138_reset(ld7138_panel->ld7138_handle), TAG, "ld7138 reset failed");
    }

    return ESP_OK;
}

static esp_err_t panel_ld7138_init(esp_lcd_panel_t *panel)
{
    ld7138_panel_t *ld7138_panel = __containerof(panel, ld7138_panel_t, base);
    esp_lcd_panel_io_handle_t io = ld7138_panel->io;

    // Инициализация LD7138 через команды LCD
    // Выход из режима сна
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x03_DISPLAY_STANDBY_ON_OFF, 
                                                   (uint8_t[]){0x00}, 1), TAG, "exit standby failed");
    vTaskDelay(pdMS_TO_TICKS(10));

    // Включение дисплея
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x02_DISPLAY_ON_OFF, 
                                                   (uint8_t[]){0x01}, 1), TAG, "display on failed");
    vTaskDelay(pdMS_TO_TICKS(10));

    // Настройка направления записи
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x05_WRITE_DIRECTION, 
                                                   (uint8_t[]){0x00}, 1), TAG, "write direction failed");

    // Настройка направления сканирования
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x06_SCAN_DIRECTION, 
                                                   (uint8_t[]){0x00}, 1), TAG, "scan direction failed");

    // Установка окна дисплея
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x07_SET_DISPLAY_WINDOW, 
                                                   (uint8_t[]){0x00, 0x00, LD7138_WIDTH - 1, LD7138_HEIGHT - 1}, 4), 
                                                   TAG, "set display window failed");

    // Настройка RGB режима
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x34_RGB_MODE, 
                                                   (uint8_t[]){0x01}, 1), TAG, "rgb mode failed");

    ESP_LOGI(TAG, "LD7138 panel initialized");
    return ESP_OK;
}

static esp_err_t panel_ld7138_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end,
                                          const void *color_data)
{
    ld7138_panel_t *ld7138_panel = __containerof(panel, ld7138_panel_t, base);
    esp_lcd_panel_io_handle_t io = ld7138_panel->io;

    x_start += ld7138_panel->x_gap;
    x_end += ld7138_panel->x_gap;
    y_start += ld7138_panel->y_gap;
    y_end += ld7138_panel->y_gap;

    // Установка окна данных
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x0A_SET_DATA_WINDOW, (uint8_t[]) {
        x_start & 0xFF,
        y_start & 0xFF,
        (x_end - 1) & 0xFF,
        (y_end - 1) & 0xFF,
    }, 4), TAG, "set data window failed");

    // Установка адреса начала записи
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x0B_SET_ADDRESS, (uint8_t[]) {
        x_start & 0xFF,
        y_start & 0xFF,
    }, 2), TAG, "set address failed");

    // Передача данных
    size_t len = (x_end - x_start) * (y_end - y_start) * ld7138_panel->fb_bits_per_pixel / 8;
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_color(io, LD7138_0x0C_DATA_WRITE_READ, color_data, len), 
                        TAG, "io tx color failed");

    return ESP_OK;
}

static esp_err_t panel_ld7138_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    // LD7138 не поддерживает инверсию цвета через команду
    // Можно реализовать программно если необходимо
    ESP_LOGW(TAG, "Color inversion not supported by LD7138");
    return ESP_OK;
}

static esp_err_t panel_ld7138_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    ld7138_panel_t *ld7138_panel = __containerof(panel, ld7138_panel_t, base);
    esp_lcd_panel_io_handle_t io = ld7138_panel->io;
    
    // Настройка направления сканирования для зеркалирования
    uint8_t scan_dir = 0x00;
    if (mirror_x) {
        scan_dir |= 0x01;  // Зеркалирование по X
    }
    if (mirror_y) {
        scan_dir |= 0x02;  // Зеркалирование по Y
    }
    
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x06_SCAN_DIRECTION, 
                                                   (uint8_t[]){scan_dir}, 1), TAG, "scan direction failed");
    return ESP_OK;
}

static esp_err_t panel_ld7138_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    ld7138_panel_t *ld7138_panel = __containerof(panel, ld7138_panel_t, base);
    esp_lcd_panel_io_handle_t io = ld7138_panel->io;
    
    // Настройка направления записи для обмена осей
    uint8_t write_dir = swap_axes ? 0x01 : 0x00;
    
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x05_WRITE_DIRECTION, 
                                                   (uint8_t[]){write_dir}, 1), TAG, "write direction failed");
    return ESP_OK;
}

static esp_err_t panel_ld7138_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    ld7138_panel_t *ld7138_panel = __containerof(panel, ld7138_panel_t, base);
    ld7138_panel->x_gap = x_gap;
    ld7138_panel->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t panel_ld7138_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    ld7138_panel_t *ld7138_panel = __containerof(panel, ld7138_panel_t, base);
    esp_lcd_panel_io_handle_t io = ld7138_panel->io;
    
    uint8_t value = on_off ? 0x01 : 0x00;
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LD7138_0x02_DISPLAY_ON_OFF, 
                                                   (uint8_t[]){value}, 1), TAG, "display on/off failed");
    return ESP_OK;
}