#!/usr/bin/env python3
# Тестовый скрипт для проверки импорта модуля isotp

try:
    import isotp
    print("✓ Модуль isotp успешно импортирован")
    print(f"Версия модуля: {isotp.__version__ if hasattr(isotp, '__version__') else 'неизвестна'}")
    
    # Проверяем доступность основных классов
    print("✓ Доступные классы и функции:")
    print(f"  - isotp.socket: {hasattr(isotp, 'socket')}")
    print(f"  - isotp.Address: {hasattr(isotp, 'Address')}")
    
    # Пытаемся создать сокет (без привязки к интерфейсу)
    try:
        sock = isotp.socket()
        print("✓ Успешно создан ISO-TP сокет")
    except Exception as e:
        print(f"✗ Ошибка при создании сокета: {e}")
        
except ImportError as e:
    print(f"✗ Ошибка импорта модуля isotp: {e}")
except Exception as e:
    print(f"✗ Непредвиденная ошибка: {e}")