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

# Переход в директорию unity-app
cd "$(dirname "$0")" || { echo "Ошибка: Не удалось перейти в директорию unity-app"; exit 1; }

# Проверка наличия CMake
if ! command -v cmake &> /dev/null; then
    echo "Ошибка: CMake не найден. Пожалуйста, установите CMake."
    exit 1
fi

# Вывод версии CMake для диагностики
echo "Версия CMake: $(cmake --version)"

# Создание директории сборки, если она не существует
mkdir -p build

# Переход в директорию сборки
cd build || { echo "Ошибка: Не удалось перейти в директорию build"; exit 1; }

# Запуск CMake для генерации файлов сборки с указанием типа сборки
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE .. || { echo "Ошибка: Не удалось сгенерировать файлы сборки с помощью CMake"; exit 1; }

# Сборка проекта
cmake --build . || { echo "Ошибка: Не удалось собрать проект"; exit 1; }

# Проверка успешности сборки
if [ $? -eq 0 ]; then
    echo "Сборка успешно завершена."
else
    echo "Ошибка при сборке проекта."
    exit 1
fi

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
else
    echo "Сборка завершена. Тесты не запускались."
fi