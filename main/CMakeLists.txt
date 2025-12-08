set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "main.cpp"
                            "twai/twai_driver.cpp"
                            "FreeRTOS-openocd.c"
                            "debug.c"

                       INCLUDE_DIRS "."
                                    "twai/"

                       REQUIRES driver
                                phy_interface
                                iso-tp
                                obd
                                lib
                                ui)
