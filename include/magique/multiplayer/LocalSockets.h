#ifndef MAGIQUE_LOCAL_MULTIPLAYER_H
#define MAGIQUE_LOCAL_MULTIPLAYER_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Local Sockets Module
//-----------------------------------------------
// .....................................................................
// This module is only for local multiplayer (LAN) - it uses valve's open source GameNetworkingSockets
// If steam integration is disabled only local sockets can be used!
// Note: To use global multiplayer look at steam/GlobalSockets
// Note: This interface can still be used for local multiplayer when steam integration is enabled!
// .....................................................................

namespace magique
{
    // Initializes local multiplayer - HAS to be called MANUALLY before any other local multiplayer methods
    bool InitLocalMultiplayer();

    //----------------- HOST -----------------//

    // Creates a LAN-socket so others can connect to you  on the given port - makes you the host
    // Returns true if it was successful
    bool CreateLocalSocket(int port);

    // Closes the listening socket - optionally specify a close code or string that can be read on the receiver
    // Returns true if an existing connection was closed successfully
    bool CloseLocalSocket(int closeCode = 0, const char* closeReason = nullptr);

    //----------------- CLIENT -----------------//

    // Connects to an open local socket at the given ip and port
    //      - ip: the ip address and port as string in the form XXX.XXX.X.XX:port (like minecraft, normal ip4)
    Connection ConnectToLocalSocket(const char* ip);

    // Disconnects from the socket (if any)
    // Optionally specify a close code or string that can be read on the receiver
    // Returns true if an existing connection was closed successfully
    bool DisconnectFromLocalSocket(int closeCode = 0, const char* closeReason = nullptr);

} // namespace magique


#endif //MAGIQUE_LOCAL_MULTIPLAYER_H