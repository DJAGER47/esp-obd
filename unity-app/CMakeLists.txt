cmake_minimum_required(VERSION 3.22)

project(unity_app LANGUAGES C CXX)

add_executable(unity_app
    src/main.cpp
    tests/iso-tp/tests_single.cpp
    tests/iso-tp/tests_long_send.cpp
    tests/iso-tp/tests_edge_cases.cpp
    tests/iso-tp/tests_twai_subscriber_iso_tp.cpp
    
    # Отключаем тесты OBD2, так как они не работают с текущей версией кода
    tests/obd/tests_obd_pid_group_1_20.cpp
    tests/obd/tests_obd_pid_group_21_40.cpp
    tests/obd/tests_obd_pid_group_41_60.cpp
    tests/obd/tests_obd_pid_group_61_80.cpp
    tests/obd/tests_obd_pid_group_81_xx.cpp
    
    ../components/iso-tp/iso_tp.cpp
    ../components/iso-tp/twai_subscriber_iso_tp.cpp
    
    # Отключаем компоненты OBD2, так как они не нужны для тестов ISO-TP
    ../components/obd/obd2_cmd.cpp
    ../components/obd/obd2_pid_1_20.cpp
    ../components/obd/obd2_pid_21_40.cpp
    ../components/obd/obd2_pid_41_60.cpp
    ../components/obd/obd2_pid_61_80.cpp
    ../components/obd/obd2_pid_81_100.cpp
    ../components/obd/obd2_pid_101_120.cpp
    ../components/obd/obd2_pid_121_140.cpp
    ../components/obd/obd2_pid.cpp

    Unity-2.6.1/src/unity.c
)

add_definitions(-DTEST_INSTANCES)

# Установка флагов компиляции в зависимости от типа сборки
target_compile_options(unity_app PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Добавим опции для санитайзеров
option(ENABLE_ASAN "Enable AddressSanitizer" ON)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" ON)
option(ENABLE_LSAN "Enable LeakSanitizer" ON)

# Настройка флагов компиляции для санитайзеров
if(ENABLE_ASAN)
    target_compile_options(unity_app PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(unity_app PRIVATE -fsanitize=address)
endif()

if(ENABLE_UBSAN)
    target_compile_options(unity_app PRIVATE -fsanitize=undefined)
    target_link_options(unity_app PRIVATE -fsanitize=undefined)
endif()

if(ENABLE_LSAN)
    target_compile_options(unity_app PRIVATE -fsanitize=leak)
    target_link_options(unity_app PRIVATE -fsanitize=leak)
endif()

set_property(TARGET unity_app PROPERTY CXX_STANDARD 17)
set_property(TARGET unity_app PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET unity_app PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET unity_app PROPERTY LINKER_LANGUAGE CXX)

# Добавление директорий заголовочных файлов
target_include_directories(unity_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/iso-tp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/obd

    ${CMAKE_CURRENT_SOURCE_DIR}/../components/iso-tp
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/phy_interface
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/obd
    
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
    
    # Включаем поддержку double в Unity
    UNITY_INCLUDE_DOUBLE
    UNITY_DOUBLE_PRECISION=1e-12
)

##################################################################
# Информационные сообщения
##################################################################
message(STATUS "=== Конфигурация Unity ======================================")
message(STATUS "Проект: ${PROJECT_NAME}")
message(STATUS "Тип сборки: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ стандарт: ${CMAKE_CXX_STANDARD}")
message(STATUS "C стандарт: ${CMAKE_C_STANDARD}")
message(STATUS "Компилятор: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "=============================================================")

