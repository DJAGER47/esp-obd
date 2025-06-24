idf_component_register(SRCS "ina226.c" "main.c"
                            "x9c103s.c"
                           
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_driver_i2c)
