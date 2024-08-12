# Add all subdirectories
add_subdirectory(assets)
add_subdirectory(core)
add_subdirectory(ecs)
add_subdirectory(external) # External dependencies (raylib)
add_subdirectory(gamedev)
add_subdirectory(internal)
add_subdirectory(multiplayer)
add_subdirectory(persistence)
add_subdirectory(ui)
add_subdirectory(util)

# Enable steam module if integration is enabled
if (MAGIQUE_STEAM_INTEGRATION)
    add_subdirectory(steam)
endif ()

# Collect all object files into a list
set(MAGIQUE_OBJECTS
        $<TARGET_OBJECTS:magique-core>
        $<TARGET_OBJECTS:magique-assets>
        $<TARGET_OBJECTS:magique-ecs>
        $<TARGET_OBJECTS:magique-util>
        $<TARGET_OBJECTS:magique-ui>
        $<TARGET_OBJECTS:magique-internal>
        $<TARGET_OBJECTS:magique-multiplayer>
        $<TARGET_OBJECTS:magique-gamedev>
        $<TARGET_OBJECTS:magique-persistence>
        $<TARGET_OBJECTS:raylib>
        $<TARGET_OBJECTS:glfw>
)

# Conditionally include the Steam object library if Steam integration is enabled
if (MAGIQUE_STEAM_INTEGRATION)
    list(APPEND MAGIQUE_OBJECTS $<TARGET_OBJECTS:magique-steam>)
endif ()

# Main library (Object libraries)
if (MAGIQUE_BUILD_SHARED)
    add_library(magique SHARED ${MAGIQUE_OBJECTS})
else ()
    add_library(magique STATIC ${MAGIQUE_OBJECTS})
endif ()

# Include directories
target_include_directories(magique PUBLIC ${CMAKE_SOURCE_DIR}/include)

if (WIN32)
    target_link_libraries(magique PRIVATE winmm)
endif ()


if (MAGIQUE_STEAM_INTEGRATION)
    if (WIN32)
        target_link_libraries(magique PRIVATE winmm ${CMAKE_SOURCE_DIR}/bin/steam/win64/steam_api64)
    elseif ()
    endif ()
else ()
endif ()