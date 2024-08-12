#ifndef MAGIQUE_MULTIPLAYER_H
#define MAGIQUE_MULTIPLAYER_H

#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// Multiplayer Module
//-----------------------------------------------
// .....................................................................
// These methods are shared for both local and global sockets.
// .....................................................................

namespace magique
{
    //----------------- MESSAGES -----------------//

    // Returns a network message object to be sent via the network - should be used directly and not stored
    // The type is very useful for correctly handling the message on the receivers end (e.g. HEALTH_UPDATE, POS_UPDATE, ...)
    // Note: The passed data will be copied when batching or sending so you should pass it directly (stack and heap memory)
    Payload CreatePayload(const void* data, int size, MessageType type);

    // Starts a new batch or appends to an existing one - batches until SendLocalBatch() is called
    // Note: there is only be 1 batch at a time!
    // Each message can be sent to a different connection (as specified by the connection)
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool BatchMessage(Connection conn, Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Sends the current batch if it exists
    // Failure: returns false if BatchMessage() was not called at least once before with valid parameters
    bool SendBatch();

    // Directly sends the message - should only be used for single messages else use BatchMessage() and SendBatch()
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool SendMessage(Connection conn, Payload message, SendFlag flag = SendFlag::RELIABLE);

    // Returns a reference to a message vector containing up to "maxMessages" incoming messages
    // Can be called multiple times until the size is 0 -> no more incoming messages
    // IMPORTANT: Each call cleans up the previously returned messages
    const std::vector<Message>& ReceiveMessages(int maxMessages = 100);

    //----------------- UTIL -----------------//

    // Sets the callback function that is called on various multiplayer events
    // See the MultiplayerEvent enum for more info about the type of events and when they are triggered
    void SetMultiplayerCallback(const std::function<void(MultiplayerEvent event)>& func);

    // Returns true if currently hosting or connected to a host
    bool IsInSession();

    // Returns true if currently in a session as host
    bool IsHost();

    // Returns true if currently in a session as client
    bool IsClient();

} // namespace magique
#endif //MAGIQUE_MULTIPLAYER_H