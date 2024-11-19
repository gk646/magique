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

// Main Thread + 3 (Worker) = 4 total threads / 95% of steam users have 4 physical cores
#define MAGIQUE_WORKER_THREADS 3

// Controls the maximum length of names for various things:
// window names in the WindowManager, column names in the DataTable, children names in the UIContainer
#define MAGIQUE_MAX_NAMES_LENGTH 16

//----------------- PERFORMANCE -----------------//

// Enables usage of SIMD (faster)
#define MAGIQUE_SIMD 1

// Estimated number of unique maps
#define MAGIQUE_EXPECTED_MAPS 32

// The size of a grid cell (a square) - MUST be a power of two (32,64,128 -> shouldn't be bigger than that)
// Should be chosen as SMALL as possible, 75% of objects should fit within the given size
// If the size of the bounding box of the object is bigger than 2*cellSize it gets slow
#define MAGIQUE_COLLISION_CELL_SIZE 64

// Maximum amount of entities allowed per cell (less is better)
#define MAGIQUE_MAX_ENTITIES_CELL 31

// Sets the coarseness of the pathfinding grid
// The smaller, the more accurate the pathing but the longer it takes to calculate
#define MAGIQUE_PATHFINDING_CELL_SIZE 32

// Controls the maximum amount of tile that can be evaluated in a single search
// Note: This is the global maximum, a specific maximum can be set on each query
#define MAGIQUE_PATHFINDING_SEARCH_CAPACITY 1024

// Checks if the entity still exists before calling the collision method
// This incurs some overhead but makes it easier to avoid accessing deleted entities
#define MAGIQUE_CHECK_EXISTS_BEFORE_EVENT 1

//----------------- ASSETS -----------------//

// Maximum width of any texture atlas
#define MAGIQUE_TEXTURE_ATLAS_SIZE 8192

// Maximum amount of tile layers in TileMaps
#define MAGIQUE_MAX_TILE_LAYERS 3

// Maximum amount of object layers in TileMaps
#define MAGIQUE_MAX_OBJECT_LAYERS 2

// Maximum amount of custom tile properties for each tile object (inside Tiled)
#define MAGIQUE_TILE_OBJECT_CUSTOM_PROPERTIES 2

//----------------- GAMEDEV -----------------//



//----------------- MULTIPLAYER -----------------//

// Maximum amount of actors (players) supported - also applies for networking (maximum amount of clients for the host)
#define MAGIQUE_MAX_PLAYERS 4

// Estimated multiplayer messages being sent each tick
#define MAGIQUE_ESTIMATED_MESSAGES 150

// Maximum length of a chat message sent in a lobby
#define MAGIQUE_MAX_LOBBY_MESSAGE_LENGTH 512

//----------------- GRAPHICS -----------------//

// Maximum amount of entities being raytraced
#define MAGIQUE_RAYTRACING_ENTITIES 50

// Maximum size of the color pool for particles
#define MAGIQUE_PARTICLE_COLORPOOL_SIZE 5


//----------------- PERSISTENCE -----------------//


//----------------- UI -----------------//

// Logical width the UI system
#define MAGIQUE_UI_RESOLUTION_X 1920.0F

// Logical height the UI system
#define MAGIQUE_UI_RESOLUTION_Y 1080.0F

//----------------- UTIL -----------------//

// Enables logging - error and fatal messages are always enabled
#define MAGIQUE_LOGGING 1

// Enables profiling - generates average times for update and render ticks
#define MAGIQUE_PROFILING 1

// Maximum length of formats and string values (SetFormatValue()))
#define MAGIQUE_MAX_FORMAT_LEN 64

// Maximum amount of timers supported at the same time
#define MAGIQUE_MAX_SUPPORTED_TIMERS 4

//----------------- COMPILATION -----------------//

// Forward declares std::function which improves compile time about 150ms per TU - only on MSVC
#define MAGIQUE_FORWARD_FUNCTION 1

//----------------- INTERNAL -----------------//

#if MAGIQUE_FORWARD_FUNCTION == 1 && defined(_MSC_VER)
namespace std
{
    template <class>
    class function;
} // namespace std
#endif

#if !defined(NDEBUG)
#define MAGIQUE_DEBUG
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

#define MAGIQUE_WORLD_BOUND_DEPTH 100


#endif //MAGIQUE_CONFIG_H