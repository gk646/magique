# Add catch 2 - the easy way
add_library(catch2 STATIC "${DEPENDENCY_PATH}/catch2v3/catch_amalgamated.cpp")

# Collect all ".cpp" files
file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Create the game executable
add_executable(${PROJECT_NAME}-test ${TEST_FILES})

#include the include directories
target_include_directories(${PROJECT_NAME}-test PRIVATE ${CMAKE_SOURCE_DIR}/include "${DEPENDENCY_PATH}/catch2v3")

# Link against the engine
target_link_libraries(${PROJECT_NAME}-test PUBLIC catch2 ${TARGET_NAME})

# Output to root of binary dir
set_target_properties(${PROJECT_NAME}-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Build the engine before if necessary
add_dependencies(${PROJECT_NAME}-test ${TARGET_NAME})

# Register the test with CMake - run from binary dir
#add_test(NAME SetupTest COMMAND ${PROJECT_NAME}-test [Setup] WORKING_DIRECTORY ${CMAKE_BINARY_DIR})