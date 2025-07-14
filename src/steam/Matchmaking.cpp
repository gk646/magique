// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <magique/steam/Matchmaking.h>
#include <magique/internal/Macros.h>

#include "internal/globals/SteamData.h"

namespace magique
{
    bool CreateSteamLobby(const SteamLobbyType type, const int maxPlayers)
    {
        SteamAPICall_t hSteamAPICall = k_uAPICallInvalid;
        hSteamAPICall = SteamMatchmaking()->CreateLobby(static_cast<ELobbyType>((int)type), maxPlayers);
        auto& steamData = global::STEAM_DATA;
        steamData.m_SteamCallResultCreateLobby.Set(hSteamAPICall, &steamData, &SteamData::OnLobbyCreated);
        return hSteamAPICall != k_uAPICallInvalid;
    }

    void JoinSteamLobby(SteamLobbyID lobbyID)
    {
        if (GetInSteamLobby())
        {
            LOG_WARNING("Cant join a lobby when in a lobby");
            return;
        }
        SteamMatchmaking()->JoinLobby(CSteamID(static_cast<uint64>(lobbyID)));
    }

    bool LeaveSteamLobby()
    {
        if (!GetInSteamLobby())
            return false;
        const auto& steam = global::STEAM_DATA;
        SteamMatchmaking()->LeaveLobby(steam.lobbyID);
        if (steam.lobbyEventCallback)
        {
            const auto lobbyID = LobbyIDFromSteam(steam.lobbyID);
            const auto steamID = MagiqueIDFromSteam(steam.userID);
            steam.lobbyEventCallback(lobbyID, steamID, SteamLobbyEvent::ON_LOBBY_EXIT);
        }
        return true;
    }

    bool GetInSteamLobby() { return global::STEAM_DATA.lobbyID.IsValid(); }

    bool GetIsSteamLobbyOwner()
    {
        const auto& steamData = global::STEAM_DATA;
        if (GetInSteamLobby())
        {
            const auto owner = SteamMatchmaking()->GetLobbyOwner(steamData.lobbyID);
            return owner == steamData.userID;
        }
        return false;
    }

    SteamLobbyID GetSteamLobbyID() { return static_cast<SteamLobbyID>(global::STEAM_DATA.lobbyID.ConvertToUint64()); }

    SteamID GetSteamLobbyOwner()
    {
        MAGIQUE_ASSERT(GetInSteamLobby(), "Cant get the lobby owner when not in a lobby");
        return static_cast<SteamID>(SteamMatchmaking()->GetLobbyOwner(global::STEAM_DATA.lobbyID).ConvertToUint64());
    }

    void SetSteamLobbyCallback(const SteamLobbyCallback& callback)
    {
        auto& steamData = global::STEAM_DATA;
        steamData.lobbyEventCallback = callback;
    }

    bool OpenSteamLobbyInviteDialogue()
    {
        auto& steam = global::STEAM_DATA;
        if (GetInSteamLobby())
        {
            SteamFriends()->ActivateGameOverlayInviteDialog(steam.lobbyID);
            return true;
        }
        return false;
    }

    bool InviteToSteamLobby(SteamID userID)
    {
        MAGIQUE_ASSERT(GetInSteamLobby(), "Cant invite others to lobby if not in a lobby");
        return SteamMatchmaking()->InviteUserToLobby(global::STEAM_DATA.lobbyID, SteamIDFromMagique(userID));
    }

} // namespace magique