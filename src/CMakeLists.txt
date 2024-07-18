# Add all subdirectories
add_subdirectory(assets)
add_subdirectory(ecs)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(util)
add_subdirectory(multiplayer)
add_subdirectory(persistence)
add_subdirectory(internal)
add_subdirectory(gamedev)

# Dependencies
add_subdirectory(external)

# Main library (Object libraries)
add_library(${TARGET_NAME} STATIC
        $<TARGET_OBJECTS:${TARGET_NAME}_core>
        $<TARGET_OBJECTS:${TARGET_NAME}_assets>
        $<TARGET_OBJECTS:${TARGET_NAME}_ecs>
        $<TARGET_OBJECTS:${TARGET_NAME}_util>
        $<TARGET_OBJECTS:${TARGET_NAME}_ui>
        $<TARGET_OBJECTS:${TARGET_NAME}_internal>
        $<TARGET_OBJECTS:${TARGET_NAME}_multiplayer>
        $<TARGET_OBJECTS:${TARGET_NAME}_gamedev>
        $<TARGET_OBJECTS:${TARGET_NAME}_persistence>
        $<TARGET_OBJECTS:raylib>
        $<TARGET_OBJECTS:glfw>
)

target_link_libraries(${TARGET_NAME} PUBLIC raylib glfw winmm ${DEPENDENCY_PATH}/sdk/redistributable_bin/win64/steam_api64.lib)

# Output library at root
set_target_properties(${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})