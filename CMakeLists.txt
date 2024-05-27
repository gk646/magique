cmake_minimum_required(VERSION 3.28)
project(magicque CXX)

set(CMAKE_CXX_STANDARD 23)

include(cmake/ProjectSetup.cmake)

add_subdirectory(src)

if (${PROJECT_PREFIX}_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()