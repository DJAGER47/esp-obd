cmake_minimum_required(VERSION 3.22)
project(unity_app)

# Создание библиотеки для Unity
add_library(unity Unity-2.6.1/src/unity.c)

# Установка свойств компиляции
set_target_properties(unity PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Добавление директорий заголовочных файлов
target_include_directories(unity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Unity-2.6.1/src
)

# Добавление компонентов
add_subdirectory(components)

# Добавление тестов
add_subdirectory(tests)

# Создание исполняемого файла
add_executable(unity_app src/main.cpp)

# Установка свойств компиляции для исполняемого файла
set_target_properties(unity_app PROPERTIES
    CXX_STANDARD 11
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