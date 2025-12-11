# Интеграционные тесты OBD2 и ISO-TP

## Описание

Интеграционные тесты для проверки корректности работы протоколов OBD2 и ISO-TP через CAN интерфейс. Тесты используют библиотеки `python-can` и `python-can-isotp` для взаимодействия с CAN шиной.

## Требования

### Python зависимости

Для запуска тестов необходимо установить следующие Python библиотеки:

```bash
pip install -r requirements.txt
```

### CAN интерфейс

Тесты могут работать с реальным CAN интерфейсом или виртуальным CAN интерфейсом (vcan).

#### Настройка виртуального CAN интерфейса

```bash
# Загрузка модуля vcan
sudo modprobe vcan

# Создание виртуального CAN интерфейса
sudo ip link add dev vcan0 type vcan

# Запуск интерфейса
sudo ip link set up vcan0
```

#### Использование реального CAN интерфейса

Для использования реального CAN интерфейса убедитесь, что он правильно настроен в системе. По умолчанию тесты будут пытаться подключиться к `vcan0`, а при неудаче - к `can0`.

## Структура тестов

- `test_iso_tp_integration.py` - интеграционные тесты для протокола ISO-TP
- `test_obd2_integration.py` - интеграционные тесты для протокола OBD2
- `run_integration_tests.py` - скрипт для запуска всех тестов
- `requirements.txt` - зависимости Python

## Запуск тестов

### Запуск всех тестов

```bash
python run_integration_tests.py
```

### Запуск тестов с подробным выводом

```bash
python run_integration_tests.py --verbose
```

### Запуск отдельного теста

```bash
python run_integration_tests.py --test test_iso_tp_integration.TestIsoTpIntegration.test_single_frame_send_receive
```

### Запуск тестов через unittest

```bash
# Запуск всех тестов ISO-TP
python -m unittest test_iso_tp_integration

# Запуск всех тестов OBD2
python -m unittest test_obd2_integration

# Запуск конкретного теста
python -m unittest test_iso_tp_integration.TestIsoTpIntegration.test_single_frame_send_receive
```

## Описание тестов

### ISO-TP интеграционные тесты

- `test_single_frame_send_receive` - тест отправки и приема одиночных кадров
- `test_long_message_send` - тест отправки длинных сообщений
- `test_flow_control_handling` - тест обработки Flow Control
- `test_iso_tp_protocol_compliance` - тест соответствия протоколу ISO-TP
- `test_error_recovery` - тест восстановления после ошибок
- `test_concurrent_operations` - тест одновременных операций

### OBD2 интеграционные тесты

- `test_pid_00_supported_pids_1_20` - тест запроса поддерживаемых PID 1-20
- `test_pid_01_monitor_status` - тест запроса статуса мониторинга
- `test_pid_04_engine_load` - тест запроса нагрузки двигателя
- `test_pid_05_coolant_temp` - тест запроса температуры охлаждающей жидкости
- `test_pid_0C_engine_rpm` - тест запроса оборотов двигателя
- `test_pid_0D_vehicle_speed` - тест запроса скорости автомобиля
- `test_pid_21_40_range` - тест запроса PID из диапазона 21-40
- `test_pid_41_60_range` - тест запроса PID из диапазона 41-60
- `test_multiple_pid_requests` - тест последовательных запросов нескольких PID
- `test_service_09_requests` - тест запросов сервиса 09
- `test_obd2_protocol_compliance` - тест соответствия протоколу OBD2
- `test_error_handling` - тест обработки ошибок OBD2
- `test_component_interaction` - тест взаимодействия между компонентами
- `test_performance_under_load` - тест производительности под нагрузкой

## Работа с реальным оборудованием

При использовании реального CAN интерфейса тесты будут отправлять реальные запросы на подключенное OBD2 оборудование и ожидать ответы. Убедитесь, что:

1. CAN интерфейс правильно подключен к OBD2 разъему
2. Скорость CAN шины настроена правильно (обычно 500 kbps для OBD2)
3. Оборудование включено и готово к работе

## Особенности работы с виртуальным интерфейсом

При использовании виртуального CAN интерфейса (vcan0) тесты проверяют только корректность отправки сообщений, так как нет реального оборудования для ответа. Это полезно для проверки:

- Корректности формирования CAN кадров
- Работы стека ISO-TP
- Обработки ошибок
- Производительности

## Добавление новых тестов

Для добавления новых тестов:

1. Создайте новый метод в соответствующем классе тестов
2. Используйте префикс `test_` для имени метода
3. Добавьте документацию в docstring
4. Используйте `self._send_obd_request()` для отправки запросов
5. Используйте `self._wait_for_obd_response()` для ожидания ответов

## Отладка

Для отладки тестов можно использовать:

```bash
python run_integration_tests.py --verbose
```

Это покажет подробную информацию о ходе выполнения тестов.