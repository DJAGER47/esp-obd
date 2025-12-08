set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "main.cpp"
                            "can_subscriber.cpp"
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

##################################################################
# Clang-tidy
##################################################################
# find_program(CLANG_TIDY_EXE NAMES "clang-tidy" REQUIRED)
# message(STATUS "CLANG_TIDY_EXE = ${CLANG_TIDY_EXE}")

# set(CMAKE_CXX_CLANG_TIDY
#      ${CLANG_TIDY_EXE}
#      -format-style='file'
#      -header-filter=.*
#      --use-color
#      -extra-arg=-std=c++17
#      -extra-arg=-Wno-unknown-warning-option
#      --exclude-header-filter=^esp-idf$
# )
# message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")

# # Копируем compile_commands.json перед применением clang-tidy
# # if(EXISTS ${CMAKE_BINARY_DIR}/compile_commands.json)
# #     # Создаем директорию clang-tidy если она не существует
# #     file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/clang-tidy)
# #     file(COPY ${CMAKE_BINARY_DIR}/compile_commands.json DESTINATION ${CMAKE_BINARY_DIR}/clang-tidy/)
# #     file(READ ${CMAKE_BINARY_DIR}/clang-tidy/compile_commands.json COMPILE_COMMANDS_CONTENT)
# #     string(REPLACE "-fno-shrink-wrap" "" COMPILE_COMMANDS_CONTENT "${COMPILE_COMMANDS_CONTENT}")
# #     string(REPLACE "-fno-tree-switch-conversion" "" COMPILE_COMMANDS_CONTENT "${COMPILE_COMMANDS_CONTENT}")
# #     string(REPLACE "-fstrict-volatile-bitfields" "" COMPILE_COMMANDS_CONTENT "${COMPILE_COMMANDS_CONTENT}")
# #     file(WRITE ${CMAKE_BINARY_DIR}/clang-tidy/compile_commands.json "${COMPILE_COMMANDS_CONTENT}")
# # endif()

# set_target_properties(${COMPONENT_LIB} PROPERTIES CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
# message(STATUS "COMPONENT_LIB = ${COMPONENT_LIB}")
# message(STATUS "CMAKE_CXX_CLANG_TIDY = ${CMAKE_CXX_CLANG_TIDY}")
# message(STATUS "CXX_CLANG_TIDY = ${CXX_CLANG_TIDY}")

##################################################################
# end
##################################################################


