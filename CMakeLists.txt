cmake_minimum_required(VERSION 3.22)

# Option to enable unit tests (native build)
option(BUILD_TESTS "Build unit tests for native platform" OFF)

# If building tests, use different configuration
if(BUILD_TESTS)
    message(STATUS "Building unit tests for native platform")
    add_subdirectory(test)
    return()
endif()

# Project name
project(MIevM C CXX ASM)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Enable assembler
enable_language(ASM)

# Specify the cross compiler
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Try to find arm-none-eabi toolchain
find_program(ARM_CC arm-none-eabi-gcc)
find_program(ARM_CXX arm-none-eabi-g++)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy)
find_program(ARM_SIZE arm-none-eabi-size)
find_program(ARM_OBJDUMP arm-none-eabi-objdump)

if(NOT ARM_CC)
    message(FATAL_ERROR "arm-none-eabi-gcc not found. Please install ARM GNU Toolchain.")
endif()

set(CMAKE_C_COMPILER ${ARM_CC})
set(CMAKE_CXX_COMPILER ${ARM_CXX})
set(CMAKE_ASM_COMPILER ${ARM_CC})
set(CMAKE_OBJCOPY ${ARM_OBJCOPY})
set(CMAKE_SIZE ${ARM_SIZE})
set(CMAKE_OBJDUMP ${ARM_OBJDUMP})

set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# MCU specific settings
set(MCU_FAMILY STM32F1xx)
set(MCU_MODEL STM32F105xC)
set(CPU_PARAMETERS
    -mcpu=cortex-m3
    -mthumb
    -mfloat-abi=soft
)

# Startup file - need to create if not exists
set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32f105xc.s)

# Linker script - need to create if not exists  
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F105XC_FLASH.ld)

# Compiler options
set(PROJECT_COMPILE_OPTIONS
    ${CPU_PARAMETERS}
    -Wall
    -Wextra
    -Wpedantic
    -fdata-sections
    -ffunction-sections
    -fstack-usage
    --specs=nano.specs
)

# Linker options
set(PROJECT_LINK_OPTIONS
    ${CPU_PARAMETERS}
    -T${LINKER_SCRIPT}
    --specs=nosys.specs
    --specs=nano.specs
    -Wl,-Map=${PROJECT_NAME}.map
    -Wl,--gc-sections
    -Wl,--print-memory-usage
)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Debug/Release specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Og -g3 -ggdb)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -O2 -g0)
endif()

# Preprocessor definitions
add_compile_definitions(
    USE_HAL_DRIVER
    ${MCU_MODEL}
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc/Legacy
)

# Project sources
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp
)

# HAL driver sources
file(GLOB HAL_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Src/*.c
)

# System sources
set(SYSTEM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/system_stm32f1xx.c
)

# All sources
set(PROJECT_SOURCES_ALL
    ${PROJECT_SOURCES}
    ${HAL_SOURCES}
    ${SYSTEM_SOURCES}
)

# Check if startup file exists, if not warn user
if(EXISTS ${STARTUP_SCRIPT})
    list(APPEND PROJECT_SOURCES_ALL ${STARTUP_SCRIPT})
else()
    message(WARNING "Startup file not found: ${STARTUP_SCRIPT}")
    message(WARNING "You may need to create startup_stm32f105xc.s")
endif()

# Check if linker script exists
if(NOT EXISTS ${LINKER_SCRIPT})
    message(WARNING "Linker script not found: ${LINKER_SCRIPT}")
    message(WARNING "You may need to create STM32F105XC_FLASH.ld")
endif()

# Create executable (without .elf extension, CMake will add it)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES_ALL})

# Apply compiler options
target_compile_options(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_OPTIONS})

# Apply linker options
target_link_options(${PROJECT_NAME} PRIVATE ${PROJECT_LINK_OPTIONS})

# Set output name to include .elf
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")

# Print executable size after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf.elf
)

# Create hex file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf.elf ${PROJECT_NAME}.hex
    COMMENT "Creating HEX file"
)

# Create binary file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf.elf ${PROJECT_NAME}.bin
    COMMENT "Creating BIN file"
)

# Create S-record file (like the existing canbridge.srec)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O srec ${PROJECT_NAME}.elf.elf ${PROJECT_NAME}.srec
    COMMENT "Creating SREC file"
)

# Create extended listing
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -h -S ${PROJECT_NAME}.elf.elf > ${PROJECT_NAME}.lst
    COMMENT "Creating LST file"
)

# Print build info
message(STATUS "=== Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "MCU: ${MCU_MODEL}")
message(STATUS "Toolchain: ${ARM_CC}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "========================")
