set(BUILD_EXAMPLES OFF )
set(BUILD_GAMES OFF)
set(OPENGL_VERSION "4.3" )
set(PLATFORM "Desktop")


set(CMAKE_C_FLAGS_DEBUG "/W3")
set(CMAKE_C_FLAGS_RELEASE "/W3")

add_subdirectory(raylib)

target_compile_options(raylib PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2 /Ob3 /GL /GS- /GF /GR- /fp:fast /arch:AVX2 /Gw /Zc:inline>
)

target_link_options(raylib PRIVATE
        $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>
)
target_include_directories(raylib PRIVATE ${MAGIQUE_PUBLIC_INCLUDE})