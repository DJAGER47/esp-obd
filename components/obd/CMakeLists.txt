idf_component_register(SRCS   "obd2_cmd.cpp"
                              "obd2_pid.cpp"
                              "obd2_pid_1_20.cpp"
                              "obd2_pid_21_40.cpp"
                              "obd2_pid_41_60.cpp"
                              "obd2_pid_61_80.cpp"
                              "obd2_pid_81_100.cpp"
                              "obd2_pid_101_120.cpp"
                              "obd2_pid_121_140.cpp"
                              "obd2_service_09.cpp"
                              "obd2_cache.cpp"
                             
                       REQUIRES iso-tp
                                freertos
                                driver
                                INCLUDE_DIRS ".")