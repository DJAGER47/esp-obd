idf_component_register(SRCS "ui.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES spi_flash lvgl esp_lcd driver phy_interface)