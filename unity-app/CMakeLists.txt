cmake_minimum_required(VERSION 3.22)

project(unity_app)

add_executable(unity_app
    src/main.cpp
    tests/iso-tp/tests_single.cpp
    tests/iso-tp/tests_long_send.cpp
    
    ../components/iso-tp/iso-tp.cpp

    Unity-2.6.1/src/unity.c
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/iso-tp

    ${CMAKE_CURRENT_SOURCE_DIR}/../components/iso-tp
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/twai_interface
    
    
    ${CMAKE_CURRENT_SOURCE_DIR}/Unity-2.6.1/src
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/freertos
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

##################################################################
# Clang-tidy
##################################################################
option(ENABLE_CLANG_TIDY "Включить проверку clang-tidy" ON)


if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

    if(CLANG_TIDY_EXE)
        message(STATUS "Clang-Tidy включен для проверки кода")
        message(STATUS "CLANG_TIDY_EXE = ${CLANG_TIDY_EXE}")
        message(STATUS "CLANG_TIDY_CONFIG_FILE = ${CMAKE_SOURCE_DIR}/.clang-tidy")
        
        # Настройка параметров clang-tidy
        set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY_EXE}
            -config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
            -format-style='file'
            --use-color
            -extra-arg=-std=c++17
            -extra-arg=-Wno-unknown-warning-option
        )
        
        # Применяем clang-tidy к целевому исполняемому файлу
        set_target_properties(unity_app PROPERTIES CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
        
        # Для C файлов (Unity)
        set(CMAKE_C_CLANG_TIDY
            ${CLANG_TIDY_EXE}
            -config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
            -format-style='file'
            --use-color
            -extra-arg=-std=c99
            -extra-arg=-Wno-unknown-warning-option
        )
        
        set_target_properties(unity_app PROPERTIES C_CLANG_TIDY "${CMAKE_C_CLANG_TIDY}")
    else()
        message(WARNING "clang-tidy не найден. Проверка будет пропущена.")
    endif()
endif()
