cmake_minimum_required(VERSION 3.28)
project(wizardquest CXX)
set(CMAKE_CXX_STANDARD 20)

# Set the path to the magique directory
set(MAGIQUE_PATH "C:/Users/gk646/Documents/CodingProjects/C++/magique")
add_subdirectory(${MAGIQUE_PATH})

# Collect the source files
file(GLOB_RECURSE GAME_FILES "src/*.cpp")
add_executable(wizardquest ${GAME_FILES})

# Dont forget to link against magique
target_link_libraries(wizardquest magique)