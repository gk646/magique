// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STEAM_MATCHMAKING_H
#define MAGIQUE_STEAM_MATCHMAKING_H

#include <magique/core/Types.h>
#include <functional>

//===============================================
// Steam Lobbies Module
//===============================================
// .....................................................................
// Note: This module needs steam to be enabled (via CMake: MAGIQUE_STEAM)
// This module allows to create steam lobbies which is vital for creating multiplayer sessions over steam.
// It should mainly be used to facility connection via global sockets
// After successful connection its intended to use the common API from multiplayer/Lobby.h - check for more info
//
// Note: You can only be in a single lobby at any time - a lobby automatically closes when its empty
// .....................................................................

namespace magique
{
    //================= SETUP =================//

    // Returns true if the async call trying to create a lobby was successful
    // Note: This causes both LOBBY_CREATED and LOBBY_ENTERED events
    bool SteamCreateLobby(SteamLobbyType type, int maxPlayers = 4);

    // Tries to connect to the lobby specified by the given id
    void SteamJoinLobby(SteamLobbyID lobbyID);

    // Leaves the current lobby specified by the given id
    // Failure: returns false if not currently in a lobby
    bool SteamLeaveLobby();

    // Opens the steam invite dialogue (overlay) to invite friends to your lobby
    // Failure: returns false if not currently in a lobby
    bool SteamOpenInviteDialog();

    // Returns true if the lobby invite was successfully sent to the given user
    bool SteamSendLobbyInvite(SteamID userID);

    //================= INFO =================//

    // Returns true if your currently in a lobby
    bool SteamIsInLobby();

    // Returns true if your currently in a lobby AND the owner of the lobby
    bool SteamIsLobbyOwner();

    // Returns the LobbyID of your current lobby
    SteamLobbyID SteamGetLobbyID();

    // Returns the SteamID of the lobby owner
    // Note: requires you to be in a lobby
    SteamID SteamGetLobbyOwner();

    // Called with the lobby id, the protagonist of that event and the event type
    // See the LobbyEvent enum for a detailed description when each event gets called
    using SteamLobbyCallback = std::function<void(SteamLobbyID lobby, SteamID steamID, SteamLobbyEvent lobbyEvent)>;

    // Sets the callback that is called for various events that happen in regard to lobbies
    void SteamSetLobbyCallback(const SteamLobbyCallback& callback);

} // namespace magique


#endif // MAGIQUE_STEAM_MATCHMAKING_H
