# ----------------------------------------------------------------------
# CMake Configuration for magique
# ----------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)
project(magique
        LANGUAGES CXX C
        DESCRIPTION "2D game engine for programmers (or those yet to be!)"
        VERSION 0.9.10
        HOMEPAGE_URL "https://github.com/gk646/magique"
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 23)

# Set CMake modules directory - path searched when using include()
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ----------------------------------------------------------------------
# Configuration Options
# ----------------------------------------------------------------------

# Options - IF USING CMAKE CACHE, CHANGES NEED A CACHE RESET TO APPLY!
# Values - Use set(MAGIQUE_LOGIC_TICKS 60) in your CMakeLists.txt BEFORE including magique to change options
# The options here are just defaults, if set before your value takes precedence

option(MAGIQUE_STEAM "Enable Steam integration (requires STEAM_PATH)" OFF)
option(MAGIQUE_LAN "Enable local networking" OFF)
option(MAGIQUE_SHARED "Build Magique as a shared library (experimental)" OFF)
option(MAGIQUE_SANITIZER "Compile with address sanitizer" OFF)
set(MAGIQUE_LOGIC_TICKS 60 CACHE STRING "Logic ticks per second")
# 0 threads is supported - then all jobs will be executed on the main thread instantly
# This is more efficient for the CPU and saves battery (preferred for mobile devices)
set(MAGIQUE_WORKER_THREADS 2 CACHE STRING "Number of worker threads")
set(MAGIQUE_COLLISION_CELL_SIZE 32 CACHE STRING "Size of a grid cell for collision detection")
set(MAGIQUE_MAX_ENTITIES_CELL 24 CACHE STRING "Maximum amount of entities allowed per cell")
set(MAGIQUE_PATHFINDING_CELL_SIZE 16 CACHE STRING "Coarseness/size of the pathfinding grid")
set(MAGIQUE_MAX_PATH_SEARCH_CAPACITY 1024 CACHE STRING "Maximum amount of tiles evaluated in a single search")
set(MAGIQUE_CHECK_EXISTS_BEFORE_EVENT 1 CACHE BOOL "Check if entity exists before calling collision method")
set(MAGIQUE_TEXTURE_ATLAS_SIZE 8192 CACHE STRING "Maximum width of any texture atlas")
set(MAGIQUE_TILE_OBJECT_CUSTOM_PROPERTIES 2 CACHE STRING "Maximum amount of custom tile properties for each tile object")
set(MAGIQUE_TILE_SET_CUSTOM_PROPERTIES 4 CACHE STRING "Maximum amount of custom tile properties for each tile in a tileset")
set(MAGIQUE_MAX_ANIM_FRAMES 16 CACHE STRING "Maximum amount of frames per single animation")
set(MAGIQUE_MAX_PLAYERS 4 CACHE STRING "Maximum amount of actors (players) supported")
set(MAGIQUE_MAX_LOBBY_MESSAGE_LEN 256 CACHE STRING "Maximum length of a chat message sent in a lobby")
set(MAGIQUE_PARTICLE_COLORPOOL_SIZE 5 CACHE STRING "Maximum size of the color pool for particles")
set(MAGIQUE_MAX_FORMAT_LEN 64 CACHE STRING "Maximum length of formats and string values")
set(MAGIQUE_MAX_SUPPORTED_TIMERS 4 CACHE STRING "Maximum amount of timers supported at the same time")
set(MAGIQUE_INCLUDE_FONT 1 CACHE BOOL "Include the default magique font: Cascadia Code")

include(ParseOptions)      # Parses options and sets parameters

# ----------------------------------------------------------------------
# Collect and Organize Object Files
# ----------------------------------------------------------------------

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# ----------------------------------------------------------------------
# Build the Main Magique Library
# ----------------------------------------------------------------------

# Choose between building a shared or static library
if (MAGIQUE_SHARED)
    add_library(magique SHARED ${SRC_FILES})
else ()
    add_library(magique STATIC ${SRC_FILES})
endif ()

include(CompileOptions)

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