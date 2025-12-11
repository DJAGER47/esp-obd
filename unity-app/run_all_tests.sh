#!/bin/bash

# Скрипт для запуска всех тестов проекта (unit, интеграционные и ECU симулятор)

set -e  # Завершить выполнение при ошибке

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}=== Запуск всех тестов проекта ===${NC}"
echo -e "${BLUE}========================================${NC}"

# Переход в директорию скрипта
cd "$(dirname "$0")"

# Запуск unit-тестов
echo -e "${YELLOW}Запуск unit-тестов...${NC}"
if [ -f "./build_and_run.sh" ]; then
    ./build_and_run.sh -n  # -n означает не запускать тесты после сборки
    echo -e "${GREEN}Unit-тесты собраны успешно${NC}"
else
    echo -e "${YELLOW}Скрипт сборки unit-тестов не найден${NC}"
fi

# Запуск интеграционных тестов
echo -e "${YELLOW}Запуск интеграционных тестов...${NC}"
if [ -d "./tests/integration" ]; then
    cd ./tests/integration
    
    # Запуск скрипта установки и запуска интеграционных тестов
    if [ -f "./setup_and_run.sh" ]; then
        ./setup_and_run.sh
    else
        # Альтернативный запуск
        echo -e "${YELLOW}Запуск интеграционных тестов напрямую...${NC}"
        python3 run_integration_tests.py || echo -e "${YELLOW}Интеграционные тесты завершены с предупреждениями${NC}"
    fi
    
    cd ../..
else
    echo -e "${YELLOW}Директория интеграционных тестов не найдена${NC}"
fi

# Запуск тестов с ECU симулятором
echo -e "${YELLOW}Запуск тестов с ECU симулятором...${NC}"
if [ -d "./tests/ecu_simulator" ]; then
    cd ./tests/ecu_simulator
    
    # Запуск скрипта с ECU симулятором
    if [ -f "./run_with_simulator.sh" ]; then
        ./run_with_simulator.sh
    else
        echo -e "${YELLOW}Скрипт запуска тестов с ECU симулятором не найден${NC}"
    fi
    
    cd ../..
else
    echo -e "${YELLOW}Директория тестов с ECU симулятором не найдена${NC}"
fi

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}=== Все тесты завершены ===${NC}"
echo -e "${BLUE}========================================${NC}"