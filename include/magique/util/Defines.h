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

#define MAGIQUE_VERSION "0.0.2"
#define MAGIQUE_DEBUG 1
#define MAGIQUE_LOGIC_TICKS 60 // 60 Ticks per second

//----------------- STEAM -----------------//

// add "set(MAGIQUE_STEAM ON)" to your CMakeLists.txt before including magique to enable steam features
#define MAGIQUE_STEAM 0

//----------------- MULTIPLAYER -----------------//

#define MAGIQUE_MAX_PLAYERS 4         // Needs to be set to the maximum amount of players
#define MAGIQUE_MESSAGES_ESTIMATE 150 // Estimate how many multiplayer messages are sent each tick

//----------------- ASSETS -----------------//

#define MAGIQUE_TEXTURE_ATLAS_WIDTH 8192  // Height of each texture atlas
#define MAGIQUE_TEXTURE_ATLAS_HEIGHT 8192 // Width of each texture atlas
#define MAGIQUE_DIRECT_HANDLES 15000      // Allows to store 15k direct handles
#define MAGIQUE_MAX_TILEMAP_LAYERS 5      // Maximum amount of layers for tilemaps
#define MAGIQUE_MAX_EXPECTED_MAPS 32      // Maximum amount of expected maps

//----------------- LOADING -----------------/  /

#define MAGIQUE_MAX_TABLE_NAME_SIZE 16

//----------------- GRAPHICS -----------------//

#define MAGIQUE_MAX_RAYTRACING_ENTITIES 50


//----------------- BRANCHES -----------------//

#if MAGIQUE_DEBUG == 1
//#define MAGIQUE_DEBUG_COLLISIONS  // Uses O(n**2) collision checking for double checking
#define MAGIQUE_DEBUG_ENTITIES
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

#endif //MAGIQUE_DEFINES_H