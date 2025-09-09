# Создание библиотеки для twai_interface
add_library(twai_interface INTERFACE)

# Добавление директорий заголовочных файлов
target_include_directories(twai_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)