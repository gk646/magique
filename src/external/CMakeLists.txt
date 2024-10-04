set(BUILD_EXAMPLES OFF)
set(BUILD_GAMES OFF)
set(OPENGL_VERSION "4.3")
set(PLATFORM "Desktop")


message(STATUS "\n-- ------------- raylib ------------------")
add_subdirectory(raylib)
message(STATUS "---------------- raylib ------------------\n")


if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

elseif (MSVC)
    set(CMAKE_C_FLAGS_RELEASE "/W3 /Ob3")
    # Clear flags
    target_compile_options(raylib PRIVATE
            $<$<CONFIG:Debug>:/W3 /Od /Zi /RTC1>
            $<$<CONFIG:Release>:/O2 /GL /GS- /GF /GR- /fp:fast /arch:AVX2 /Gw /Zc:inline>
    )

    target_compile_options(raylib PRIVATE /W3)
    target_link_options(raylib PRIVATE $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>)
endif ()

target_include_directories(raylib PRIVATE ${MAGIQUE_PUBLIC_INCLUDE})