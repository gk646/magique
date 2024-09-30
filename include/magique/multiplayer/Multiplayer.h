#ifndef MAGIQUE_MULTIPLAYER_H
#define MAGIQUE_MULTIPLAYER_H

#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// Multiplayer Module
//-----------------------------------------------
// .....................................................................
// This module allows to send messages between applications and is works with both local and global sockets
// Apart from creating your own networking model magique proposes a unique way:
//      - Peer-To-Peer where 1 peer is an authoritative host
//          PROS:
//              - Works without external servers!
//              - Allows to develop host and client code in the same project
//              - Easier network architecture (no server)
//              - The whole gamestate is only simulated on the hosts computer (single point of truth)
//              - The clients are basically only remote controlling their character in the hosts simulation
//         CONS:
//              - Doesn't work well with large amounts of people (host has to simulate game and incoming messages)
//              - The host has no latency (as the simulates the gamestate)
//              - It's easier for the host to cheat compared to the server model
//
//      How to use it:
//         -> Use EnterClientMode() on all clients - this skips all game simulation apart from the rendering
//         -> You then need to manually:
//              - Send position updates, used abilities... to the clients and update their gamestate
//              - Send the inputs from the client to the host (so the host can simulate the clients character)
//
// Note: Packets should not be bigger than 1200 bytes to avoid fragmentation.
//       But for optimal performance try to merge smaller packets into a single bigger one that is close to this limit.
// .....................................................................

namespace magique
{

    // Puts this game into client mode - all game simulation is skipped except rendering
    // All things that are skipped:
    //      - Dynamic and Static collisions
    //      - ALL scripting event methods (
    void EnterClientMode();
    void ExitClientMode();

    // Returns true if this game is currently in client mode
    bool IsInClientMode();

    //----------------- MESSAGES -----------------//

    // Returns a network message object to be sent via the network - should be used directly and not stored
    // The type is very useful for correctly handling the message on the receivers end (e.g. HEALTH_UPDATE, POS_UPDATE, ...)
    // Note: The passed data will be copied when batching or sending so you should pass it directly (stack and heap memory)
    Payload CreatePayload(const void* data, int size, MessageType type);

    // Starts a new batch or appends to an existing one - batches until SendLocalBatch() is called
    // Each message can be sent to a different connection (as specified by the connection)
    // Note: There is only be single batch at a time!
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool BatchMessage(Connection conn, Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Sends the current batch if it exists
    // Failure: returns false if BatchMessage() was not called at least once before with valid parameters
    bool SendBatch();

    // Directly sends the message - should only be used for single messages else use BatchMessage() and SendBatch()
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool SendMessage(Connection conn, Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Returns a reference to a message vector containing up to "maxMessages" incoming messages
    // Can be called multiple times until the size is 0 -> no more incoming messages
    // IMPORTANT: Each call cleans up the previously returned messages
    const std::vector<Message>& ReceiveIncomingMessages(int maxMessages = 100);

    //----------------- UTIL -----------------//

    // Sets the callback function that is called on various multiplayer events
    // See the MultiplayerEvent enum for more info about the type of events and when they are triggered
    void SetMultiplayerCallback(const std::function<void(MultiplayerEvent event)>& func);

    // Returns true if currently hosting or connected to a host
    bool IsInMultiplayerSession();

    // Returns true if currently in a session as host
    bool IsHost();

    // Returns true if currently in a session as client
    bool IsClient();

} // namespace magique
#endif //MAGIQUE_MULTIPLAYER_H