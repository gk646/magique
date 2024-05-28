
# Add all subdirectories
add_subdirectory(assets)
add_subdirectory(ecs)
add_subdirectory(lua)
add_subdirectory(game)
add_subdirectory(loading)

# Dependencies
add_subdirectory(external)


# Main library (Object libraries)
add_library(${TARGET_NAME} STATIC
        core/Allocator.h
        $<TARGET_OBJECTS:${TARGET_NAME}_assets>
        $<TARGET_OBJECTS:${TARGET_NAME}_ecs>
        $<TARGET_OBJECTS:${TARGET_NAME}_lua>
        $<TARGET_OBJECTS:${TARGET_NAME}_game>
        $<TARGET_OBJECTS:${TARGET_NAME}_loading>
)

target_link_libraries(${TARGET_NAME} PUBLIC raylib)


# Output library at root
set_target_properties(${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})