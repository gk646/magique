#ifndef MAGIQUE_DEFINES_H
#define MAGIQUE_DEFINES_H

//-----------------------------------------------
// Defines
//-----------------------------------------------
// .....................................................................
// These are core engine defines which MUST not be changed
// After any changes the library has to be recompiled!
// It can be useful to use these in your code if you need access to them
// .....................................................................

//----------------- CORE -----------------//

#define MAGIQUE_LOGIC_TICKS 60 // 60 Ticks per second

//----------------- MULTIPLAYER -----------------//

#define MAGIQUE_MULITPLAYER_SUPPORT 0 // 1 to enable multiplayer
#define MAGIQUE_MAX_PLAYERS 4

//----------------- ASSETS -----------------//

#define MAGIQUE_TEXTURE_ATLAS_WIDTH 8192  // Height of each texture atlas
#define MAGIQUE_TEXTURE_ATLAS_HEIGHT 8192 // Width of each texture atlas
#define MAGIQUE_DIRECT_HANDLES 15000      // Allows to store 15k direct handles (for tile maps)
#define MAGIQUE_MAX_TILEMAP_LAYERS 5

//----------------- GRAPHICS -----------------//

#define MAGIQUE_MAX_RAYTRACING_ENTITIES 50

// 1 to Enable Debug
#if defined(_DEBUG) || !defined(NDEBUG)
#define MAGIQUE_DEBUG 1
#else
#define MAGIQUE_DEBUG 0
#endif


#endif //MAGIQUE_DEFINES_H