cmake_minimum_required(VERSION 3.21)
project(magique CXX C)

set(CMAKE_CXX_STANDARD 20)

# Set project properties
set(MAGIQUE_VERSION "0.0.8")

# Set cmake files directory
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# MAGIQUE_IS_MAIN determines whether the project is being used from root
# or if it is added as a dependency (through add_subdirectory for example).
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(MAGIQUE_IS_MAIN TRUE)
else ()
    set(MAGIQUE_IS_MAIN FALSE)
endif ()

# Options - NEEDS A CACHE RESET IF USING CMAKE CACHE!
option(MAGIQUE_STEAM_INTEGRATION "Set to enable steam integration (Must set STEAM_SDK_PATH)" OFF)
option(MAGIQUE_NETWORKING "Set to enable local networking" ON)
option(MAGIQUE_BUILD_SHARED "Set to enable building magique as shared library" OFF)
option(MAGIQUE_USE_SIMD "Set to enable usage of SIMD (faster)" ON)
option(MAGIQUE_ENABLE_SANITIZER "Set to compile with address sanitizer" OFF)

# Parse the options and set parameters
include(ParseOptions)

# Setup compiler options
include(CompilerOptions)

# Add the source targets
add_subdirectory(src)