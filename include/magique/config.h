// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CONFIG_H
#define MAGIQUE_CONFIG_H

//===============================================
// Config
//===============================================
// .....................................................................
// The config options move to cmake/Config.cmake
// To change options add set them BEFORE including magique in your CMakeLists.txt:
//        set(<Name> <Value>)
// e.g.   set(MAGIQUE_MAX_PLAYERS 4)
// e.g.   set(MAGIQUE_PATHFINDING_CELL_SIZE 16)
//
// .....................................................................

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

#endif //MAGIQUE_CONFIG_H
