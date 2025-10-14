#pragma once

#include "lvgl.h"
#include "ssd1283a.h"

/**
 * @brief Инициализация LVGL с дисплеем SSD1283A
 *
 * @param display указатель на объект дисплея SSD1283A
 * @return true если инициализация прошла успешно, иначе false
 */
bool lvgl_port_init(SSD1283A* display);

/**
 * @brief Обновление дисплея (вызывать в цикле)
 */
void lvgl_port_flush_ready();

/**
 * @brief Задержка для LVGL
 *
 * @param delay_ms время задержки в миллисекундах
 */
void lvgl_port_delay(uint32_t delay_ms);

/**
 * @brief Получение тиков для LVGL
 *
 * @return текущее количество тиков
 */
uint32_t lvgl_port_get_tick();

/**
 * @brief Задача обработки LVGL (вызывать в отдельном потоке)
 */
void lvgl_port_task();

/**
 * @brief Создание задачи для LVGL
 */
void lvgl_port_create_task();