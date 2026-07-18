// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STEAM_MATCHMAKING_H
#define MAGIQUE_STEAM_MATCHMAKING_H

#include <magique/core/Types.h>
#include <functional>

//===============================================
// Steam Matchmaking Module
//===============================================
// .....................................................................
// Note: This module needs steam to be enabled (via CMake: MAGIQUE_STEAM)
// This module allows to create/search for steam lobbies which is vital for creating multiplayer sessions over steam.
// It should mainly be used to facility connection via global sockets
// After successful connection its intended to use the common API from multiplayer/Lobby.h - check for more info
//
// Note: You can only be in a single lobby at any time - a lobby automatically closes when its empty
// .....................................................................

namespace magique
{

    // Combines all callbacks into a single handler
    struct ISteamMatchmakingCallbacks
    {
        virtual ~ISteamMatchmakingCallbacks() = default;

        // Called for GameLobbyJoinRequested_t
        // This happens when user clicks a received game invite while in-game
        // Note: You should probably leave the current lobby and join this new one
        virtual void onLobbyJoinRequest(SteamLobbyID lobby, SteamID invitee) {}

        // Called for various lobby events: see SteamLobbyEvent
        virtual void onLobbyEvent(SteamLobbyID lobby, SteamID user, SteamLobbyEvent lobbyEvent) {}

        // Called when data is returned after a call to SteamSearchLobbies()
        virtual void onLobbySearchResult(const std::vector<SteamLobbyID>& lobbies) {}

        //
        virtual void onLobbyChatMessage(std::string_view chatMessage, SteamID sender) {}

        // Called when the meta data fields of the lobby change
        virtual void onLobbyDataUpdate() {}
    };

    // Returns true if the async call trying to create a lobby was successful
    // Note: This causes both LOBBY_CREATED and LOBBY_ENTERED events
    bool SteamLobbyCreate(SteamLobbyType type, int maxPlayers = 4);

    // Tries to connect to the lobby specified by the given id (or id as string)
    void SteamLobbyJoin(SteamLobbyID lobbyID);
    void SteamLobbyJoin(std::string_view lobbyID);

    // Returns the LobbyID of your current lobby
    SteamLobbyID SteamLobbyGetID();

    // Sends a lobby msg up to 4k characters
    // Note: Using Lobby.h/sendChatMsg() also calls this internally
    bool SteamLobbySendMsg(std::string_view msg);

    // Sets the value for the specified key - only works if your owner
    void SteamLobbySetData(std::string_view key, std::string_view value);

    // Returns the value for the key of the specified lobby
    // Failure: Returns "" (empty string) if the lobby is invalid or no such key
    std::string_view SteamLobbyGetData(std::string_view key, SteamLobbyID id = SteamLobbyGetID());

    // Returns the current player count and limit of the given lobby
    std::pair<int, int> SteamLobbyGetPlayerCount(SteamLobbyID id);

    // Leaves the current lobby specified by the given id
    // Failure: returns false if not currently in a lobby
    bool SteamLeaveLobby();

    // Opens the steam invite dialogue (overlay) to invite friends to your lobby
    // Failure: returns false if not currently in a lobby
    bool SteamOpenInviteDialog();

    // Returns true if the lobby invite was successfully sent to the given user
    bool SteamSendLobbyInvite(SteamID userID);

    // Returns true if your currently in a lobby
    bool SteamIsInLobby();

    // Returns true if your currently in a lobby AND the owner of the lobby
    bool SteamIsLobbyOwner();

    // Returns the SteamID of the owner of the given lobby
    SteamID SteamGetLobbyOwner(SteamLobbyID lobby = SteamLobbyGetID());

    // Allows to filter for certain properties
    struct SteamSearchFilter
    {
        // Filters lobbies where the lobby value at the given key doesn't match with the given value and comparator
        SteamSearchFilter& string(std::string_view key, std::string_view value,
                                  SteamComparisonFilter comp = SteamComparisonFilter::Equal);

        // Filters lobbies where the lobby value at the given key doesn't match with the given value and comparator
        SteamSearchFilter& number(std::string_view key, int value,
                                  SteamComparisonFilter comp = SteamComparisonFilter::Equal);
    };

    // Start a search with the currently applied filters - returns up to 50 lobbies
    // Note: Filters are cleared after each search call - need to be reapplied
    // Per default sorts after geographical distance, doesn't show full lobbies and uses the Default distance filter
    void SteamSearchLobbies();


} // namespace magique


#endif // MAGIQUE_STEAM_MATCHMAKING_H
