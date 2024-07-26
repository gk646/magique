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

    //----------------- SETUP -----------------//

    // Creates a socket so others can connect to you - using this makes you the host
    // Uses IP4/IP6 if steamfeatures are off - otherwise P2P
    Connection CreateListenSocket();

    // Connects to a opened socket - using this makes you the client
    Connection ConnectToSocket();

    //----------------- MESSAGES -----------------//

    // Starts a new batch or appends to an existing one - can be called as many times as needed and batches until SendBatch() is called
    // Note: there can only be 1 batch at a time! - copies the data you pass in (allows for stack and dynamic memory)
    // Each message can be sent to a different connection as specified within a batch
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool BatchMessage(Connection conn, const void* message, int size, SendFlag flag = SendFlag::RELIABLE);

    // Sends the current batch if it exists
    // Failure: returns false if BatchMessage() was not called at least once before with valid parameters
    bool SendBatch();

    // Directly sends the message - should only be used for single messages else use BatchMessage() and SendBatch()
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool SendMessage(Connection conn, const void* message, int size, SendFlag flag = SendFlag::RELIABLE);


    //----------------- MISC -----------------//

    // True if your a host
    bool IsHost();


} // namespace magique


#endif //MAGIQUE_MULTIPLAYER_H