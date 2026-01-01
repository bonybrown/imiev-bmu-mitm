cmake_minimum_required(VERSION 3.14)

project(MIevM_Tests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Path to CppUTest source in the parent directory
set(CPPUTEST_HOME "${CMAKE_CURRENT_SOURCE_DIR}/../cpputest")

# Check if CppUTest source exists
if(NOT EXISTS "${CPPUTEST_HOME}/CMakeLists.txt")
    message(FATAL_ERROR "CppUTest source not found at ${CPPUTEST_HOME}. Please clone it with:\n"
                        "cd ${CMAKE_CURRENT_SOURCE_DIR}/..\n"
                        "git clone --depth 1 --branch v4.0 https://github.com/cpputest/cpputest.git")
endif()

# Build CppUTest from source
set(TESTS OFF CACHE BOOL "Switch off CppUTest Test build")
set(MEMORY_LEAK_DETECTION ON CACHE BOOL "Enable memory leak detection")
add_subdirectory(${CPPUTEST_HOME} ${CMAKE_CURRENT_BINARY_DIR}/cpputest-build)

# Include directories
include_directories(
    ${CPPUTEST_HOME}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/CMSIS/Include
)

# Test executable
add_executable(MIevM_Tests
    test_main.cpp
    test_voltage_byte.cpp
    test_can_message_373.cpp
    test_can_message_374.cpp
    test_can_queue.cpp
    test_utility.cpp
    ../Src/VoltageByte.cpp
    ../Src/CanMessage373.cpp
    ../Src/CanMessage374.cpp
    ../Src/App.cpp
    ../Src/utility.c
)

# Link against CppUTest
target_link_libraries(MIevM_Tests
    CppUTest
    CppUTestExt
)

# Add test
add_test(NAME MIevM_Tests COMMAND MIevM_Tests)
