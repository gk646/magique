// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STEAM_LOBBIES_H
#define MAGIQUE_STEAM_LOBBIES_H

#include <string>
#include <magique/core/Types.h>
#include <magique/internal/PlatformIncludes.h>

//===============================================
// Steam Lobbies Module
//===============================================
// .....................................................................
// Note: This module needs steam to be enabled (via CMake: MAGIQUE_STEAM)
// This module allows to create steam lobbies which is vital for creating multiplayer sessions over steam.
// Note: You can only be in a single lobby at any time - a lobby automatically closes when its empty
// .....................................................................

namespace magique
{

    //================= SETUP =================//

    // Returns true if the async call trying to create a lobby was successful
    // Note: This causes both LOBBY_CREATED and LOBBY_ENTERED events
    bool CreateSteamLobby(LobbyType type, int maxPlayers);

    // Tries to connect to the lobby specified by the given id
    void JoinSteamLobby(LobbyID lobbyID);

    // Leaves the current lobby specified by the given id
    // Failure: returns false if not currently in a lobby
    bool LeaveSteamLobby();

    // Opens the steam invite dialogue to invite friends to your lobby
    // Failure: returns false if not currently in a lobby
    bool OpenLobbyInviteDialogue();

    // Returns true if the lobby invite was successfully sent to the given user
    bool InviteUserToLobby(SteamID userID);

    //================= CHAT =================//

    // Sends a new chat message in the lobby chat
    // Note: requires you to be in a lobby
    void SendLobbyChatMessage(const char* message);

    // Called with the steamID of the sender and the sent string
    using ChatCallback = std::function<void(SteamID sender, const std::string& chatMessage)>;

    // Sets the callback that is called on each new chat message sent in the lobby
    // Note: This will also be called for your OWN message - so you don't have to make a special case for those
    void SetChatMessageCallback(const ChatCallback& chatCallback);

    //================= INFO =================//

    // Returns true if your currently in a lobby
    bool GetIsInLobby();

    // Returns true if your currently in a lobby AND the owner of the lobby
    bool GetIsLobbyOwner();

    // Returns the LobbyID of your current lobby
    LobbyID GetLobbyID();

    // Returns the SteamID of the lobby owner
    // Note: requires you to be in a lobby
    SteamID GetLobbyOwner();

    // Called with the lobby id, the protagonist of that event and the event type
    // See the LobbyEvent enum for a detailed description when each event gets called
    using LobbyCallback = std::function<void(LobbyID lobby, SteamID steamID, LobbyEvent lobbyEvent)>;

    // Sets the callback that is called for various events that happen in regard to lobbies
    void SetLobbyEventCallback(const LobbyCallback& lobbyEventCallback);


} // namespace magique


#endif //MAGIQUE_STEAM_LOBBIES_H