
# Add all subdirectories
add_subdirectory(assets)
add_subdirectory(ecs)
add_subdirectory(lua)
add_subdirectory(external)
add_subdirectory(game)
add_subdirectory(loading)

# Main library
add_library(${TARGET_NAME} STATIC link.cpp)


# Output library at root
set_target_properties(${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# Linking
target_link_libraries(${TARGET_NAME} PUBLIC
        ${TARGET_NAME}-lua
        ${TARGET_NAME}-assets
        ${TARGET_NAME}-ecs
        ${TARGET_NAME}-loading
        ${TARGET_NAME}-game
)