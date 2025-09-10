idf_component_register(SRCS "iso-tp.cpp"
                       REQUIRES twai_interface 
                                freertos
                                driver
                       INCLUDE_DIRS ".")