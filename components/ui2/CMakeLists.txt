idf_component_register(SRCS "ui2.cpp" "ld7138.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES spi_flash lvgl esp_lcd driver phy_interface mutex)