#ifndef MAGIQUE_CONFIG_H
#define MAGIQUE_CONFIG_H

//-----------------------------------------------
// Config
//-----------------------------------------------
// .....................................................................
// This allows to configure magique at compile time
// 1: to enable an option
// 0: to disable an option
// .....................................................................

//----------------- CORE -----------------//

// Logic ticks per second
#define MAGIQUE_LOGIC_TICKS 60

// Main Thread + 3 (Worker) = 4 total threads / 95% of steam users have 4 pyhsical cores
#define MAGIQUE_WORKER_THREADS 3

//----------------- PERFORMANCE -----------------//

// Enables usage of SIMD (faster)
#define MAGIQUE_SIMD 1

// Estimated number of unique maps
#define MAGIQUE_EXPECTED_MAPS 32

// The size of a grid cell (a square) - MUST be a power of two (32,64,128 -> shouldnt be bigger than that)
// Should be chosen as SMALL as possible, 75% of objects should fit within the given size
// If the size of the bounding box of the object is bigger than 2*cellSize it gets slow
#define MAGIQUE_COLLISION_CELL_SIZE 64

// Maximum amount of entities allowed per cell (less is better)
#define MAGIQUE_MAX_ENTITIES_CELL 31

// Sets the coarseness of the pathfinding grid
// The smaller the more accurate the pathing but the longer it takes to calculate
#define MAGIQUE_PATHFINDING_CELL_SIZE 16


//----------------- ASSETS -----------------//

// Maximum width of any texture atlas
#define MAGIQUE_TEXTURE_ATLAS_SIZE 8192

// Maximum amount of tile layers in tilemaps
#define MAGIQUE_MAX_TILE_LAYERS 3

// Maximum amount of object layers in tilemaps
#define MAGIQUE_MAX_OBJECT_LAYERS 2


//----------------- UI -----------------//

// Logical width the UI system
#define MAGIQUE_UI_RESOLUTION_X 1920.0F

// Logical height the UI system
#define MAGIQUE_UI_RESOLUTION_Y 1080.0F


//----------------- MULTIPLAYER -----------------//

// Maximum amount of actors supported
#define MAGIQUE_MAX_PLAYERS 4

// Estimated multiplayer messages being sent each tick
#define MAGIQUE_ESTIMATED_MESSAGES 150


//----------------- GRAPHICS -----------------//

// Maximum amount of entities being raytraced
#define MAGIQUE_RAYTRACING_ENTITIES 50

// Maximum size of the color pool for particles
#define MAGIQUE_PARTICLE_COLORPOOL_SIZE 5


//----------------- PERSISTENCE -----------------//

// Maximum length of column names for the datatable
#define MAGIQUE_TABLE_NAME_LEN 16


//----------------- UTIL -----------------//

// Enables logging - error and fatal messages are always enabled
#define MAGIQUE_LOGGING 1

// Enables profiling - generates average times for update and render ticks
#define MAGIQUE_PROFILING 1

// Maximum length of formats and string values (SetFormatValue()))
#define MAGIQUE_MAX_FORMAT_LEN 64


//----------------- COMPILATION -----------------//

// Tries to forward declare std::function - only possible on MSVC
#define MAGIQUE_FORWARD_FUNCTION 1


//----------------- IMPLEMENTATION -----------------//

#if MAGIQUE_FORWARD_FUNCTION == 1
namespace std
{
    template <class>
    class function;
} // namespace std
#endif

#if !defined(NDEBUG)
#define MAGIQUE_DEBUG
#endif

#ifdef MAGIQUE_USE_STEAM
#define MAGIQUE_STEAM 1
#else
#define STEAMNETWORKINGSOCKETS_STANDALONELIB
#endif

#define IS_POWER_OF_TWO(x) (((x) != 0) && ((x) & ((x)-1)) == 0)

#if IS_POWER_OF_TWO(MAGIQUE_COLLISION_CELL_SIZE)
#else
#error "Collision Cell size is not a power of 2. Choose from: 32,64, 128"
#endif

#if IS_POWER_OF_TWO(MAGIQUE_PATHFINDING_CELL_SIZE)
#else
#error "PathFinding Cell size is not a power of 2. Choose from: 8,16,32"
#endif

#endif //MAGIQUE_CONFIG_H