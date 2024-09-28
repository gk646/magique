
# Determine if the project is being built as the main project or as a dependency
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(MAGIQUE_IS_MAIN TRUE)
else()
    set(MAGIQUE_IS_MAIN FALSE)
endif()

# In development mode - add a test directory
if (MAGIQUE_IS_MAIN)
    message(STATUS "Using magique in development mode - adding a tests directory")
    add_subdirectory(tests)
endif ()

# No steam sdk path given
if (MAGIQUE_STEAM AND NOT STEAM_SDK_PATH)
    message(FATAL_ERROR "Trying to use Steam integration without specifying the steam sdk path!
                         Use set(STEAM_SDK_PATH \"path/to/steam/sdk\") BEFORE adding magique"
    )
endif ()

if(MAGIQUE_STEAM AND MAGIQUE_LAN)
    message(STATUS "Both Steam and LAN is enabled. Enabling Steam allows access both Local and Global multiplayer")
    set(MAGIQUE_LAN OFF)
endif ()

if (MAGIQUE_STEAM)
    set(MAGIQUE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/steam")
    message(STATUS "Using magique WITH steam integration")
    add_compile_definitions(MAGIQUE_USE_STEAM)
elseif (MAGIQUE_LAN)
    set(MAGIQUE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/gamenetworkingsockets")
    message(STATUS "Using magique WITH local networking only")
else ()
    message(STATUS "Using magique WITHOUT steam integration")
    message(STATUS "Using magique WITHOUT local networking only")
endif ()

add_compile_definitions(MAGIQUE_VERSION="${MAGIQUE_VERSION}")

# Set parameters
set(MAGIQUE_PUBLIC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MAGIQUE_PRIVATE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MAGIQUE_CXSTRUCTS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/external/cxstructs)