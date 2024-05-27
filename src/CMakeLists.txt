
# Add all subdirectories
add_subdirectory(assets)
#add_subdirectory(audio)
#add_subhandler(buildtools)
#add_subhandler(ecs)
add_subdirectory(lua)
add_subdirectory(external)
add_subdirectory(game)

# Main library
add_library(${TARGET_NAME} STATIC link.cpp)


# Output library at root
set_target_properties(${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# Includes
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)


# Linking
target_link_libraries(${TARGET_NAME} PUBLIC
        ${TARGET_NAME}-lua
        ${TARGET_NAME}-game
)