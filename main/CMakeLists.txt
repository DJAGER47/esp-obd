idf_component_register(SRCS "main.c"
                            "ina226.c" 
                            "x9c103s.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_driver_i2c)
