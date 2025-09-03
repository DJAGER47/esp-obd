set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "main.cpp"
                            "twai_driver.cpp"
                            "iso-tp.cpp"
                            "obd2_cmd.cpp"
                            "obd2_pid.cpp"

                       REQUIRES driver
                       INCLUDE_DIRS ".")
