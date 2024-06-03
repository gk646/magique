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

#define MAGIQUE_MULITPLAYER_SUPPORT 0 // 1 to enable multiplayer
#define MAGIQUE_LOGIC_TICKS 60        // 60 Ticks per second

//-----------------MULTIPLAYER-----------------//

#define MAGQIQUE_MAX_PLAYERS 4

// 1 to Enable Debug
#if defined(_DEBUG) || !defined(NDEBUG)
#define MAGIQUE_DEBUG 1
#else
#define MAGIQUE_DEBUG 0
#endif

#endif //MAGIQUE_DEFINES_H