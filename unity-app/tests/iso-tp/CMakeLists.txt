# Создание библиотеки для тестов iso-tp
add_library(iso-tp-tests tests.cpp)

# Установка свойств компиляции
set_target_properties(iso-tp-tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Линковка с библиотеками
target_link_libraries(iso-tp-tests
    unity
    iso-tp
    twai_interface
)

# Добавление директорий заголовочных файлов
target_include_directories(iso-tp-tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/components/iso-tp
    ${CMAKE_SOURCE_DIR}/../components/iso-tp
    ${CMAKE_SOURCE_DIR}/components/twai_interface/../../../components/twai_interface
)