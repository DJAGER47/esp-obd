# Blink Example with INA226 Monitoring and X9C103S Control

## Описание платы ESP32-C3 SuperMini
ESP32-C3 SuperMini - компактная плата на базе микроконтроллера ESP32-C3 с WiFi и Bluetooth LE. Основные характеристики:
- Процессор: 32-битный RISC-V, 160 МГц
- Память: 400KB SRAM, 4MB Flash
- Интерфейсы: GPIO, I2C, SPI, UART, PWM

### Распиновка и свободные ножки:

Используемые:
- GPIO6  - X9C103S U/D
- GPIO7  - X9C103S CS
- GPIO8  - I2C SCL (INA226)
- GPIO9  - X9C103S INC
- GPIO10 - I2C SDA (INA226)

Свободные GPIO с функциями:
- GPIO0  - Boot mode, ADC1, Strapping
- GPIO1  - UART TX, ADC1
- GPIO2  - LED (настраивается), ADC1
- GPIO3  - UART RX, ADC1
- GPIO4  - ADC1
- GPIO5  - ADC2 (конфликт с Wi-Fi)
- GPIO11 - SPI CS, PWM
- GPIO12 - SPI MISO, PWM
- GPIO13 - SPI MOSI, PWM
- GPIO14 - SPI CLK, PWM
- GPIO15 - PWM
- GPIO16 - PWM
- GPIO17 - PWM
- GPIO18 - USB DP
- GPIO19 - USB DM
- GPIO20 - PWM
- GPIO21 - PWM

Примечания:
- GPIO0-4: АЦП (12-бит)
- GPIO5: АЦП только при отключенном Wi-Fi
- GPIO11-14: SPI2 интерфейс
- GPIO15-21: ШИМ (8 каналов)
- GPIO0  - Boot mode
- GPIO8  - I2C SCL (подключен к INA226)
- GPIO10 - I2C SDA (подключен к INA226)
- GPIO2  - LED (настраивается через menuconfig)
- GPIO3  - UART TX
- GPIO4  - UART RX
- GPIO5  - USB D-
- GPIO6  - USB D+
- GPIO18 - USB DP
- GPIO19 - USB DM

## Подключенная периферия

### Датчик INA226 (мониторинг питания)
- Адрес I2C: 0x40
- SCL: GPIO8
- SDA: GPIO10
- Измеряет:
  - Напряжение (до 36V)
  - Ток (зависит от шунта)
  - Мощность

### Цифровой потенциометр X9C103S
- CS:   GPIO7
- U/D:  GPIO6
- INC:  GPIO5
- Диапазон: 0-99 ступеней
- Управление: через драйвер x9c103s

### LED
- GPIO настраивается через menuconfig (по умолчанию GPIO2)
- Поддерживаются:
  - Обычный LED (режим GPIO)
  - Адресные LED (WS2812 через библиотеку led_strip)

## Схема подключения

```
ESP32-C3 SuperMini:
  GPIO9  --- X9C103S INC
  GPIO6  --- X9C103S U/D
  GPIO7  --- X9C103S CS
  GPIO8 (SCL) --- INA226 SCL
  GPIO10 (SDA) --- INA226 SDA
  GPIO2 (LED) --- LED (анод)
  GND --- INA226 GND и LED (катод)
  VCC (3.3V) --- INA226 VCC
```

## Архитектура проекта

Проект состоит из следующих ключевых компонентов:

1. **Драйвер INA226** (main/ina226.h, main/ina226_new.c):
   - Поддержка двух реализаций (legacy/new)
   - Измерение напряжения, тока и мощности
   - Конфигурируемые параметры шунта

2. **Драйвер X9C103S** (main/x9c103s.h):
   - Управление цифровым потенциометром
   - Поддержка 100 ступеней регулировки
   - Защита от переполнения

3. **Основной цикл** (main/main.c):
   - Инициализация периферии
   - Цикл измерения и управления
   - Логирование показаний

## Как использовать проект

### Конфигурация проекта
В меню `Example Configuration`:
1. Выберите тип LED:
   - `GPIO` для обычного светодиода
   - `LED strip` для адресной ленты
2. Укажите GPIO для LED (по умолчанию 2)
3. Установите период мигания (мс)
4. Выберите драйвер INA226:
   - `Legacy driver` - оригинальная реализация на driver/i2c.h
   - `New driver` - современная реализация на i2c_master.h (рекомендуется)

### Выбор драйвера INA226
Проект поддерживает две реализации драйвера INA226:

1. **Legacy driver** (по умолчанию отключен):
   - Использует устаревший driver/i2c.h
   - Подходит для обратной совместимости
   - Ограниченная функциональность

2. **New driver** (рекомендуется):
   - Использует современный i2c_master.h
   - Полная документация API в ina226.h
   - Подробные комментарии к коду
   - Поддержка всех функций ESP-IDF
   - Расширенная обработка ошибок
   - Примеры использования:
     ```c
     // Инициализация
     ina226_config_t config = {
         .i2c_port = I2C_NUM_0,
         .i2c_addr = 0x40,
         .shunt_resistor = 0.1, // Ом
         .max_current = 2.0     // А
     };
     ina226_handle_t dev = ina226_init(&config);
     
     // Чтение показаний
     float voltage, current, power;
     ina226_get_voltage(dev, &voltage);
     ina226_get_current(dev, &current);
     ina226_get_power(dev, &power);
     ```

Для переключения между драйверами:
1. Откройте `make menuconfig`
2. Перейдите в `Component config` -> `Example Configuration`
3. Выберите нужный драйвер
4. Сохраните конфигурацию и пересоберите проект

## Примеры работы

### Вывод в лог
```text
I (315) example: Voltage: 3.30V, Current: 0.12A, Power: 0.40W
I (1325) example: Turning the LED ON!
I (2325) example: Voltage: 3.28V, Current: 0.15A, Power: 0.49W
I (3325) example: Turning the LED OFF!
```

### Обработка ошибок
При возникновении ошибок I2C драйвер возвращает соответствующие коды:
```c
esp_err_t err = ina226_get_voltage(dev, &voltage);
if (err != ESP_OK) {
    ESP_LOGE(TAG, "Ошибка чтения напряжения: %s", esp_err_to_name(err));
    // Дополнительная обработка ошибки
}
```

### Настройка потенциометра
```c
x9c103s_set_resistance(dev, 50); // Установка на 50% диапазона
```

## Troubleshooting
- Если LED не мигает, проверьте:
  - Правильность подключения
  - Настройки GPIO в menuconfig
- Если INA226 не работает, проверьте:
  - Подключение I2C (SCL/SDA)
  - Питание датчика (3.3V)
