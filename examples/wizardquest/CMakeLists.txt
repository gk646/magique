cmake_minimum_required(VERSION 3.28)
project(wizardquest CXX)
set(CMAKE_CXX_STANDARD 20)

# Set the path to the magique directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ magique)

# Collect the source files
file(GLOB_RECURSE GAME_FILES "src/*.cpp")
add_executable(wizardquest ${GAME_FILES})

# Dont forget to link against magique
target_link_libraries(wizardquest PUBLIC magique)

# OPTIONAL - Add src include path for clean includes
target_include_directories(wizardquest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)