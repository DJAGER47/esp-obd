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

## Как использовать проект

(Сохраняется существующая информация из раздела "How to Use Example")

### Конфигурация проекта
В меню `Example Configuration`:
1. Выберите тип LED:
   - `GPIO` для обычного светодиода
   - `LED strip` для адресной ленты
2. Укажите GPIO для LED (по умолчанию 2)
3. Установите период мигания (мс)

## Пример вывода
Проект выводит показания INA226:
```text
I (315) example: Voltage: 3.30V, Current: 0.12A, Power: 0.40W
I (1325) example: Turning the LED ON!
I (2325) example: Voltage: 3.28V, Current: 0.15A, Power: 0.49W
I (3325) example: Turning the LED OFF!
```

## Troubleshooting
- Если LED не мигает, проверьте:
  - Правильность подключения
  - Настройки GPIO в menuconfig
- Если INA226 не работает, проверьте:
  - Подключение I2C (SCL/SDA)
  - Питание датчика (3.3V)
