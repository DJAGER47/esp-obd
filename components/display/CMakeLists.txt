idf_component_register(SRCS   "ssd1283a.cpp"
                              "lvgl_port.cpp"
                              
                        REQUIRES driver
                                 lvgl
                                 freertos
                        INCLUDE_DIRS ".")