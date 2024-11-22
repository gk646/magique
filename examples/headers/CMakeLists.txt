# ----------------------------------------------------------------------
# CMake Configuration for examples
# ----------------------------------------------------------------------

add_executable(magique-example main.cpp)

add_subdirectory(../../ magique) # Path to the magique directory

target_link_libraries(magique-example PRIVATE magique)