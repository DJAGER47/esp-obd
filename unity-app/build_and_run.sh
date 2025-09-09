#!/bin/bash

# Скрипт для сборки проекта unity-app и запуска тестов

# Параметры по умолчанию
BUILD_TYPE="Release"
RUN_TESTS=true

# Обработка аргументов командной строки
while [[ $# -gt 0 ]]; do
  case $1 in
    -d|--debug)
      BUILD_TYPE="Debug"
      shift
      ;;
    -n|--no-tests)
      RUN_TESTS=false
      shift
      ;;
    -h|--help)
      echo "Использование: $0 [ОПЦИИ]"
      echo "Опции:"
      echo "  -d, --debug     Сборка с отладочной информацией"
      echo "  -n, --no-tests  Не запускать тесты после сборки"
      echo "  -h, --help      Показать эту справку"
      exit 0
      ;;
    *)
      echo "Неизвестный параметр: $1"
      exit 1
      ;;
  esac
done

cd build
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE .. || { echo "Ошибка: Не удалось сгенерировать файлы сборки с помощью CMake"; exit 1; }
cmake --build . || { echo "Ошибка: Не удалось собрать проект"; exit 1; }

# Проверка наличия исполняемого файла
if [ ! -f "unity_app" ]; then
    echo "Исполняемый файл не найден."
    exit 1
fi

# Запуск тестов, если это требуется
if [ "$RUN_TESTS" = true ]; then
    echo "Запуск тестов..."
    ./unity_app

    # Проверка успешности выполнения тестов
    if [ $? -eq 0 ]; then
        echo "Тесты успешно завершены."
    else
        echo "Ошибка при выполнении тестов."
        exit 1
    fi
fi