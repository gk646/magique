// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CONFIG_H
#define MAGIQUE_CONFIG_H

//===============================================
// Config
//===============================================
// .....................................................................
// This allows to configure magique at compile time
// 1: to enable an option
// 0: to disable an option
// .....................................................................

//================= CORE =================//

// Logic ticks per second
#define MAGIQUE_LOGIC_TICKS (60)

// Time taken for a single update tick (on average)
#define MAGIQUE_TICK_TIME (1.0F / MAGIQUE_LOGIC_TICKS)

// Main Thread + 2 (Worker) = 3 total threads / 95% of steam users have 4 physical cores
#define MAGIQUE_WORKER_THREADS (2)

// Controls the maximum length of names for various things:
// window names in the WindowManager, column names in the DataTable, children names in the UIContainer
#define MAGIQUE_MAX_NAMES_LENGTH (16)

//================= PERFORMANCE =================//

// Enables usage of SIMD (faster)
#define MAGIQUE_SIMD 1

// Estimated number of unique maps - used as static buffer size (if above allocation happens)
#define MAGIQUE_EXPECTED_MAPS (32)

// The size of a grid cell (a square)
// Should be chosen as SMALL as possible, such that most objects fit within the given size
// If the size of the bounding box of objects is bigger than 2*cellSize it gets "slow"
// Note: Must be a power of two (32,64,128 -> shouldn't be bigger than that)
#define MAGIQUE_COLLISION_CELL_SIZE (32)

// Maximum amount of entities allowed per cell (less is better) - if more they are not collision checked anymore
// Also used for static collision objects!
#define MAGIQUE_MAX_ENTITIES_CELL (24)

// Sets the coarseness/size of the pathfinding grid
// The smaller, the more accurate the pathing but the longer it takes to calculate
// Note: Must be a power of two (16,32,64 -> shouldn't be bigger than that)
#define MAGIQUE_PATHFINDING_CELL_SIZE (8)

// Controls the maximum amount of tiles that can be evaluated in a single search
// Note: This is the total number of search iterations - you can specify a path length limit individually
#define MAGIQUE_MAX_PATH_SEARCH_CAPACITY (1024)

// Checks if the entity still exists before calling the collision method
// This incurs some overhead but makes it easier to avoid accessing deleted entities
#define MAGIQUE_CHECK_EXISTS_BEFORE_EVENT 1

//================= ASSETS =================//

// Maximum width of any texture atlas
#define MAGIQUE_TEXTURE_ATLAS_SIZE (8192)

// Maximum amount of tile layers in TileMaps
#define MAGIQUE_MAX_TILE_LAYERS (3)

// Maximum amount of object layers in TileMaps
#define MAGIQUE_MAX_OBJECT_LAYERS (2)

// Maximum amount of custom tile properties for each tile object (inside Tiled)
#define MAGIQUE_TILE_OBJECT_CUSTOM_PROPERTIES (2)
// Maximum amount of custom tile properties for each tile in a tileset
#define MAGIQUE_TILE_SET_CUSTOM_PROPERTIES (4)

// Maximum amount of columns supported for a imported csv file
#define MAGIQUE_MAX_CSV_COLUMNS (16)

// Maximum amount of frames per single animation - used for duration array so each frame can have different duration
#define MAGIQUE_MAX_ANIM_FRAMES (16)

//================= GAMEDEV =================//

//================= MULTIPLAYER =================//

// Maximum amount of actors (players) supported - also applies for networking (maximum amount of clients for the host)
#define MAGIQUE_MAX_PLAYERS (4)

// Estimated multiplayer messages being sent each tick
#define MAGIQUE_ESTIMATED_MESSAGES (150)

// Maximum length of a chat message sent in a lobby
#define MAGIQUE_MAX_LOBBY_MESSAGE_LEN (256)

//================= GRAPHICS =================//

// Maximum amount of entities being raytraced
#define MAGIQUE_RAYTRACING_ENTITIES (50)

// Maximum size of the color pool for particles
#define MAGIQUE_PARTICLE_COLORPOOL_SIZE (5)


//================= PERSISTENCE =================//

//================= UI =================//

//================= UTIL =================//

// Enables profiling - generates average times for update and render ticks
#define MAGIQUE_PROFILING 1

// Maximum length of formats and string values (SetFormatValue()))
#define MAGIQUE_MAX_FORMAT_LEN (64)

// Maximum amount of timers supported at the same time
#define MAGIQUE_MAX_SUPPORTED_TIMERS (4)

//================= COMPILATION =================//

// Includes the default magique font: Cascadia Code
#define MAGIQUE_INCLUDE_FONT 1

//================= INTERNAL =================//

#if !defined(NDEBUG)
#define MAGIQUE_DEBUG
#endif

#define IS_POWER_OF_TWO(x) (((x) != 0) && ((x) & ((x) - 1)) == 0)

#if IS_POWER_OF_TWO(MAGIQUE_COLLISION_CELL_SIZE)
#else
#error "Collision Cell size is not a power of 2. Choose from: 32,64, 128"
#endif

#if IS_POWER_OF_TWO(MAGIQUE_PATHFINDING_CELL_SIZE)
#else
#error "PathFinding Cell size is not a power of 2. Choose from: 8,16,32"
#endif

#define MAGIQUE_WORLD_BOUND_DEPTH (100)
#define MAGIQUE_LOBBY_PACKET_TYPE (static_cast<MessageType>(UINT8_MAX))
#define MAGIQUE_BATCHED_PACKET_TYPE (static_cast<MessageType>(UINT8_MAX - 1))

#endif //MAGIQUE_CONFIG_H
