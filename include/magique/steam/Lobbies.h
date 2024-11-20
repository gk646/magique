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
    void ConnectToSteamLobby(SteamID lobbyID);

    // Leaves the lobby specified by the given id
    void LeaveSteamLobby(SteamID lobbyID);

    // Opens the steam invite dialogue to invite friends to your lobby
    // Note: This requires you to be in lobby
    void OpenInviteDialogue();

    //================= CHAT =================//

    // Sends a new chat message in the lobby chat
    void SendLobbyMessage(const char* message);

    // Sets the callback that is called on each new chat message sent in the lobby
    void SetChatMessageCallback(const std::function<void(SteamID sender, const std::string& chatMessage)>& chatCallback);

    //================= INFO =================//

    // Returns true if your currently in a lobby
    bool IsInLobby();

    // Returns true if your currently in a lobby AND the owner of the lobby
    bool IsLobbyOwner();

    // Returns the SteamID of the lobby owner
    SteamID GetLobbyOwner();

    // Sets the callback that is called for various events that happen during the lobby lifetime
    // Check LobbyEvents for when each event is called, gets passed the SteamID of the affected user (e.g. who joined?)
    void SetLobbyEventCallback(const std::function<void(SteamID steamID, LobbyEvent lobbyEvent)>& lobbyEventCallback);


} // namespace magique


#endif //MAGIQUE_STEAM_LOBBIES_H