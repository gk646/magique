// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MULTIPLAYER_H
#define MAGIQUE_MULTIPLAYER_H

#include <vector>
#include <magique/core/Types.h>
#include <magique/internal/PlatformIncludes.h>

//===============================================
// Multiplayer Module
//===============================================
// .....................................................................
// This module allows to send messages between applications and is works with both local and global sockets
// Apart from creating your own networking model magique proposes a unique way:
//      - Peer-To-Peer where 1 peer is an authoritative host
//          PROS:
//              - Works without external servers!
//              - Code for both sides is in the same project (could be a con)
//              - Allows to develop host and client code in the same project
//              - Easier network architecture (no server)
//              - The whole gamestate is only simulated on the hosts computer (single point of truth)
//              - The clients are basically only remote controlling their character in the hosts simulation
//         CONS:
//              - Doesn't work well with large amounts of people (host has to simulate game and incoming messages)
//              - The host has no latency (as it simulates the gamestate)
//              - It's easier for the host to cheat compared to the server model
//
//      How to use it:
//         -> Use EnterClientMode() on all clients - this skips all game simulation apart from the rendering
//         -> You then need to manually:
//              - Send position updates, used abilities... to the clients and update their gamestate
//              - Send the inputs from the client to the host (so the host can simulate the clients character)
//
// You can however make a dedicated server program with magique as well.
// Note: Packets should not be bigger than 1200 bytes to avoid fragmentation (To still have overhead towards the MTU)
//       But for optimal performance try to merge smaller packets into a single bigger one that is close to this limit.
//
// .....................................................................

namespace magique
{

    //================= MESSAGES =================//

    // Returns a network message object to be sent via the network - should be used directly and not stored
    // The type is very useful for correctly handling the message on the receivers end (e.g. HEALTH_UPDATE, POS_UPDATE, ...)
    // Note: The passed data will be copied when batching and sending (so supports both stack and heap memory)
    Payload CreatePayload(const void* data, int size, MessageType type);

    // Batches (stores) the payload internally - batch is sent (and cleared) when SendBatch() is called
    // Each message can be sent to a different connection (as specified by the connection)
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool BatchMessage(Connection conn, Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Batches the given payload for each current connection
    // This is equal to calling BatchMessage() for each connection from GetCurrentConnections()
    void BatchMessageToAll(Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Sends the current batch if it exists
    // Failure: returns false if BatchMessage() was not called at least once before with valid parameters
    bool SendBatch();

    // Directly sends the message - should only be used for single messages else use BatchMessage() and SendBatch()
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool SendMessage(Connection conn, Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Directly sends the given message to all current connections
    // This is equal to calling SendMessage() for each connection from GetCurrentConnections()
    void SendMessageToAll(Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Returns a reference to a message vector containing up to "maxMessages" incoming messages
    // Can be called multiple times until the size is 0 -> no more incoming messages
    // IMPORTANT: Each call cleans up the previously returned messages (copy if you wanna its information)
    const std::vector<Message>& ReceiveIncomingMessages(int maxMessages = 100);

    //================= UTIL =================//

    // Returns the vector that contains all current valid connections
    const std::vector<Connection>& GetCurrentConnections();

    // Called with the current even and the affected connection
    // ClientEntity mappings will be deleted after the callback (on disconnect)
    using MultiplayerCallback = std::function<void(MultiplayerEvent event, Connection conn)>;

    // Sets the callback function that is called on various multiplayer events
    // See the MultiplayerEvent enum for more info about the type of events and when they are triggered
    void SetMultiplayerCallback(const MultiplayerCallback& func);

    // Returns true if currently hosting or connected to a host
    bool GetInMultiplayerSession();

    // Returns true if currently in a session as the host
    bool GetIsHost();

    // Returns true if currently in a session as the host AND at least 1 client
    bool GetIsActiveHost();

    // Returns true if currently in a session as a client
    bool GetIsClient();

    // Sets the entity for the given connection - allows to create a mapping between the remote client and a local entity
    // Note: Needs to be manually set BUT is automatically deleted AFTER the connection is disconnected or session is closed
    // Note: An existing mapping can be updated with a new entity
    void SetConnectionEntityMapping(Connection conn, entt::entity entity);

    // Returns the entity mapped to this connection - entt::null if none was set
    entt::entity GetConnectionEntityMapping(Connection conn);

    //================= CLIENT-MODE =================//

    // Puts this game into client mode - all game simulation is skipped except rendering
    // Comprehensive list of things that are SKIPPED in ClientMode:
    //      - Dynamic and Static collisions
    //      - ALL scripting event methods
    void EnterClientMode();
    void ExitClientMode();

    // Returns true if this game is currently in client mode
    bool GetIsClientMode();

    //================= UTIL =================//

    // Returns true if magique was built with steam
    bool GetSteamLoaded();

    // Returns true if magique was built with GameNetworkingSockets (local sockets)
    bool GetNetworkingSocketsLoaded();

} // namespace magique


#endif //MAGIQUE_MULTIPLAYER_H