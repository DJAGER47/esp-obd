cmake_minimum_required(VERSION 3.22)
project(unity_app)

##################################################################
# Объединенный CMakeLists.txt для unity-app
# Содержит все настройки компонентов и тестов в одном файле
##################################################################

# Создание библиотеки для Unity
add_library(unity Unity-2.6.1/src/unity.c)

# Установка флагов компиляции для библиотеки Unity в зависимости от типа сборки
target_compile_options(unity PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Установка свойств компиляции
set_target_properties(unity PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Добавление директорий заголовочных файлов
target_include_directories(unity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Unity-2.6.1/src
)

##################################################################
# Компоненты
##################################################################

# Создание библиотеки для twai_interface
add_library(twai_interface INTERFACE)

# Добавление директорий заголовочных файлов для twai_interface
target_include_directories(twai_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/twai_interface
)

# Создание библиотеки для iso-tp
add_library(iso-tp ../components/iso-tp/iso-tp.cpp)

# Установка флагов компиляции в зависимости от типа сборки
target_compile_options(iso-tp PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Установка свойств компиляции
set_target_properties(iso-tp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Линковка с библиотеками
target_link_libraries(iso-tp 
    twai_interface
    esp_mocks
)

# Добавление директорий заголовочных файлов для iso-tp
target_include_directories(iso-tp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/iso-tp
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/twai_interface
)

# Создаем интерфейсные библиотеки для моков ESP-IDF
add_library(esp_mocks INTERFACE)

# Добавляем директории заголовочных файлов для моков
target_include_directories(esp_mocks INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

##################################################################
# Тесты
##################################################################

# Создание библиотеки для базовых тестов iso-tp
add_library(iso-tp-tests tests/iso-tp/tests_single.cpp)

# Создание библиотеки для расширенных тестов iso-tp
add_library(iso-tp-tests-extended tests/iso-tp/tests_long_send.cpp)

# Установка флагов компиляции для базовых тестов в зависимости от типа сборки
target_compile_options(iso-tp-tests PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Установка свойств компиляции для базовых тестов
set_target_properties(iso-tp-tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Установка флагов компиляции для расширенных тестов в зависимости от типа сборки
target_compile_options(iso-tp-tests-extended PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Установка свойств компиляции для расширенных тестов
set_target_properties(iso-tp-tests-extended PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Линковка с библиотеками для базовых тестов
target_link_libraries(iso-tp-tests
    unity
    iso-tp
    twai_interface
)

# Линковка с библиотеками для расширенных тестов
target_link_libraries(iso-tp-tests-extended
    unity
    iso-tp
    twai_interface
)

# Добавление директорий заголовочных файлов для базовых тестов
target_include_directories(iso-tp-tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/iso-tp
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/iso-tp
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/twai_interface
)

# Добавление директорий заголовочных файлов для расширенных тестов
target_include_directories(iso-tp-tests-extended PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/iso-tp
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/iso-tp
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/twai_interface
)

# Создание библиотеки для тестов
add_library(tests INTERFACE)

# Линковка с библиотеками тестов компонентов
target_link_libraries(tests INTERFACE 
    iso-tp-tests 
    iso-tp-tests-extended
)

##################################################################
# Исполняемый файл
##################################################################

# Создание исполняемого файла
add_executable(unity_app src/main.cpp)

# Установка флагов компиляции в зависимости от типа сборки
target_compile_options(unity_app PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Установка свойств компиляции для исполняемого файла
set_target_properties(unity_app PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Линковка с библиотеками
target_link_libraries(unity_app 
    unity
    tests
)

# Добавление директорий заголовочных файлов
target_include_directories(unity_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

##################################################################
# Информационные сообщения
##################################################################
message(STATUS "=== Конфигурация Unity App ===")
message(STATUS "Проект: ${PROJECT_NAME}")
message(STATUS "Тип сборки: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ стандарт: ${CMAKE_CXX_STANDARD}")
message(STATUS "Компилятор: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "==============================")