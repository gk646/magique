# ----------------------------------------------------------------------
# CMake Configuration for magique
# ----------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)
project(magique LANGUAGES CXX C)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 23)
set(MAGIQUE_VERSION "0.9.0")

# Set CMake modules directory - path searched when using include()
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ----------------------------------------------------------------------
# Configuration Options
# ----------------------------------------------------------------------

# Options - IF USING CMAKE CACHE, CHANGES NEED A CACHE RESET TO APPLY!
option(MAGIQUE_STEAM "Enable Steam integration (requires STEAM_PATH)" OFF)
option(MAGIQUE_LAN "Enable local networking" OFF)
option(MAGIQUE_SHARED "Build Magique as a shared library (experimental)" OFF)
option(MAGIQUE_SANITIZER "Compile with address sanitizer" OFF)

# ----------------------------------------------------------------------
# Include Configuration
# ----------------------------------------------------------------------

include(ParseOptions)      # Parses options and sets parameters

# ----------------------------------------------------------------------
# Source Targets
# ----------------------------------------------------------------------

# Add source directories
add_subdirectory(src/assets)
add_subdirectory(src/core)
add_subdirectory(src/ecs)
add_subdirectory(src/gamedev)
add_subdirectory(src/internal)
add_subdirectory(src/persistence)
add_subdirectory(src/ui)
add_subdirectory(src/util)
add_subdirectory(src/steam)
add_subdirectory(src/multiplayer)

# ----------------------------------------------------------------------
# Collect and Organize Object Files
# ----------------------------------------------------------------------

# Gather object files from all submodules
set(MAGIQUE_OBJECTS
        $<TARGET_OBJECTS:magique-core>
        $<TARGET_OBJECTS:magique-assets>
        $<TARGET_OBJECTS:magique-ecs>
        $<TARGET_OBJECTS:magique-util>
        $<TARGET_OBJECTS:magique-ui>
        $<TARGET_OBJECTS:magique-internal>
        $<TARGET_OBJECTS:magique-gamedev>
        $<TARGET_OBJECTS:magique-persistence>
        $<TARGET_OBJECTS:magique-steam>
        $<TARGET_OBJECTS:magique-multiplayer>
)

# ----------------------------------------------------------------------
# Build the Main Magique Library
# ----------------------------------------------------------------------

# Choose between building a shared or static library
if (MAGIQUE_SHARED)
    add_library(magique SHARED ${MAGIQUE_OBJECTS})
else ()
    add_library(magique STATIC ${MAGIQUE_OBJECTS})
endif ()

# Set include directories for the main library
target_include_directories(magique PUBLIC ${MAGIQUE_PUBLIC_INCLUDE})

# ----------------------------------------------------------------------
# Linking Binaries and External Dependencies
# ----------------------------------------------------------------------

# Link against the GameNetworkingSockets or the Steam SDK and platform-specific libraries
include(LinkBinaries)
include(LinkRaylib)

# ----------------------------------------------------------------------
# End of CMake Configuration
# ----------------------------------------------------------------------

message(STATUS "/n-- ---------------------------------------")