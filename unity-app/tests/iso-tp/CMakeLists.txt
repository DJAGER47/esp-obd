# Создание библиотеки для базовых тестов iso-tp
add_library(iso-tp-tests tests_single.cpp)

# Создание библиотеки для расширенных тестов iso-tp
add_library(iso-tp-tests-extended tests_long_send.cpp)

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
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/components/iso-tp
    ${CMAKE_SOURCE_DIR}/../components/iso-tp
    ${CMAKE_SOURCE_DIR}/components/twai_interface/../../../components/twai_interface
)

# Добавление директорий заголовочных файлов для расширенных тестов
target_include_directories(iso-tp-tests-extended PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/components/iso-tp
    ${CMAKE_SOURCE_DIR}/../components/iso-tp
    ${CMAKE_SOURCE_DIR}/components/twai_interface/../../../components/twai_interface
)