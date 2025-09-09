# Создание библиотеки для iso-tp
add_library(iso-tp ../../../components/iso-tp/iso-tp.cpp)

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

# Добавление директорий заголовочных файлов
target_include_directories(iso-tp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/iso-tp
    ${CMAKE_CURRENT_SOURCE_DIR}/../twai_interface
)