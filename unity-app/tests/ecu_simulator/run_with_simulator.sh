#!/bin/bash

# Скрипт для запуска ECU симулятора и интеграционных тестов

set -e  # Завершить выполнение при ошибке

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}====================================${NC}"
echo -e "${BLUE}=== Запуск ECU симулятора и тестов ===${NC}"
echo -e "${BLUE}====================================${NC}"

# Переход в директорию скрипта
cd "$(dirname "$0")"

# Проверка наличия ECU симулятора
if [ ! -d "ecu-simulator" ]; then
    echo -e "${RED}Ошибка: Директория ecu-simulator не найдена${NC}"
    exit 1
fi

# Проверка наличия Python
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}Ошибка: Python 3 не найден${NC}"
    exit 1
fi

# Установка зависимостей
echo -e "${YELLOW}Установка зависимостей Python...${NC}"
pip install python-can can-isotp

# Настройка виртуального CAN интерфейса
echo -e "${YELLOW}Настройка виртуального CAN интерфейса...${NC}"
if ! sudo modprobe vcan 2>/dev/null; then
    echo -e "${RED}Ошибка: Не удалось загрузить модуль vcan${NC}"
    exit 1
fi

if ! sudo ip link add dev vcan0 type vcan 2>/dev/null; then
    echo -e "${YELLOW}Интерфейс vcan0 уже существует${NC}"
else
    echo -e "${GREEN}Создан виртуальный CAN интерфейс vcan0${NC}"
fi

if ! sudo ip link set up vcan0 2>/dev/null; then
    echo -e "${RED}Ошибка: Не удалось запустить интерфейс vcan0${NC}"
    exit 1
else
    echo -e "${GREEN}Интерфейс vcan0 запущен${NC}"
fi

# Запуск ECU симулятора в фоновом режиме
echo -e "${YELLOW}Запуск ECU симулятора...${NC}"
cd ecu-simulator

# Обновление конфигурации для использования vcan0
if [ -f "ecu_config.json" ]; then
    echo -e "${YELLOW}Обновление конфигурации для использования vcan0...${NC}"
    # Создание временной конфигурации с vcan0
    python3 -c "
import json
import os
config_file = 'ecu_config.json'
if os.path.exists(config_file):
    with open(config_file, 'r') as f:
        config = json.load(f)
    config['can_interface']['value'] = 'vcan0'
    config['can_interface_type']['value'] = 'virtual'
    with open(config_file, 'w') as f:
        json.dump(config, f, indent=2)
    print('Конфигурация обновлена для использования vcan0')
"
fi

# Запуск симулятора в фоновом режиме
SIMULATOR_PID_FILE="/tmp/ecu_simulator.pid"
sudo python3 ecu_simulator.py > ecu_simulator_test.log 2>&1 &
SIMULATOR_PID=$!
echo $SIMULATOR_PID > $SIMULATOR_PID_FILE

# Ожидание запуска симулятора
echo -e "${YELLOW}Ожидание запуска ECU симулятора...${NC}"
sleep 3

# Проверка, что симулятор запущен
if kill -0 $SIMULATOR_PID 2>/dev/null; then
    echo -e "${GREEN}ECU симулятор запущен (PID: $SIMULATOR_PID)${NC}"
else
    echo -e "${RED}Ошибка: Не удалось запустить ECU симулятор${NC}"
    # Вывод лога ошибок
    if [ -f "ecu_simulator_test.log" ]; then
        echo -e "${YELLOW}Лог ошибок:${NC}"
        tail -20 ecu_simulator_test.log
    fi
    exit 1
fi

# Возврат в директорию тестов
cd ..

# Запуск интеграционных тестов
echo -e "${YELLOW}Запуск интеграционных тестов...${NC}"
TEST_RESULT=0
python3 run_ecu_simulator_tests.py --verbose || TEST_RESULT=1

# Остановка ECU симулятора
echo -e "${YELLOW}Остановка ECU симулятора...${NC}"
if [ -f "$SIMULATOR_PID_FILE" ]; then
    SIMULATOR_PID=$(cat $SIMULATOR_PID_FILE)
    if kill -0 $SIMULATOR_PID 2>/dev/null; then
        kill $SIMULATOR_PID
        sleep 2
        if kill -0 $SIMULATOR_PID 2>/dev/null; then
            kill -9 $SIMULATOR_PID 2>/dev/null || true
        fi
    fi
    rm -f $SIMULATOR_PID_FILE
fi

echo -e "${BLUE}====================================${NC}"
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}=== Все тесты пройдены успешно! ===${NC}"
else
    echo -e "${RED}=== Некоторые тесты завершились с ошибками! ===${NC}"
fi
echo -e "${BLUE}====================================${NC}"

exit $TEST_RESULT