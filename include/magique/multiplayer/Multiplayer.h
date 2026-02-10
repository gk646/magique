// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MULTIPLAYER_H
#define MAGIQUE_MULTIPLAYER_H

#include <functional>
#include <magique/core/Core.h>
#include <magique/util/Logging.h>
#include <magique/internal/InternalTypes.h>
#include <enchantum/enchantum.hpp>

//===============================================
// Multiplayer Module
//===============================================
// .....................................................................
// Note: You might want to look at Lobby.h if you want more high level features like chat or metadata

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
//       The batching system ALREADY does this for you under the hood!
// IMPORTANT: MessageType=UINT8_MAX and UINT8_MAX-1 are reserved for magique
//
// .....................................................................

namespace magique
{
    //================= BATCHING =================//

    // The batching system is very efficient and gameplay tested
    // When using batching all messages are condensed into a single packet (up to when its full)
    // This batch of messages is then sent as a single packet, when calling SendBatch() (hence the reserved type)
    // This is way more efficient than sending many small message independently (as there is overhead for each send)
    // On the receiver magique handles the unpacking and allows you to iterate all the message independently again
    // Note: magique keeps separate buffers for reliable and unreliable message per peer (as a packet can only have 1 flag)
    //       The reliable buffer has higher priority and will be sent first
    //       It is also sent before every unreliable packet - even if not full
    //       This is done because often vital game updates are sent reliably and should arrive first (spawn, despawn)
    //       This means ALL messages arrive in the order they were batch in (within the flag group)

    // Batches (stores) the payload internally - batch is sent (and cleared) when SendBatch() is called
    // Each message can be sent to a different connection (as specified by the connection)
    void BatchMessage(Connection conn, Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Only works if you are a client - same as BatchMessage with the first available connection
    void BatchToHost(Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Batches the given payload for each current connection
    // This is equal to calling BatchMessage() for each connection from GetCurrentConnections()
    void BatchMessageToAll(Payload payload, SendFlag flag = SendFlag::RELIABLE);
    // Allows to specify a connection that is excluded
    void BatchMessageToAll(Payload payload, SendFlag flag, Connection exclude);

    // Flushed the batching buffers for all connections
    void SendBatch();

    //================= MESSAGES =================//

    // Directly sends the message - should only be used for single messages else use BatchMessage() and SendBatch()
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool SendMessage(Connection conn, Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Directly sends the given message to all current connections
    // This is equal to calling SendMessage() for each connection from GetCurrentConnections()
    void SendMessageToAll(Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Flush any messages waiting on the nagle timer or on the sockets
    // Note: You should call this at the end of each game tick to get all the message out
    void FlushMessages();

    // Returns a reference to a message vector containing up to "maxMessages" incoming messages
    // Can be called multiple times until the size is 0 -> no more incoming messages
    // IMPORTANT: Each call cleans up the previously returned messages (copy if you wanna store its information)
    const std::vector<Message>& ReceiveIncomingMessages(int maxMessages = MAGIQUE_ESTIMATED_MESSAGES);

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

    // True if EITHER host or client in a local socket environment
    bool InLocalSocketSession();

    // True if either host or client in a global socket environment
    bool InGlobalSocketSession();

    // Sets the entity for the given connection - allows to create a mapping between the remote client and a local entity
    // Note: Needs to be manually set BUT is automatically deleted AFTER the connection is disconnected or session is closed
    // Note: An existing mapping can be updated with a new entity
    void SetConnectionEntityMapping(Connection conn, entt::entity entity);

    // Returns the entity mapped to this connection - entt::null if none was set
    entt::entity GetConnectionEntityMapping(Connection conn);

    // Returns the (first) connection mapped to the given entity (if any)
    // Failure: Returns INVALID_CONNECTION if mapping exists for the given entity
    Connection GetConnectionEntityMapping(entt::entity entity);

    // Returns the number of the connection - between 0 and MAGIQUE_MAX_PLAYERS-1
    // This numbers stays the same for as long as the connection doesn't terminate (regardless of other disconnects or joins)
    // Note: This is very useful if you want to index into an array based on the connection
    // Failure: -1 if connection is not valid
    int GetConnectionNum(Connection conn);

    // Prints statistics on the amount of sent packages
    // Only works in debug mode
    // Note: The definition of MessageType NEEDS to be included (so enum names can be printed)
    //       Otherwise you get strange compiler errors...
    template <typename T = MessageType>
    void PrintPacketStats();

    //================= CLIENT-MODE =================//

    // Puts this game into client mode - all game simulation (should) is skipped except rendering
    // Comprehensive list of things that are SKIPPED in ClientMode:
    //      - Dynamic and Static collisions
    //      - ALL scripting event methods
    void EnterClientMode();
    void ExitClientMode();

    // Returns true if this game is currently in client mode
    bool GetIsClientMode();


} // namespace magique


// IMPLEMENTATION


namespace magique
{

    namespace internal
    {
        MultiplayerStatsData getStats();

        template <typename T = MessageType>
        void PrintDirectionStats(const std::array<MessageCount, UINT8_MAX>& stats, uint32_t bytes)
        {
            static_assert(std::is_enum_v<MessageType> && sizeof(MessageType) > 0,
                          "Include the header where enum class MessageTypes is defined!");

            uint32_t total = 0;
            const auto ticks = GetEngineTick();

            printf("\t%-25s || %10s | %10s\n", "Message // Stat", "Count", "Avg/tick");
            for (const auto& entry : stats)
            {
                if (entry.count == 0)
                    continue;
                const float avg = static_cast<float>(entry.count) / ticks;

                auto enumName = enchantum::to_string((T)entry.type);
                if (enumName.empty())
                {
                    enumName = TextFormat("%d", (int)entry.type);
                }
                printf("\t%-25s || %10d | %10.2f \n", std::string{enumName}.c_str(), entry.count, avg);
                total += entry.count;
            }
            puts("\t---------------------------------------------------");

            printf("\t%-25s || %10d | %10.2f \n", "Total", (int)total, static_cast<float>(total) / ticks);
            printf("\t%-25s || %10d | %10.2f", "Bytes", (int)bytes, static_cast<float>(bytes) / ticks);
            const auto passedSeconds = ticks / MAGIQUE_LOGIC_TICKS;
            printf(" => %.2f MB/h\n", ((static_cast<float>(bytes) / passedSeconds) * 60 * 60) / 1'000'000);
        }

    } // namespace internal

    template <typename T>
    void PrintPacketStats()
    {
        const auto data = internal::getStats();
        if (data.bytesOut != 0 || data.bytesIn != 0)
        {
            LOG_INFO("Incoming Network Stats:");
            internal::PrintDirectionStats(data.incoming, data.bytesIn);
            LOG_INFO("Outgoing Network Stats:");
            internal::PrintDirectionStats(data.outgoing, data.bytesOut);
        }
    }

} // namespace magique

#endif // MAGIQUE_MULTIPLAYER_H
