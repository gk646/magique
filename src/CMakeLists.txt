# Add all subdirectories
add_subdirectory(assets)
add_subdirectory(ecs)
add_subdirectory(scripting)
add_subdirectory(game)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(util)
add_subdirectory(persistence)

# Dependencies
add_subdirectory(external)


# Main library (Object libraries)
add_library(${TARGET_NAME} STATIC
        $<TARGET_OBJECTS:${TARGET_NAME}_core>
        $<TARGET_OBJECTS:${TARGET_NAME}_assets>
        $<TARGET_OBJECTS:${TARGET_NAME}_ecs>
        $<TARGET_OBJECTS:${TARGET_NAME}_scripting>
        $<TARGET_OBJECTS:${TARGET_NAME}_game>
        $<TARGET_OBJECTS:${TARGET_NAME}_util>
        $<TARGET_OBJECTS:${TARGET_NAME}_ui>
        $<TARGET_OBJECTS:${TARGET_NAME}_persistence>
)

target_link_libraries(${TARGET_NAME} PUBLIC raylib)

# Output library at root
set_target_properties(${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})