#ifndef MAGIQUE_MULTIPLAYER_H
#define MAGIQUE_MULTIPLAYER_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Multiplayer Module
//-----------------------------------------------
// .....................................................................
// Per DEFAULT steam features are turned OFF! This means connections only over IP4 or IP6
// Setting "set(STEAMWORKS ON)" in cmake before including magique enables all steam features
// and secure P2P connection over the steam relay network.
//
// This means that the behavior of these multiplayer functions changes with configuration but the interface does not
// .....................................................................

namespace magique
{
    // Initializes multiplayer - HAS to be called MANUALLY before any other multiplayer methods
    // Automatically creates the test steam_appid.txt file with id 480
    bool InitMultiplayer();

    //----------------- HOST -----------------//

    // Creates a socket so others can connect to you - using this makes you the host
    // Uses IP4/IP6 if steamfeatures are off - otherwise P2P
    // Returns true if it was successful
    bool CreateListenSocket(const char* adress);

    // Closes the existing listening socket
    // Optionally specify a close code or string that can be read on the receiver
    // Returns true if and existing connection was closed successfully
    bool CloseListenSocket(int closeCode = 0, const char* closeReason = nullptr);

    //----------------- CLIENT -----------------//

    // Connects to a opened socket - using this makes you the client
#ifdef MAGIQUE_STEAM == 1
    Connection ConnectToSocket(SteamID steamID);
#else
    Connection ConnectToSocket(const char* adress);
#endif

    // Disconnects from the socket (if any)
    // Optionally specify a close code or string that can be read on the receiver
    // Returns true if and existing connection was closed successfully
    bool DisconnectFromSocket(int closeCode = 0, const char* closeReason = nullptr);

    //----------------- MESSAGES -----------------//

    // Starts a new batch or appends to an existing one - can be called as many times as needed and batches until SendBatch() is called
    // Note: there can only be 1 batch at a time! - copies the data you pass in (allows for stack and dynamic memory)
    // Each message can be sent to a different connection
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool BatchMessage(Connection conn, const void* message, int size, SendFlag flag = SendFlag::RELIABLE);

    // Sends the current batch if it exists
    // Failure: returns false if BatchMessage() was not called at least once before with valid parameters
    bool SendBatch();

    // Directly sends the message - should only be used for single messages else use BatchMessage() and SendBatch()
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool SendMessage(Connection conn, const void* message, int size, SendFlag flag = SendFlag::RELIABLE);

    //----------------- CALLBACKS -----------------//

    void SetOnNewConnectionCallback(const std::function<void(Connection newConnection)>& func);

    //----------------- UTIL -----------------//

    // True if your a host
    bool IsHost();

} // namespace magique


#endif //MAGIQUE_MULTIPLAYER_H