cmake_minimum_required(VERSION 3.21)
project(magique CXX)

set(CMAKE_CXX_STANDARD 20)

include(cmake/ProjectSetup.cmake)
include(cmake/BuildRelease.cmake)

add_subdirectory(src)

set(${PROJECT_PREFIX}_BUILD_TESTS ON)

if (${PROJECT_PREFIX}_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()