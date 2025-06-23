idf_component_register(SRCS "main.c"
                            "x9c103s.c"
                            "ina226.c"
                            "ina226_new.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_driver_i2c)
