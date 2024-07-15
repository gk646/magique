# Set the CMake project name
set(PROJECT_PREFIX "MAGIQUE")
set(TARGET_NAME "magique")

# Set the library path
set(DEPENDENCY_PATH "C:/Users/gk646/Documents/Libraries")
set(SOURCE_PATH "C:/Users/gk646/Documents/CodingProjects/C++")
set(${PROJECT_PREFIX}_VERSION "0.0.1")

option(${PROJECT_PREFIX}_ENABLE_SANITIZER "Set to compile with adress sanitizer")
option(${PROJECT_PREFIX}_BUILD_TESTS "Set to compile the test executable")


include(cmake/CompilerOptions.cmake)