#pragma once

#include "ld7138.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Инициализация дисплея LD7138 с интеграцией в LVGL
 *
 * @param config Конфигурация дисплея
 * @param disp_handle Указатель для сохранения дескриптора дисплея LVGL
 * @param ld7138_handle Указатель для сохранения дескриптора LD7138
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_lvgl_init(const ld7138_config_t *config, lv_display_t **disp_handle, ld7138_handle_t *ld7138_handle);

/**
 * @brief Деинициализация дисплея LD7138 с интеграцией в LVGL
 *
 * @param disp_handle Дескриптор дисплея LVGL
 * @param ld7138_handle Дескриптор LD7138
 * @return esp_err_t ESP_OK в случае успеха
 */
esp_err_t ld7138_lvgl_free(lv_display_t *disp_handle, ld7138_handle_t ld7138_handle);

#ifdef __cplusplus
}
#endif