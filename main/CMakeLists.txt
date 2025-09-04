set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "main.cpp"
                            "twai/twai_driver.cpp"
                            "iso-tp/iso-tp.cpp"
                            "obd/obd2_cmd.cpp"
                            "obd/obd2_pid.cpp"
                            "obd/obd2_pid_1_20.cpp"
                            "obd/obd2_pid_21_40.cpp"
                            "obd/obd2_pid_41_60.cpp"
                            "obd/obd2_pid_61_80.cpp"
                            

                       REQUIRES driver
                       INCLUDE_DIRS "."
                                    "twai/"
                                    "iso-tp/"
                                    "obd/")
