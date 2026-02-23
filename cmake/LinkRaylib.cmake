# ----------------------------------------------------------------------
# Add raylib
# ----------------------------------------------------------------------

# Set build params
set(BUILD_EXAMPLES OFF)
set(BUILD_GAMES OFF)
set(PLATFORM "Desktop")
set(SUPPORT_MODULE_RAUDIO ON)

if (UNIX)
    set(GLFW_BUILD_X11 ON)
    # Weird bug where key repeats are not detected if this is enabled
    # Only happens in raylib not when the official examples are built with the same packaged GLFW...?
    set(GLFW_BUILD_WAYLAND OFF)
endif (UNIX)

message(STATUS "\n-- ------------- raylib ------------------")
add_subdirectory(src/external/raylib)

# Compiler args for raylib
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # For both modes
    target_compile_options(raylib PRIVATE
            -Wall -march=haswell -fno-exceptions -fvisibility=hidden
    )
    target_compile_options(raylib PRIVATE
            $<$<CONFIG:Debug>: -Og -Wall -g >
            $<$<CONFIG:Release>:-Ofast -DNDEBUG>
    )
    target_link_options(raylib PRIVATE)
elseif (MSVC)
    target_compile_options(raylib PRIVATE
            $<$<CONFIG:Debug>:/W3 /Od /Zi /RTC1>
            $<$<CONFIG:Release>:/DNDEBUG /W4 /Ob2 /O2 /GL /GF /GR- /fp:fast /arch:AVX2 /Gw /Zc:inline>
    )
    target_link_options(raylib PRIVATE $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>)
endif ()

# Includes for raylib
target_include_directories(raylib PRIVATE ${MAGIQUE_PUBLIC_INCLUDE}/raylib src/external/raylib-compat)
target_compile_definitions(raylib PRIVATE SUPPORT_CUSTOM_FRAME_CONTROL MAX_TEXTFORMAT_BUFFERS=5)

# ----------------------------------------------------------------------
# Link raylib
# ----------------------------------------------------------------------

# Link raylib to magique
target_link_libraries(magique PUBLIC raylib)