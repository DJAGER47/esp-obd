# Добавление поддиректорий для тестов каждого компонента
add_subdirectory(iso-tp)

# Создание библиотеки для тестов
add_library(tests INTERFACE)

# Линковка с библиотеками тестов компонентов
target_link_libraries(tests INTERFACE iso-tp-tests iso-tp-tests-extended)