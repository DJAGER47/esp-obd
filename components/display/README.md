# Компонент Display для SSD1283A

Этот компонент предоставляет драйвер для цветного OLED дисплея SSD1283A с поддержкой LVGL.

## Особенности

- Поддержка цветного дисплея SSD1283A (130x130 пикселей, 16-битный цвет RGB565)
- Интеграция с библиотекой LVGL для создания графических интерфейсов
- Работа по SPI интерфейсу
- Поддержка базовых графических примитивов

## Подключение

### Аппаратное подключение

| Пин ESP32 | Пин SSD1283A | Описание |
|-----------|--------------|----------|
| GPIO23    | MOSI         | Последовательные данные |
| GPIO18    | SCLK         | Тактовый сигнал |
| GPIO5     | CS           | Выбор кристалла |
| GPIO4     | DC           | Data/Command |
| GPIO22    | RST          | Сброс |

### Использование в коде

```cpp
#include "ssd1283a.h"
#include "lvgl_port.h"

// Создание экземпляра дисплея
SSD1283A display(GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_22, SPI2_HOST);

// Инициализация
if (!display.init()) {
    ESP_LOGE("APP", "Failed to initialize display");
    return;
}

// Инициализация LVGL
if (!lvgl_port_init(&display)) {
    ESP_LOGE("APP", "Failed to initialize LVGL");
    return;
}

// Создание задачи LVGL
lvgl_port_create_task();
```

## API

### Класс SSD1283A

#### Основные методы

- `bool init()` - Инициализация дисплея
- `void clear()` - Очистка дисплея
- `void clear(uint16_t color)` - Очистка дисплея указанным цветом
- `void display()` - Отображение буфера на дисплее
- `void on()` - Включение дисплея
- `void off()` - Выключение дисплея
- `void setContrast(uint8_t contrast)` - Установка контрастности

#### Графические примитивы

- `void drawPixel(int16_t x, int16_t y, uint16_t color)` - Установка пикселя
- `void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)` - Рисование линии
- `void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)` - Рисование прямоугольника
- `void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)` - Заполнение прямоугольника

#### Получение информации

- `uint16_t* getBuffer()` - Получение указателя на буфер дисплея
- `int16_t getWidth()` - Получение ширины дисплея
- `int16_t getHeight()` - Получение высоты дисплея

### LVGL Port

#### Функции

- `bool lvgl_port_init(SSD1283A* display)` - Инициализация LVGL с дисплеем
- `void lvgl_port_task()` - Обработка задач LVGL (вызывать в цикле)
- `void lvgl_port_create_task()` - Создание задачи для LVGL

## Цвета

Дисплей использует 16-битный формат цвета RGB565:
- 5 бит для красного канала
- 6 бит для зеленого канала
- 5 бит для синего канала

Примеры основных цветов:
- Черный: `0x0000`
- Белый: `0xFFFF`
- Красный: `0xF800`
- Зеленый: `0x07E0`
- Синий: `0x001F`

## Пример использования с LVGL

```cpp
// Создание простого интерфейса
lv_obj_t * scr = lv_scr_act();

// Создание метки
lv_obj_t * label = lv_label_create(scr);
lv_label_set_text(label, "Hello World!");
lv_obj_set_style_text_color(label, lv_color_hex(0x00ff00), LV_PART_MAIN);
lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

// Создание кнопки
lv_obj_t * btn = lv_btn_create(scr);
lv_obj_set_size(btn, 80, 40);
lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);

lv_obj_t * btn_label = lv_label_create(btn);
lv_label_set_text(btn_label, "Button");
lv_obj_center(btn_label);
```

## Зависимости

- ESP-IDF driver
- LVGL
- FreeRTOS

## Сборка

Компонент автоматически включается в сборку при добавлении зависимости `display` в CMakeLists.txt вашего проекта.