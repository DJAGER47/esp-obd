# Запуск тестов под санитайзерами

## Общая информация

Санитайзеры - это инструменты времени выполнения, которые помогают обнаруживать ошибки в программе во время её выполнения. В этом проекте поддерживаются следующие санитайзеры:

1. **AddressSanitizer (ASan)**: Обнаруживает ошибки работы с памятью (утечки, двойное освобождение, выход за границы и т.д.)
2. **UndefinedBehaviorSanitizer (UBSan)**: Обнаруживает неопределенное поведение (переполнение, деление на ноль и т.д.)
3. **ThreadSanitizer (TSan)**: Обнаруживает гонки данных в многопоточных приложениях

## Сборка с санитайзерами

Для сборки проекта с санитайзерами используется скрипт `build_with_sanitizers.sh`:

```bash
# Сборка с AddressSanitizer
./build_with_sanitizers.sh --asan

# Сборка с UndefinedBehaviorSanitizer
./build_with_sanitizers.sh --ubsan

# Сборка с ThreadSanitizer
./build_with_sanitizers.sh --tsan
```

Также можно комбинировать с опциями сборки:

```bash
# Debug сборка с AddressSanitizer
./build_with_sanitizers.sh --debug --asan

# Release сборка с UndefinedBehaviorSanitizer
./build_with_sanitizers.sh --release --ubsan
```

## Запуск тестов с санитайзерами

После сборки с санитайзерами, тесты можно запустить следующим образом:

```bash
# Запуск тестов с AddressSanitizer
./build_with_sanitizers.sh --asan && ./build_sanitizers/unity_app

# Запуск тестов с UndefinedBehaviorSanitizer
./build_with_sanitizers.sh --ubsan && ./build_sanitizers/unity_app

# Запуск тестов с ThreadSanitizer
./build_with_sanitizers.sh --tsan && ./build_sanitizers/unity_app
```

## Настройка переменных окружения

Для некоторых санитайзеров может потребоваться настройка переменных окружения:

```bash
# Для AddressSanitizer
export ASAN_OPTIONS=detect_leaks=1:abort_on_error=1

# Для UndefinedBehaviorSanitizer
export UBSAN_OPTIONS=print_stacktrace=1

# Для ThreadSanitizer
export TSAN_OPTIONS=halt_on_error=1
```

## Анализ результатов

При запуске тестов с санитайзерами, они будут выводить информацию об обнаруженных ошибках в stderr. В случае обнаружения ошибок, санитайзеры обычно аварийно завершают программу с подробным отчетом об ошибке.

Пример вывода при обнаружении утечки памяти:

```
=================================================================
==12345==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 1024 byte(s) in 1 object(s) allocated from:
    #0 0x... in malloc
    #1 0x... in some_function src/some_file.cpp:123:15

SUMMARY: AddressSanitizer: 1024 byte(s) leaked in 1 allocation(s).
```

## Рекомендации по использованию

1. **AddressSanitizer (ASan)**: Рекомендуется использовать при разработке для обнаружения ошибок работы с памятью.

2. **UndefinedBehaviorSanitizer (UBSan)**: Полезен для поиска скрытых ошибок неопределенного поведения.

3. **ThreadSanitizer (TSan)**: Может быть полезен при тестировании многозадачных компонентов, хотя в текущей версии проекта многопоточность ограничена из-за использования mock-объектов.

## Известные ограничения

1. ThreadSanitizer может давать ложные срабатывания при использовании с некоторыми библиотеками.
2. Санитайзеры могут значительно замедлить выполнение программы.
3. Некоторые санитайзеры несовместимы друг с другом (например, ASan и TSan нельзя использовать одновременно).