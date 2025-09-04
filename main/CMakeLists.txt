set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "main.cpp"
                            "twai_driver.cpp"
                            "iso-tp.cpp"
                            "obd2_cmd.cpp"
                            "obd2_pid.cpp"
                            "obd2_pid_1_20.cpp"
                            "obd2_pid_21_40.cpp"
                            "obd2_pid_41_60.cpp"
                            "obd2_pid_61_80.cpp"

                       REQUIRES driver
                       INCLUDE_DIRS ".")
