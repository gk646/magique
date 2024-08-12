#ifndef MAGIQUE_LOCAL_MULTIPLAYER_H
#define MAGIQUE_LOCAL_MULTIPLAYER_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Local Multiplayer Module
//-----------------------------------------------
// .....................................................................
// This module is only for local multiplayer (LAN) - it uses valve's open source GameNetworkingSockets
// If steam integration is disabled this is the only multiplayer module!
// Note: To use global multiplayer look at steam/SteamMultiplayer
// Note: This interface can still be used for local multiplayer when steam integration is enabled!
// .....................................................................

namespace magique
{
    // Initializes local multiplayer - HAS to be called MANUALLY before any other local multiplayer methods
    bool InitLocalMultiplayer();

    //----------------- HOST -----------------//

    // Creates a LAN-socket so others can connect to you - using this makes you the host
    // Returns true if it was successful
    bool CreateLocalSocket(const char* adress);

    // Closes the listening socket - optionally specify a close code or string that can be read on the receiver
    // Returns true if an existing connection was closed successfully
    bool CloseLocalSocket(int closeCode = 0, const char* closeReason = nullptr);

    //----------------- CLIENT -----------------//

    // Connects to an open socket on that port
    LocalConnection ConnectLocalSocket(int port);

    // Disconnects from the socket (if any)
    // Optionally specify a close code or string that can be read on the receiver
    // Returns true if and existing connection was closed successfully
    bool DisconnectLocalSocket(int closeCode = 0, const char* closeReason = nullptr);

    //----------------- MESSAGES -----------------//

    // Starts a new batch or appends to an existing one - batches until SendLocalBatch() is called
    // Note: there is only be 1 batch at a time! - copies the data you pass in (allows for stack and dynamic memory)
    // Each message can be sent to a different connection (as specified by the connection)
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool BatchLocalMessage(LocalConnection conn, const void* message, int size, SendFlag flag = SendFlag::RELIABLE);

    // Sends the current batch if it exists
    // Failure: returns false if BatchLocalMessage() was not called at least once before with valid parameters
    bool SendLocalBatch();

    // Directly sends the message - should only be used for single messages else use BatchMessage() and SendBatch()
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool SendLocalMessage(LocalConnection conn, const void* message, int size, SendFlag flag = SendFlag::RELIABLE);

    //----------------- UTIL -----------------//

    uint32_t GetIPAdress();

    // Sets the callback function that is called on various local multiplayer events
    // See the MultiplayerEvent enum for more info and type of events
    void SetLocalMultiplayerCallback(const std::function<void(MultiplayerEvent event)>& func);

    // Returns true if currently the host, false if the client
    bool IsHost();

} // namespace magique


#endif //MAGIQUE_LOCAL_MULTIPLAYER_H