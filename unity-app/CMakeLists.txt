cmake_minimum_required(VERSION 3.22)
project(unity_app)

##################################################################
# Объединенный CMakeLists.txt для unity-app
# Содержит все настройки компонентов и тестов в одном файле
# Все объединено в один исполняемый файл
##################################################################

# Создание исполняемого файла, включающего все исходные файлы
add_executable(unity_app
    # Основной файл
    src/main.cpp
    
    # Файлы Unity
    Unity-2.6.1/src/unity.c
    
    # Файлы компонента iso-tp
    ../components/iso-tp/iso-tp.cpp
    
    # Тестовые файлы
    tests/iso-tp/tests_single.cpp
    tests/iso-tp/tests_long_send.cpp
)

# Установка флагов компиляции в зависимости от типа сборки
target_compile_options(unity_app PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Установка свойств компиляции для исполняемого файла
set_target_properties(unity_app PROPERTIES
    CXX_STANDARD 17
    C_STANDARD 99
    CXX_STANDARD_REQUIRED ON
    C_STANDARD_REQUIRED ON
)

# Добавление директорий заголовочных файлов
target_include_directories(unity_app PRIVATE
    # Для основного приложения
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    
    # Для Unity
    ${CMAKE_CURRENT_SOURCE_DIR}/Unity-2.6.1/src
    
    # Для компонента iso-tp
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/iso-tp
    
    # Для twai_interface (используется в iso-tp)
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/twai_interface
    
    # Для моков
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/freertos
    
    # Для тестов
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/iso-tp
)

# Определение макросов препроцессора
target_compile_definitions(unity_app PRIVATE
    # Отключаем реальную реализацию ESP_LOG
    ESP_LOG_H_
    
    # Отключаем реальную реализацию FreeRTOS
    FREERTOS_H
    TASK_H
)

##################################################################
# Информационные сообщения
##################################################################
message(STATUS "=== Конфигурация Unity App (объединенный исполняемый файл) ===")
message(STATUS "Проект: ${PROJECT_NAME}")
message(STATUS "Тип сборки: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ стандарт: ${CMAKE_CXX_STANDARD}")
message(STATUS "C стандарт: ${CMAKE_C_STANDARD}")
message(STATUS "Компилятор: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "=============================================================")