cmake_minimum_required(VERSION 3.28)
project(wizardquest)

set(CMAKE_CXX_STANDARD 20)

set(MAGIQUE_PATH "C:/Users/gk646/Documents/CodingProjects/C++/magique")

file(GLOB_RECURSE GAME_FILES "src/*.cpp")

add_compile_options(/Zc:preprocessor)

add_executable(asteroids ${GAME_FILES})

target_include_directories(asteroids PRIVATE "${MAGIQUE_PATH}/include")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(asteroids PRIVATE "${MAGIQUE_PATH}/cmake-build-debug/magique.lib" winmm)
else ()
    target_link_libraries(asteroids PRIVATE "${MAGIQUE_PATH}/cmake-build-release/magique.lib" winmm)
endif ()