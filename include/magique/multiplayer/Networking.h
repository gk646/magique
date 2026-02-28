// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_NETWORKING_H
#define MAGIQUE_NETWORKING_H

#include <functional>
#include <magique/core/Engine.h>
#include <magique/util/Logging.h>
#include <magique/internal/InternalTypes.h>
#include <enchantum/enchantum.hpp>

//===============================================
// Networking Module
//===============================================
// .....................................................................
// Note: You might want to look at Lobby.h if you want more high level features like chat or metadata
//
// This module allows to send messages between applications and is works with both local and global sockets
// Apart from creating your own networking model magique proposes a unique way:
//      - Peer-To-Peer where 1 peer is an authoritative host
//          PROS:
//              - Works without external servers!
//              - Code for both sides is in the same project (could be a con)
//              - The whole gamestate is only simulated on the hosts computer (single point of truth)
//              - The clients are basically only remote controlling their character in the hosts simulation
//         CONS:
//              - Doesn't work well with large amounts of people (host has to simulate game and handle incoming messages)
//              - The host has no latency and thus an advantage (as it simulates the gamestate)
//              - It's easier for the host to cheat compared to the server model
//
//      How to use it:
//         -> Use EnterClientMode() on all clients - this skips all game simulation apart from the rendering
//         -> You then need to manually:
//              - Send position updates, used abilities... to the clients and update their gamestate
//              - Send the inputs from the client to the host (so the host can simulate the clients character)
//
// The way to think about it is to design a server client structure but the host running at the server
// For example the send input method for the host directly assigns it locally, the client actually sends it over the wire.
// But they call the same method. Like this a later split to a full server-client is more easily possible
//
// You can however make a dedicated server program with magique as well.
// Note: Packets should not be bigger than 1200 bytes to avoid fragmentation (To still have overhead towards the MTU)
// IMPORTANT: MessageType=UINT8_MAX and UINT8_MAX-1 are reserved for magique
// .....................................................................

namespace magique
{
    // Closes an open global OR local socket (if any)
    // Optionally specify a close code or string that can be read on the receiver
    // Returns true if an existing connection was closed successfully
    bool NetworkCloseSocket(int code = 0, std::string_view reason = {});

    // Disconnects the given connection - if no connections remain closes the network sessions
    // Optionally specify a close code or string that can be read on the receiver
    // Returns true if the connection was closed successfully
    bool NetworkCloseConnection(Connection conn, int code = 0, std::string_view reason = {});

    //================= SENDING =================//

    // Directly sends the message - should only be used for single messages else use BatchMessage() and SendBatch()
    // Failure: returns false if passed data is invalid, invalid connection or invalid send flag
    bool NetworkSend(Connection conn, Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Return true if successfully sent a message to all current connections
    // This is equal to calling SendMessage() for each connection from GetCurrentConnections()
    bool NetworkSendAll(Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Sends to the first available connection (the host if you are a client)
    bool NetworkSendHost(Payload payload, SendFlag flag = SendFlag::RELIABLE);

    // Flush any messages waiting on the nagle timer or on the sockets
    // Note: You should call this at the end of each game tick to get all the message out
    void NetworkFlush();

    // Returns a reference to a message vector containing up to "maxMessages" incoming messages
    // Can be called multiple times until the size is 0 -> no more incoming messages
    // IMPORTANT: Each call cleans up the previously returned messages (copy if you wanna store its information)
    const std::vector<Message>& NetworkReceive(int maxMessages = 150);

    //================= UTIL =================//

    // Returns the vector that contains all current valid connections
    const std::vector<Connection>& NetworkGetConnections();

    struct NetworkEventData
    {
        SteamID steam;                // Steamid (only valid if using global sockets) else SteamID::INVALID
        int closeCode;                // Specified close code
        std::string_view closeReason; // Specified close string
    };

    // Called with the current even and the affected connection
    // ClientEntity mappings will be deleted after the callback (on disconnect)
    // SteamID will be SteamID::INVALID when using LocalSocket
    using NetworkCallback = std::function<void(NetworkEvent event, Connection conn, const NetworkEventData& data)>;

    // Sets the callback function that is called on various multiplayer events
    // See the MultiplayerEvent enum for more info about the type of events and when they are triggered
    void NetworkSetCallback(const NetworkCallback& func);

    // Returns true if currently hosting or connected to a host
    bool NetworkInSession();

    // Returns true if currently in a session as the host
    bool NetworkIsHost();

    // Returns true if currently in a session as a client
    bool NetworkIsClient();

    // Returns true if NOT in a session OR NetworkIsHost()
    // Note: This is useful with the p2p model, as UI and logic should work if either not in session (single player) or the host
    bool NetworkIsLocalPlayer();

    // Sets the entity for the given connection - allows to create a mapping between the remote client and a local entity
    // Note: Needs to be manually set BUT is automatically deleted AFTER the connection is disconnected or session is closed
    // Note: An existing mapping can be updated with a new entity
    void NetworkSetConnMapping(Connection conn, entt::entity entity);

    // Returns the entity mapped to this connection - entt::null if none was set
    entt::entity NetworkGetConnMapping(Connection conn);

    // Returns the (first) connection mapped to the given entity (if any)
    // Failure: Returns INVALID_CONNECTION if mapping exists for the given entity
    Connection NetworkGetConnMapping(entt::entity entity);

    // Returns the number of the connection
    // This numbers stays the same for as long as the connection doesn't terminate (regardless of other disconnects or joins)
    // Note: This is very useful if you want to index into an array based on the connection
    // Failure: -1 if connection is not valid
    int NetworkGetConnNumber(Connection conn);

    // Prints statistics on the amount of sent packages
    // Only works in debug mode
    // Note: The definition of MessageType NEEDS to be included (so enum names can be printed)
    //       Otherwise you get strange compiler errors...
    template <typename T = MessageType>
    void NetworkPrintStats();

    //================= CLIENT-MODE =================//

    // Puts this game into client mode - all game simulation (should) is skipped except rendering
    // Comprehensive list of things that are SKIPPED in ClientMode:
    //      - Dynamic and Static collisions
    //      - ALL scripting event methods
    void NetworkEnterClientMode();
    void NetworkExitClientMode();

    // Returns true if this game is currently in client mode
    bool NetworkIsClientMode();


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
            const auto ticks = EngineGetTick();

            printf("\t%-25s || %10s | %10s\n", "Message // Stat", "Count", "Avg/tick");
            for (const auto& entry : stats)
            {
                if (entry.count == 0)
                    continue;
                const float avg = static_cast<float>(entry.count) / ticks;

                auto enumName = enchantum::to_string(static_cast<T>(entry.type));
                if (enumName.empty())
                {
                    enumName = TextFormat("%d", (int)entry.type);
                }
                printf("\t%-25s || %10d | %10.2f \n", enumName.data(), entry.count, avg);
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
    void NetworkPrintStats()
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

#endif // MAGIQUE_NETWORKING_H
