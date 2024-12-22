# Function to copy
function(copy_shared_library ...)
    # Iterate over each item in the arguments
    foreach (source IN LISTS ARGV)
        add_custom_command(TARGET magique POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${source}"
                ${CMAKE_BINARY_DIR}
                COMMENT "Copied ${source} to the root of the build directory. Adjust if your executable is located elsewhere."
        )
    endforeach ()
    message(STATUS "Copied networking libs to the root of the build directory. Adjust if your executable is located elsewhere.")
endfunction()


# Link against the GameNetworkingSockets or the Steam SDK (and platform specific libs)
if (MAGIQUE_STEAM)
    if (WIN32)
        target_link_libraries(magique PRIVATE winmm ${MAGIQUE_BINARY_DIR}/win64/steam_api64.lib)
        copy_shared_library("${MAGIQUE_BINARY_DIR}/win64/steam_api64.dll")
    elseif (LINUX)
        target_link_libraries(magique PRIVATE ${MAGIQUE_BINARY_DIR}/linux64/libsteam_api.so)
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
        target_link_libraries(magique PRIVATE ${MAGIQUE_BINARY_DIR}/linux64/libGameNetworkingSockets.so)
        target_link_libraries(magique PRIVATE ${MAGIQUE_BINARY_DIR}/linux64/libprotobuf.so.30)
        copy_shared_library("${MAGIQUE_BINARY_DIR}/linux64/libGameNetworkingSockets.so" "${MAGIQUE_BINARY_DIR}/linux64/libprotobuf.so.30")
        set_target_properties(magique PROPERTIES BUILD_RPATH "$ORIGIN" INSTALL_RPATH "$ORIGIN") # To find the lib
    elseif (OSX)
        message(FATAL_ERROR "OS is (currently) not supported for GameNetworkingSockets!")
        target_link_libraries(magique PRIVATE ${MAGIQUE_BINARY_DIR}/osx/GameNetworkingSockets)
        copy_shared_library("${MAGIQUE_BINARY_DIR}/osx/libGameNetworkingSockets.dylib")
    else ()
        message(FATAL_ERROR "OS is not supported for GameNetworkingSockets!")
    endif ()
endif ()


# Compile the definition that need to be inherited by targets to not get build errors
# Inherit C++20 to target

if (MAGIQUE_STEAM)
    target_compile_definitions(magique PUBLIC MAGIQUE_STEAM)
elseif (MAGIQUE_LAN)
    target_compile_definitions(magique PUBLIC MAGIQUE_LAN)
endif ()

if (WIN32)
    target_link_libraries(magique PRIVATE ws2_32.lib)
    if (MSVC)
        target_compile_options(magique PUBLIC /std:c++20 /Zc:preprocessor)
    endif ()
elseif (UNIX)
    target_compile_options(magique PUBLIC -std=c++20 -flto=auto -fno-rtti)
else ()
    #
endif ()

# Make internal things public in the header
if (MAGIQUE_IS_MAIN)
    target_compile_definitions(magique PUBLIC MAGIQUE_TEST_MODE)
endif ()