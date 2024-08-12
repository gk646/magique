# In development mode - add a test directory
if (MAGIQUE_IS_MAIN)
    message(STATUS "Using magique in development mode - adding a test directory")
    add_subdirectory(test)
endif ()

# No steam sdk path given
if (MAGIQUE_STEAM_INTEGRATION AND NOT STEAM_SDK_PATH)
    message(FATAL_ERROR "Trying to use Steam integration without specifying the steam sdk path!
                         Use set(STEAM_SDK_PATH \"path/to/steam/sdk\") BEFORE adding magique"
    )
endif ()

# If a path is given turn it on (if not already)
if (EXISTS STEAM_SDK_PATH)
    set(MAGIQUE_STEAM_INTEGRATION ON)
endif ()


if (MAGIQUE_STEAM_INTEGRATION)
    set(MAGIQUE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/steam")
    message(STATUS "Using magique WITH steam integration")
else ()
    set(MAGIQUE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/gamenetworkingsockets")
    message(STATUS "Using magique WITHOUT steam integration")
endif ()

# Set parameters
set(MAGIQUE_PUBLIC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MAGIQUE_PRIVATE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MAGIQUE_CXSTRUCTS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/external/cxstructs)