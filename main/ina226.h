#ifndef INA226_H
#define INA226_H

#include <stdint.h>
#include "esp_err.h"

// Конфигурация INA226
typedef struct {
    float shunt_resistance;  // Сопротивление шунта в Омах
    float max_current;       // Максимальный ток в Амперах
} ina226_config_t;

/**
 * @brief Инициализация INA226
 * @param config Указатель на структуру конфигурации
 * @return ESP_OK при успехе, код ошибки при неудаче
 */
esp_err_t ina226_init(const ina226_config_t *config);

/**
 * @brief Чтение значений с датчика
 * @param voltage Указатель для сохранения напряжения (V)
 * @param current Указатель для сохранения тока (A)
 * @param power Указатель для сохранения мощности (W)
 * @return ESP_OK при успехе, код ошибки при неудаче
 */
esp_err_t ina226_read_values(float *voltage, float *current, float *power);

#endif