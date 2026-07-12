// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_LOBBY_H
#define MAGIQUE_LOBBY_H

#include <string>
#include <functional>

//===============================================
// Lobby Module
//===============================================
// .....................................................................
// Note: This module needs either LAN or steam to be enabled (via CMake: MAGIQUE_STEAM or MAGIQUE_LAN)
//
// This module works directly on top Local-/Globalsockets - no lobbies are actually created (as separate entities)
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
//
// Suggested Workflow :
//      1. Finding together:
//               - Steam: Open a steam lobby or join one and e.g. connect to the host - you can also filter lobbies and more
//               - LAN:   Connect via IP-address, either manual exchange or use LocalSockets.h helpers
//      2. Once a connection with either socket type is established this API takes over
//      3. Use lobby features like chat messages or metadata or configure the gamestate
//      5. Use metadata to tell clients to start the game
//      6. Continuously send game updates
//          - New joining clients read the metadata and immediately join the game
//
// Note: The lobby only works if you call NetworkReceive() (each tick) - lobby packets are filtered and handled automatically
// .....................................................................

namespace magique
{
    using LobbyChatCallback = std::function<void(std::string_view sender, std::string_view msg)>;
    using LobbyMetadataCallback = std::function<void(std::string_view key, std::string_view val)>;

    // Sets the callback that is called on each new chat message sent in the lobby
    // Note: This will also be called for your OWN message - so you don't have to make a special case for those
    void LobbySetChatCallback(const LobbyChatCallback& callback);

    // Sets the callback for any metadata changes
    void LobbySetMetadataCallback(const LobbyMetadataCallback& callback);

    // Sends a chat message - message replicates to the steam lobby
    void LobbySendChatMsg(std::string_view sender, std::string_view message);

    // Sets the metadata - only works on the host - replicates to the steam lobby
    // Note: metadata is automatically cleared if the multiplayer session closes
    std::string_view LobbyGetMetadata(std::string_view key);
    void LobbySetMetadata(std::string_view key, std::string_view value);

} // namespace magique

#endif // MAGIQUE_LOBBY_H
