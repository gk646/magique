cmake_minimum_required(VERSION 3.28)
project(assetloading CXX)
set(CMAKE_CXX_STANDARD 20)

# Collect the source files
file(GLOB_RECURSE DEMO_FILES "src/*.cpp")
add_executable(assetloading ${DEMO_FILES})

# Set the path to the magique directory - inside this project we can just use the relative path
add_subdirectory(../../.. magique)

# Don't forget to link against magique
target_link_libraries(assetloading PUBLIC magique)

# OPTIONAL - Add src include path for clean includes
target_include_directories(assetloading PRIVATE src)