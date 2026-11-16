cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32_starlet_ecu)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
option(STARLET_SYNC_BENCH "Enable PC6/PB8/PB9 GPIO sync acquisition with bench limits" OFF)
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# STM32CubeMX生成済みファイル、または明示したSTM32CubeF4 SDKを使用する。
set(STM32CUBE_F4_PATH "" CACHE PATH "STM32CubeF4 v1.28.2 root (recursive checkout)")
if(STM32CUBE_F4_PATH)
    get_filename_component(STARLET_SDK "${STM32CUBE_F4_PATH}" ABSOLUTE)
    set(STARLET_DRIVERS "${STARLET_SDK}/Drivers")
    set(STARLET_STARTUP "${STARLET_DRIVERS}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f446xx.s")
else()
    set(STARLET_DRIVERS "${CMAKE_SOURCE_DIR}/Drivers")
    set(STARLET_STARTUP "${CMAKE_SOURCE_DIR}/startup_stm32f446xx.s")
endif()
foreach(required IN ITEMS
    "${STARLET_STARTUP}"
    "${STARLET_DRIVERS}/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h"
    "${STARLET_DRIVERS}/CMSIS/Include/core_cm4.h"
    "${STARLET_DRIVERS}/CMSIS/Device/ST/STM32F4xx/Include/stm32f446xx.h")
    if(NOT EXISTS "${required}")
        message(FATAL_ERROR "Missing SDK file: ${required}. See README: STM32CubeF4 v1.28.2 setup; set STM32CUBE_F4_PATH or generate with CubeMX.")
    endif()
endforeach()

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/engine_sync.c
    Core/Src/engine_sync_service.c
)

if(STARLET_SYNC_BENCH)
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Core/Src/engine_sync_board.c)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE STARLET_SYNC_BENCH=1)
endif()

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Core/Inc
    ${STARLET_DRIVERS}/STM32F4xx_HAL_Driver/Inc
    ${STARLET_DRIVERS}/CMSIS/Device/ST/STM32F4xx/Include
    ${STARLET_DRIVERS}/CMSIS/Include
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

