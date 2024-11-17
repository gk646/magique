set(BUILD_EXAMPLES OFF)
set(BUILD_GAMES OFF)
set(OPENGL_VERSION "4.3")
set(PLATFORM "Desktop")

if (UNIX)
    set(GLFW_BUILD_WAYLAND ON) # Use wayland per default
endif (UNIX)

message(STATUS "\n-- ------------- raylib ------------------")
add_subdirectory(raylib)
message(STATUS "---------------- raylib ------------------\n")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(raylib PRIVATE
            $<$<CONFIG:Debug>: -Og -Wall -Wextra -g >
            $<$<CONFIG:Release>:-Ofast -ffast-math -funroll-loops -march=native -DNDEBUG>
    )
    target_link_options(raylib PRIVATE )
elseif (MSVC)
    target_compile_options(raylib PRIVATE
            $<$<CONFIG:Debug>:/W3 /Od /Zi /RTC1>
            $<$<CONFIG:Release>:/DNDEBUG /W4 /Ob2 /O2 /GL /GF /GR- /fp:fast /arch:AVX2 /Gw /Zc:inline>
    )
    target_link_options(raylib PRIVATE $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>)
endif()

target_include_directories(raylib PRIVATE ${MAGIQUE_PUBLIC_INCLUDE})