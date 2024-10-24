# Function to copy
function(copy_shared_library source)
    add_custom_command(TARGET magique POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${source}"
            ${CMAKE_BINARY_DIR})
    message(STATUS "Copied networking libraries to the root of the build directory. Adjust if your executable is somewhere else")
endfunction()

# Link against the GameNetworkingSockets or the Steam SDK (and platform specific libs)
if (MAGIQUE_STEAM)
    if (WIN32)
        target_link_libraries(magique PRIVATE winmm ${MAGIQUE_BINARY_DIR}/win64/steam_api64.lib)
        copy_shared_library("${MAGIQUE_BINARY_DIR}/win64/steam_api64.dll")
    elseif (LINUX)
        target_link_libraries(magique PRIVATE ${MAGIQUE_BINARY_DIR}/linux64/steam_api)
        copy_shared_library("${MAGIQUE_BINARY_DIR}/linux64/libsteam_api.so")
    elseif (OSX)
        target_link_libraries(magique PRIVATE ${MAGIQUE_BINARY_DIR}/osx/steam_api)
        copy_shared_library("${MAGIQUE_BINARY_DIR}/osx/libsteam_api.dylib")
    else ()
        message(FATAL_ERROR "OS is not supported for Steam integration!")
    endif ()
elseif (MAGIQUE_LAN)
    if (WIN32)
        target_link_libraries(magique PRIVATE winmm ${MAGIQUE_BINARY_DIR}/win64/GameNetworkingSockets.lib)
        copy_shared_library("${MAGIQUE_BINARY_DIR}/win64/GameNetworkingSockets.dll")
    elseif (LINUX)
        target_link_libraries(magique PRIVATE ${MAGIQUE_BINARY_DIR}/linux64/GameNetworkingSockets)
        copy_shared_library("${MAGIQUE_BINARY_DIR}/linux64/libGameNetworkingSockets.so")
    elseif (OSX)
        target_link_libraries(magique PRIVATE ${MAGIQUE_BINARY_DIR}/osx/GameNetworkingSockets)
        copy_shared_library("${MAGIQUE_BINARY_DIR}/osx/libGameNetworkingSockets.dylib")
    else ()
        message(FATAL_ERROR "OS is not supported for GameNetworkingSockets!")
    endif ()
endif ()

if (WIN32)
    target_link_libraries(magique PRIVATE ws2_32.lib)
    if (MSVC)
        target_compile_options(magique PUBLIC /Zc:preprocessor)
    endif ()
endif ()

target_link_libraries(magique PUBLIC raylib)

message(STATUS "--------------- magique ------------------\n")