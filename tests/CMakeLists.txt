enable_testing()

# Parse files
file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(magique-tests ${TEST_FILES})

target_link_libraries(magique-tests PRIVATE raylib magique)
target_include_directories(magique-tests PRIVATE catch2 ../src)
target_compile_definitions(magique-tests PRIVATE MAGIQUE_TEST_MODE) # makes internal things public

include(CTest)
include(catch_cmake/Catch.cmake)
catch_discover_tests(magique-tests TEST_PREFIX "Magique.")