cmake_minimum_required(VERSION 3.28)
project(asteroids)

set(CMAKE_CXX_STANDARD 23)


set(MAGIQUE_PATH "C:/Users/gk646/Documents/CodingProjects/C++/magique")

file(GLOB_RECURSE GAME_FILES "src/*.cpp")

add_executable(asteroids ${GAME_FILES})

target_include_directories(asteroids PRIVATE "${MAGIQUE_PATH}/include")
target_link_libraries(asteroids PRIVATE "${MAGIQUE_PATH}/cmake-build-debug/magique.lib" winmm)