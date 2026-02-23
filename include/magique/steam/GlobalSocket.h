// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STEAM_GLOBAL_SOCKETS_H
#define MAGIQUE_STEAM_GLOBAL_SOCKETS_H

#include <magique/core/Types.h>

//===============================================
// Global Sockets Module
//===============================================
// .....................................................................
// Note: This module needs steam to be enabled (via CMake: MAGIQUE_STEAM)
// This module is only for global multiplayer - it uses the steam backend with P2P networking
// This allows seamless multiplayer with any steam user anywhere in the world out of the box!
// Note: If you create a global socket you should use steam/Lobbies.h to build a session!
// Intended Workflow to create a global multiplayer session:
//      1. Create a global socket
//      2. Create a game lobby
//      3. Let clients join the lobby and connect to you when they enter the lobby!
// .....................................................................

namespace magique
{
    // Returns true if global multiplayer was initialized successfully
    // Note: HAS to be called MANUALLY before any other global multiplayer methods
    bool GlobalSocketInit();

    // Creates a global steam sockets so others can connect to you - makes you the host
    // Returns true if it was successful
    bool GlobalSocketCreate();

    // Connects to a global socket via the steam id
    // Note: To get the steam id of the clients you usually have to create a lobby
    Connection GlobalSocketConnect(SteamID steamID);

} // namespace magique


#endif //MAGIQUE_STEAM_GLOBAL_SOCKETS_H