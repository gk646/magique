cmake_minimum_required(VERSION 3.28)
project(wizardquest CXX)
set(CMAKE_CXX_STANDARD 20)

# Collect the source files
file(GLOB_RECURSE GAME_FILES "src/*.cpp")
add_executable(wizardquest ${GAME_FILES})

set(STEAM_PATH "/home/gk646/devlibs/steamworks_sdk_161/sdk")
set(MAGIQUE_STEAM ON) #- Use this to enable steam support

# Set the path to the magique directory
add_subdirectory(../../.. magique)

# Don't forget to link against magique
target_link_libraries(wizardquest PUBLIC magique)

# OPTIONAL - Add src include path for clean includes
target_include_directories(wizardquest PRIVATE src)