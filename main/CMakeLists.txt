set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "main.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_common freertos)
