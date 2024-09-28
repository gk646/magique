#ifndef MAGIQUE_GLOBAL_SOCKETS_H
#define MAGIQUE_GLOBAL_SOCKETS_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Global Sockets Module
//-----------------------------------------------
// .....................................................................
// Note: This module needs steam to be enabled (via CMake: MAGIQUE_STEAM)
// This module is only for global multiplayer - it uses the steam backend with P2P networking
// This allows seamless multiplayer with any steam user anywhere in the world out of the box!
// Note: See steam/Steam.h for helper methods and steam/Lobbies.h to create game lobbies
// .....................................................................

namespace magique
{

    // Returns true if global multiplayer was initialized successfully
    // Note: HAS to be called MANUALLY before any other global multiplayer methods
    bool InitGlobalMultiplayer();

    //----------------- HOST -----------------//

    // Creates a global sockets so others can connect to you  on the given port - makes you the host
    // Returns true if it was successful
    bool CreateGlobalSocket();

    // Closes the listening socket - optionally specify a close code or string that can be read on the receiver
    // Returns true if an existing connection was closed successfully
    bool CloseGlobalSocket(int closeCode = 0, const char* closeReason = nullptr);

    //----------------- CLIENT -----------------//

    // Connects to a global local socket at the given ip and port
    //      - ip: the ip address and port as string in the form XXX.XXX.X.XX:port (like minecraft, normal ip4)
    Connection ConnectToGlobalSocket(SteamID steamID);

    // Disconnects from the socket (if any)
    // Optionally specify a close code or string that can be read on the receiver
    // Returns true if an existing connection was closed successfully
    bool DisconnectFromGlobalSocket(int closeCode = 0, const char* closeReason = nullptr);

} // namespace magique


#endif //MAGIQUE_GLOBAL_SOCKETS_H