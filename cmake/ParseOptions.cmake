# magique CMake section start
message(STATUS "--------------- magique ------------------")
message(STATUS "Version    : ${MAGIQUE_VERSION}")

if (MAGIQUE_SHARED)
    if (MAGIQUE_STEAM OR MAGIQUE_LAN)
        message(FATAL_ERROR "Cannot build magique as dynamic library with networking enabled!")
    endif ()
    message(STATUS "LibraryType: Shared")
else ()
    message(STATUS "LibraryType: Static")
endif ()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "BuildType  : Debug")
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    message(STATUS "BuildType  : Release")
endif ()

# Determine if the project is being built as the main project or as a dependency
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(MAGIQUE_IS_MAIN TRUE)
else ()
    set(MAGIQUE_IS_MAIN FALSE)
endif ()

# In development mode - add a test directory
if (MAGIQUE_IS_MAIN)
    message(STATUS "Usage      : Development Mode (Adding /tests as CMake subdirectory)")
    add_subdirectory(tests)
else ()
    message(STATUS "Usage      : User Mode")
endif ()

# No steam sdk path given
if (MAGIQUE_STEAM AND NOT STEAM_PATH)
    message(FATAL_ERROR "Trying to use Steam integration without specifying the steam sdk path!\n
    Use 'set(STEAM_PATH \"path/to/steam/sdk\")' BEFORE addding magique via add_subdirectory(...)")
endif ()

if (MAGIQUE_STEAM AND MAGIQUE_LAN)
    message(FATAL_ERROR "Both Steam and LAN is enabled! Enabling Steam allows access to both Local and Global multiplayer")
endif ()

if (MAGIQUE_STEAM)
    set(MAGIQUE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/steam")
    message(STATUS "Networking : Steam")
    message(STATUS "    LocalSockets : Supported")
    message(STATUS "    GlobalSockets: Supported")
elseif (MAGIQUE_LAN)
    set(MAGIQUE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/gamenetworkingsockets")
    message(STATUS "Networking : GameNetworkingSockets")
    message(STATUS "    LocalSockets : Supported")
    message(STATUS "    GlobalSockets: Unsupported")
else ()
    message(STATUS "Networking : None")
    message(STATUS "    LocalSockets : Unsupported")
    message(STATUS "    GlobalSockets: Unsupported")
endif ()

# Set parameters
set(MAGIQUE_PUBLIC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MAGIQUE_PRIVATE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MAGIQUE_CXSTRUCTS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/external/cxstructs)