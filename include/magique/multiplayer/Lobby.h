// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_LOBBY_H
#define MAGIQUE_LOBBY_H

#include <string>
#include <magique/core/Types.h>
#include <magique/internal/PlatformIncludes.h>

//===============================================
// Lobby Module
//===============================================
// .....................................................................
// Note: This module needs either LAN or steam to be enabled (via CMake: MAGIQUE_STEAM or MAGIQUE_LAN)
//
// This module works directly on top Local-/Globalsockets - no lobbies are actually created (as separate entites)
// So everytime you open a listen socket (host) or connect to one (client) the lobby is open
// It directly reflects the state of the sockets - this is why there is no lobby callback
// Note: Use the multiplayer callbacks to react to connection events
//
// In order to provide a similar interface with steam lobbies magique Lobbies work only AFTER the connection is established
// Because you can be in a steam lobby without a global socket connection (as it runs on steam servers) - this complicates things
// Note: Look at steam/Matchmaking.h on how to create steam lobbies - use them to connect to peers - then magique Lobbies take over

// The role of a lobby is mainly grouping up players before the game starts and to exchanging data (and chat):
//      - Pick the gamemode/classes for all players
//      - Pick the map to spawn into
//      - ...
// So it comes before sending game simulation updates to all clients - however the lobby is open the whole time
// Note: You can use the start signal to know when to send game updates: GAME_STATE vs LOBBY_STATE
//
// Suggested Workflow :
//      1. Finding together:
//               - Steam: Open a steam lobby or join one and e.g. connect to the host - you can also filter lobbies and more
//               - LAN:   Connect via IP-address, either manual exchange or use LocalSockets.h helpers
//      2. Once a connection with either socket type is established this API takes over
//      3. Use lobby features like chat messages or metadata or configure the gamestate
//      5. Set the start signal to true - this starts the game for all clients
//      6. Continuously send game updates
//          - New joining clients read the metadata and immediately join the game
//
// Note: The lobby only works if you call ReceiveIncomingMessages() (each tick) - lobby packets are filtered and handled automatically
// .....................................................................

namespace magique
{
    using LobbyChatCallback = std::function<void(Connection sender, const char* chatMessage)>;

    // Sets the callback that is called on each new chat message sent in the lobby
    // Note: This will also be called for your OWN message - so you don't have to make a special case for those
    void SetLobbyChatCallback(const LobbyChatCallback& callback);

    using LobbyMetadataCallback = std::function<void(Connection sender, const char* key, const char* val)>;

    // Sets the callback for any metadata actions
    void SetLobbyMetadataCallback(const LobbyMetadataCallback& callback);

    // Returns the global lobby object
    Lobby& GetLobby();

    struct Lobby final
    {
        // Sets the value of the start signal for all members
        // Note: Only works as the lobby owner
        // Note: Automatically set to false if you leave a lobby
        void setStartSignal(bool value);

        // Gets the value of the start signal
        [[nodiscard]] bool getStartSignal() const;

        // Sends a new chat message in the lobby chat
        // Note: requires you to be in a lobby - callback not called for sent messages
        void sendChatMessage(const char* message);

        // Sets the metadata - behavior differs from clients vs hosts
        // Host:   Immediately sets the value and sends the update to all clients
        // Client: Sent the change only to the host - host then decides what to do with it
        // Note: metadat is automatically cleared if the multiplayer session closes
        void setMetadata(const char* key, const char* value);

        // Gets the value for the given key
        const std::string& getMetadata(const char* key);
    };

} // namespace magique

#endif //MAGIQUE_LOBBY_H