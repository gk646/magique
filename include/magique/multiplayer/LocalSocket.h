// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_LOCAL_SOCKETS_H
#define MAGIQUE_LOCAL_SOCKETS_H

#include <magique/core/Types.h>

//===============================================
// Local Sockets Module
//===============================================
// .....................................................................
// This module is only for local multiplayer (LAN) - it uses valve's open source GameNetworkingSockets
// If steam integration is disabled only local sockets can be used!
// Note: To use global multiplayer look at steam/GlobalSockets
// Note: This interface can still be used for local multiplayer when steam integration is enabled!
// .....................................................................

namespace magique
{
    // Returns true if local multiplayer was initialized successfully
    // Note: HAS to be called MANUALLY before any other local multiplayer methods
    bool LocalSocketInit();

    // Creates a LAN-socket so others can connect to you on the given port - makes you the host
    // Returns true if it was successful
    bool LocalSocketCreate(uint16_t port = 60000);

    // Connects to an open local socket at the given ip and port
    //      - ip : the ip address in the form XXX.XXX.X.XX (normal ip4)
    //      - port: a number between 0 and 65536
    // Note: On success the connection is stored internally as well and can be accessed via GetCurrentConnections()
    Connection LocalSocketConnect(const char* ip, uint16_t port = 60000);

    // Returns a pointer to the local ip as a string - always save to access and cached after the first call
    // Failure: returns nullptr if it's not possible to retrieve the ip
    const char* LocalSocketGetIP();

} // namespace magique

#endif //MAGIQUE_LOCAL_SOCKETS_H