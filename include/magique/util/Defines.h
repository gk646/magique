#ifndef MAGIQUE_DEFINES_H
#define MAGIQUE_DEFINES_H

//-----------------------------------------------
// Defines
//-----------------------------------------------
// ................................................................................
// These are core engine defines and shouldnt be changed unless you know what it does
// After any changes the library has to be recompiled!
// It can be useful to use these in your code if you need access to them
// ................................................................................

//----------------- CORE -----------------//

#define MAGIQUE_VERSION "0.0.6"  // Version number
#define MAGIQUE_DEBUG 1          // 1 to enable debug - 0 to disable
#define MAGIQUE_LOGIC_TICKS 60   // Logic ticks per second
#define MAGIQUE_WORKER_THREADS 3 // Main Thread + 3 (Worker) = 4 total threads / 95% of steam users have 4 pyhsical cores

//----------------- STEAM -----------------//

// add "set(MAGIQUE_USE_STEAM ON)" to your CMakeLists.txt before including magique to enable steam features
#define MAGIQUE_STEAM 0

//----------------- MULTIPLAYER -----------------//

#define MAGIQUE_MAX_PLAYERS 4         // Maximum amount of actors supported
#define MAGIQUE_MESSAGES_ESTIMATE 200 // Estimate how many multiplayer messages are sent each tick

//----------------- ASSETS -----------------//

#define MAGIQUE_TEXTURE_ATLAS_WIDTH 8192  // Height of each texture atlas
#define MAGIQUE_TEXTURE_ATLAS_HEIGHT 8192 // Width of each texture atlas
#define MAGIQUE_DIRECT_HANDLES 15000      // Allows to store 15k direct handles
#define MAGIQUE_MAX_TILEMAP_LAYERS 5      // Maximum amount of layers for tilemaps
#define MAGIQUE_MAX_EXPECTED_MAPS 32      // Maximum amount of expected maps

//----------------- UI -----------------//

#define MAGIQUE_UI_RESOLUTION_X 1920.0F // Logical resolution of 1920x1080 since its the most common
#define MAGIQUE_UI_RESOLUTION_Y 1080.0F // Logical resolution of 1920x1080 since its the most common
#define MAGIQUE_MAX_FORMAT_LEN 64       // Limits both placeholders and string values

//----------------- PERSISTENCE -----------------//

#define MAGIQUE_MAX_TABLE_NAME_SIZE 16

//----------------- GRAPHICS -----------------//

#define MAGIQUE_MAX_RAYTRACING_ENTITIES 50

//----------------- PERFORMANCE -----------------//

#define MAGIQUE_USE_SIMD // up to AVX2 -> supported by 93% of steam users (Intel since 2013, AMD since 2015)

//----------------- CONDITIONALS -----------------//

#if MAGIQUE_DEBUG == 1
//#define MAGIQUE_DEBUG_COLLISIONS  // Uses O(n**2) collision checking for double checking
#define MAGIQUE_DEBUG_PROFILE
#endif

#if defined(_DEBUG) || !defined(NDEBUG)
#define MAGIQUE_DEBUG 1
#else
#undef MAGIQUE_DEBUG
#define MAGIQUE_DEBUG 0
#endif

#if MAGIQUE_CMAKE_USE_STEAM
#define MAGIQUE_STEAM 1
#else
#define STEAMNETWORKINGSOCKETS_STANDALONELIB
#define MAGIQUE_STEAM 0
#endif

#if MAGIQUE_NO_SIMD // Remove simd usage
#undef MAGIQUE_USE_SIMD
#endif

#endif //MAGIQUE_DEFINES_H