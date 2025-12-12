#!/bin/bash

# Скрипт для сборки проекта unity-app с санитайзерами

# Параметры по умолчанию
BUILD_TYPE="Debug"
SANITIZER=""

# Обработка аргументов командной строки
while [[ $# -gt 0 ]]; do
  case $1 in
    -d|--debug)
      BUILD_TYPE="Debug"
      shift
      ;;
    -r|--release)
      BUILD_TYPE="Release"
      shift
      ;;
    --asan)
      SANITIZER="asan"
      shift
      ;;
    --ubsan)
      SANITIZER="ubsan"
      shift
      ;;
    --tsan)
      SANITIZER="tsan"
      shift
      ;;
    -h|--help)
      echo "Использование: $0 [ОПЦИИ]"
      echo "Опции:"
      echo "  -d, --debug     Сборка с отладочной информацией (по умолчанию)"
      echo "  -r, --release   Release сборка"
      echo "  --asan          Включить AddressSanitizer"
      echo "  --ubsan         Включить UndefinedBehaviorSanitizer"
      echo "  --tsan          Включить ThreadSanitizer"
      echo "  -h, --help      Показать эту справку"
      exit 0
      ;;
    *)
      echo "Неизвестный параметр: $1"
      exit 1
      ;;
  esac
done

# Создание директории сборки
mkdir -p build_sanitizers
cd build_sanitizers

# Генерация файлов сборки с включенным санитайзером
case $SANITIZER in
  asan)
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DENABLE_ASAN=ON ..
    ;;
  ubsan)
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DENABLE_UBSAN=ON ..
    ;;
  tsan)
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DENABLE_TSAN=ON ..
    ;;
  *)
    echo "Не указан санитайзер. Собираем без санитайзеров."
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
    ;;
esac

# Сборка проекта
cmake --build .

# Проверка наличия исполняемого файла
if [ ! -f "unity_app" ]; then
    echo "Исполняемый файл не найден."
    exit 1
fi

echo "Сборка завершена успешно."