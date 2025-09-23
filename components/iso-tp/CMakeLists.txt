idf_component_register(SRCS "iso_tp.cpp"
                       REQUIRES twai_interface 
                                freertos
                                driver
                       INCLUDE_DIRS ".")