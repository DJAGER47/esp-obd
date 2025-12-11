#!/bin/bash

# Скрипт для установки зависимостей и запуска интеграционных тестов

set -e  # Завершить выполнение при ошибке

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Интеграционные тесты OBD2 ===${NC}"

# Проверка наличия Python
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}Ошибка: Python 3 не найден${NC}"
    exit 1
fi

echo -e "${YELLOW}Проверка Python версии...${NC}"
python3 --version

# Переход в директорию скрипта
cd "$(dirname "$0")"

# Установка зависимостей
echo -e "${YELLOW}Установка зависимостей Python...${NC}"
if [ -f "requirements.txt" ]; then
    pip install -r requirements.txt
else
    echo -e "${YELLOW}Установка основных зависимостей...${NC}"
    pip install python-can python-can-isotp
fi

# Проверка доступности CAN интерфейса
echo -e "${YELLOW}Проверка CAN интерфейса...${NC}"
CAN_AVAILABLE=false

# Проверка vcan0
if ip link show vcan0 &> /dev/null; then
    echo -e "${GREEN}Найден виртуальный CAN интерфейс vcan0${NC}"
    CAN_AVAILABLE=true
# Проверка can0
elif ip link show can0 &> /dev/null; then
    echo -e "${GREEN}Найден CAN интерфейс can0${NC}"
    CAN_AVAILABLE=true
else
    echo -e "${YELLOW}CAN интерфейс не найден. Создание виртуального интерфейса...${NC}"
    # Попытка создать виртуальный CAN интерфейс
    if sudo modprobe vcan 2>/dev/null && sudo ip link add dev vcan0 type vcan 2>/dev/null && sudo ip link set up vcan0 2>/dev/null; then
        echo -e "${GREEN}Виртуальный CAN интерфейс vcan0 успешно создан${NC}"
        CAN_AVAILABLE=true
    else
        echo -e "${YELLOW}Не удалось создать виртуальный CAN интерфейс. Тесты будут запущены без реального CAN.${NC}"
    fi
fi

# Запуск тестов
echo -e "${YELLOW}Запуск интеграционных тестов...${NC}"

if [ "$CAN_AVAILABLE" = true ]; then
    echo -e "${GREEN}Запуск тестов с доступным CAN интерфейсом${NC}"
    python3 run_integration_tests.py --verbose
else
    echo -e "${YELLOW}Запуск тестов без CAN интерфейса (только проверка синтаксиса)${NC}"
    # Запуск тестов с минимальным выводом для проверки синтаксиса
    python3 -m py_compile test_iso_tp_integration.py test_obd2_integration.py run_integration_tests.py
    echo -e "${GREEN}Синтаксическая проверка пройдена успешно${NC}"
fi

echo -e "${GREEN}=== Завершено ===${NC}"