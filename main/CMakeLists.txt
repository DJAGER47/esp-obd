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

find_program(CLANG_TIDY_EXE NAMES "clang-tidy" REQUIRED)
message(STATUS "CLANG_TIDY_EXE = ${CLANG_TIDY_EXE}")

set(CMAKE_CXX_CLANG_TIDY
     ${CLANG_TIDY_EXE};
     -p ${CMAKE_BINARY_DIR}/clang-tidy/;
     -format-style='file';
     --use-color;
     -extra-arg=-std=c++17;
     -extra-arg=-Wno-unknown-warning-option;
)
message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")

# Копируем compile_commands.json перед применением clang-tidy
if(EXISTS ${CMAKE_BINARY_DIR}/compile_commands.json)
    file(COPY ${CMAKE_BINARY_DIR}/compile_commands.json DESTINATION ${CMAKE_BINARY_DIR}/clang-tidy/)
    # Заменяем -fno-shrink-wrap на пустую строку в скопированном файле
    file(READ ${CMAKE_BINARY_DIR}/clang-tidy/compile_commands.json COMPILE_COMMANDS_CONTENT)
    string(REPLACE "-fno-shrink-wrap" "" COMPILE_COMMANDS_CONTENT "${COMPILE_COMMANDS_CONTENT}")
    string(REPLACE "-fno-tree-switch-conversion" "" COMPILE_COMMANDS_CONTENT "${COMPILE_COMMANDS_CONTENT}")
    string(REPLACE "-fstrict-volatile-bitfields" "" COMPILE_COMMANDS_CONTENT "${COMPILE_COMMANDS_CONTENT}")
    file(WRITE ${CMAKE_BINARY_DIR}/clang-tidy/compile_commands.json "${COMPILE_COMMANDS_CONTENT}")
endif()

set_target_properties(${COMPONENT_LIB} PROPERTIES CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
message(STATUS "COMPONENT_LIB = ${COMPONENT_LIB}")
