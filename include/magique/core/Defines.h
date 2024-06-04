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
#define MAGQIQUE_MAX_PLAYERS 4


//----------------- ASSETS -----------------//

#define MAGIQUE_TEXTURE_ATLAS_WIDTH 4096     // Height of each texture atlas
#define MAGIQUE_TEXTURE_ATLAS_HEIGHT 4096    // Width of each texture atlas
#define MAGIQUE_DIRECT_TEXTURE_STORAGE 15000 // Allows to store 15k textures statically, accessed per id (for tile maps)

// 1 to Enable Debug
#if defined(_DEBUG) || !defined(NDEBUG)
#define MAGIQUE_DEBUG 1
#else
#define MAGIQUE_DEBUG 0
#endif


#endif //MAGIQUE_DEFINES_H