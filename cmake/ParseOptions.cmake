# In development mode - add a test directory
if (MAGIQUE_IS_MAIN)
    message(STATUS "Using magique in development mode - adding a test directory")
    add_subdirectory(test)
endif ()

# Evaluate options
if (MAGIQUE_STEAM_INTEGRATION AND NOT STEAM_SDK_PATH)
    message(FATAL_ERROR "Trying to use Steam integration without specifying the steam sdk path!
                         Use set(STEAM_SDK_PATH \"path/to/steam/sdk\") BEFORE adding magique"
    )
endif ()


# If a path is given turn it on (if not already)
if (EXISTS STEAM_SDK_PATH)
    set(MAGIQUE_STEAM_INTEGRATION ON)
endif ()