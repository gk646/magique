cmake_minimum_required(VERSION 3.28)
project(asteroids)

set(CMAKE_CXX_STANDARD 23)

# Set the path to the magique directory
# Note: This is used when the magique directory is outside your project root directory
#       You can simply copy it into the root of your project as well

set(MAGIQUE_PATH ../../..)

add_subdirectory(${MAGIQUE_PATH} magique)

# Collect the source files
file(GLOB_RECURSE GAME_FILES "src/*.cpp")
add_executable(asteroids ${GAME_FILES})

# Dont forget to link against magique
target_link_libraries(asteroids PRIVATE magique)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(asteroids PRIVATE -fno-rtti) # magique is compiled without rtti - can be changed
endif ()