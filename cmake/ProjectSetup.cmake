
# Set the CMake project name
set(PROJECT_PREFIX "MAGIC")
set(TARGET_NAME "magic")

# Set the library path
set(DEPENDENCY_PATH "C:/Users/gk646/Documents/Libraries")

option(${PROJECT_PREFIX}_ENABLE_SANITIZER "Set to compile with adress sanitizer")
option(${PROJECT_PREFIX}_BUILD_TESTS "Set to compile the test executable")
option(${PROJECT_PREFIX}_VERSION "0.0.1")