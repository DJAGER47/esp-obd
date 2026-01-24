set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "main.cpp"
                            "obd_data_polling.cpp"
                            "twai/twai_driver.cpp"
                            "FreeRTOS-openocd.c"
                            "debug.c"
                            "reset_handler.cpp"
                            "critical_section.cpp"
                            "vehicle_params.cpp"
                            "ui/ui.cpp"

                         #    "ui2/ui2.cpp"
                         #    "ui2/ld7138.cpp"
                         #    "ui2/ld7138_lvgl.cpp"
                         #    "ui2/esp_lcd_panel_ld7138.c"


                       INCLUDE_DIRS "."
                                    "twai/"
                                    "ui/"
                                   #  "ui2/"

                       REQUIRES driver
                                phy_interface
                                iso-tp
                                obd
                                lib
                                esp_lcd
                                )
