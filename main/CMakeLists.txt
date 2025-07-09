set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "main.cpp" "CANDriver.cpp" "PIDParser.cpp" "TripCalculator.cpp" "FuelCalculator.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_common freertos esp_driver_twai)
