cmake_minimum_required(VERSION 3.22)

project(unity_app LANGUAGES C CXX)

add_executable(unity_app
    src/main.cpp
    tests/iso-tp/tests_single.cpp
    tests/iso-tp/tests_long_send.cpp
    tests/iso-tp/tests_edge_cases.cpp
    
    tests/obd/tests_obd_core_methods.cpp
    tests/obd/tests_obd_pid_group_1_20.cpp
    tests/obd/tests_obd_pid_group_21_40.cpp
    tests/obd/tests_obd_pid_group_41_60.cpp
    tests/obd/tests_obd_pid_group_61_80.cpp
    
    ../components/iso-tp/iso_tp.cpp
    ../components/obd/obd2_cmd.cpp
    ../components/obd/obd2_pid_1_20.cpp
    ../components/obd/obd2_pid_21_40.cpp
    ../components/obd/obd2_pid_41_60.cpp
    ../components/obd/obd2_pid_61_80.cpp
    ../components/obd/obd2_pid.cpp

    Unity-2.6.1/src/unity.c
)

# Установка флагов компиляции в зависимости от типа сборки
target_compile_options(unity_app PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

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

##################################################################
# Clang-tidy
##################################################################
# option(ENABLE_CLANG_TIDY "Включить проверку clang-tidy" OFF)


# if(ENABLE_CLANG_TIDY)
#     find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

#     if(CLANG_TIDY_EXE)
#         message(STATUS "Clang-Tidy включен для проверки кода")
#         message(STATUS "CLANG_TIDY_EXE = ${CLANG_TIDY_EXE}")
#         message(STATUS "CLANG_TIDY_CONFIG_FILE = ${CMAKE_SOURCE_DIR}/.clang-tidy")
        
#         # Настройка параметров clang-tidy
#         set(CMAKE_CXX_CLANG_TIDY
#             ${CLANG_TIDY_EXE}
#             -config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
#             --use-color
#             -extra-arg=-Wno-unknown-warning-option
#             -extra-arg=-std=c++17
#             # Системные пути для C++
#             -extra-arg=-I/usr/include/c++/11
#             -extra-arg=-I/usr/include/x86_64-linux-gnu/c++/11
#             -extra-arg=-I/usr/include/c++/11/backward
#             -extra-arg=-I/usr/lib/gcc/x86_64-linux-gnu/11/include
#             -extra-arg=-I/usr/local/include
#             -extra-arg=-I/usr/include/x86_64-linux-gnu
#             -extra-arg=-I/usr/include
#             # Пути проекта
#             -extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/src
#             -extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/tests/iso-tp
#             -extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/tests/obd
#             -extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/../components/iso-tp
#             -extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/../components/phy_interface
#             -extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/../components/obd
#             -extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/Unity-2.6.1/src
#             -extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/mocks
#             -extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/mocks/freertos
#             # Макросы
#             -extra-arg=-DESP_LOG_H_
#             -extra-arg=-DFREERTOS_H
#             -extra-arg=-DTASK_H
#         )
        
#         set_target_properties(unity_app PROPERTIES CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
#     else()
#         message(WARNING "clang-tidy не найден. Проверка будет пропущена.")
#     endif()
# endif()
