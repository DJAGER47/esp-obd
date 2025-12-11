#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Скрипт для запуска интеграционных тестов OBD2 и ISO-TP
"""

import unittest
import sys
import os
import argparse
import time

# Добавляем путь к тестам в sys.path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

def run_tests(test_pattern=None, verbosity=2):
    """Запуск интеграционных тестов"""
    
    # Создание тестового набора
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    
    # Если указан паттерн, запускаем только соответствующие тесты
    if test_pattern:
        # Загрузка тестов по паттерну
        suite = loader.loadTestsFromName(test_pattern)
    else:
        # Загрузка всех тестов
        iso_tp_tests = loader.loadTestsFromName('test_iso_tp_integration')
        obd2_tests = loader.loadTestsFromName('test_obd2_integration')
        suite.addTests(iso_tp_tests)
        suite.addTests(obd2_tests)
    
    # Создание раннера и запуск тестов
    runner = unittest.TextTestRunner(verbosity=verbosity)
    result = runner.run(suite)
    
    return result.wasSuccessful()

def check_prerequisites():
    """Проверка необходимых компонентов"""
    try:
        import can
        import isotp
    except ImportError as e:
        print(f"Ошибка импорта: {e}")
        print("Установите необходимые зависимости:")
        print("pip install -r requirements.txt")
        return False
    
    # Проверка доступности SocketCAN
    try:
        bus = can.interface.Bus(channel='vcan0', bustype='socketcan')
        bus.shutdown()
    except Exception as e:
        print(f"Предупреждение: Не удалось подключиться к vcan0: {e}")
        print("Убедитесь, что виртуальный CAN интерфейс настроен:")
        print("sudo modprobe vcan")
        print("sudo ip link add dev vcan0 type vcan")
        print("sudo ip link set up vcan0")
    
    return True

def main():
    """Основная функция"""
    parser = argparse.ArgumentParser(description='Запуск интеграционных тестов OBD2 и ISO-TP')
    parser.add_argument('--test', '-t', help='Запустить только указанный тест (например, test_iso_tp_integration.TestIsoTpIntegration.test_single_frame_send_receive)')
    parser.add_argument('--verbose', '-v', action='store_true', help='Подробный вывод')
    parser.add_argument('--quiet', '-q', action='store_true', help='Минимальный вывод')
    
    args = parser.parse_args()
    
    # Проверка предварительных требований
    if not check_prerequisites():
        return 1
    
    # Определение уровня детализации
    if args.quiet:
        verbosity = 0
    elif args.verbose:
        verbosity = 2
    else:
        verbosity = 1
    
    print("Запуск интеграционных тестов...")
    print("=" * 50)
    
    # Запуск тестов
    start_time = time.time()
    success = run_tests(args.test, verbosity)
    end_time = time.time()
    
    print("=" * 50)
    print(f"Тесты завершены за {end_time - start_time:.2f} секунд")
    
    if success:
        print("Все тесты пройдены успешно!")
        return 0
    else:
        print("Некоторые тесты завершились с ошибками!")
        return 1

if __name__ == '__main__':
    sys.exit(main())