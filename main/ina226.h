#ifndef INA226_H
#define INA226_H

#include <stdint.h>
#include "esp_err.h"

/**
 * @brief Конфигурация датчика INA226
 *
 * @note Для точных измерений рекомендуется использовать шунты с низким TCR
 * (температурным коэффициентом сопротивления)
 */
typedef struct {
    float shunt_resistance;  ///< Сопротивление шунта в Ом
    uint8_t max_current;       ///< Максимальный измеряемый ток в Амперах
} ina226_config_t;

/**
 * @brief Инициализация датчика INA226
 *
 * @param config Указатель на структуру конфигурации (не может быть NULL)
 * @return
 *   - ESP_OK: успешная инициализация
 *   - ESP_ERR_INVALID_ARG: неверные параметры конфигурации
 *   - ESP_FAIL: ошибка связи с датчиком
 *
 * @note Перед использованием необходимо настроить I2C интерфейс
 */
void ina226_init(const ina226_config_t *config);

/**
 * @brief Чтение измеренных значений с датчика
 *
 * @param voltage Указатель для сохранения напряжения в вольтах (V)
 * @param current Указатель для сохранения тока в амперах (A)
 * @param power Указатель для сохранения мощности в ваттах (W)
 * @return
 *   - ESP_OK: успешное чтение
 *   - ESP_ERR_INVALID_ARG: нулевые указатели
 *   - ESP_FAIL: ошибка чтения данных
 *
 * @note Для точных измерений рекомендуется усреднять несколько показаний
 *
 * @code
 * float v, i, p;
 * if (ina226_read_values(&v, &i, &p) == ESP_OK) {
 *     printf("V=%.2fV, I=%.2fA, P=%.2fW\n", v, i, p);
 * }
 * @endcode
 */
esp_err_t ina226_read_values(float *voltage, float *current, float *power);

#endif