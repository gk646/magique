// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STEAM
#include "magique/steam/Matchmaking.h"
#include "magique/core/Types.h"
#include "magique/util/Logging.h"
namespace magique
{
    bool SteamCreateLobby(const SteamLobbyType type, const int maxPlayers) { M_ENABLE_STEAM_ERROR(false) }
    void SteamJoinLobby(SteamLobbyID lobbyID) { M_ENABLE_STEAM_ERROR() }
    bool SteamLeaveLobby() { M_ENABLE_STEAM_ERROR(false) }
    bool SteamIsInLobby() { M_ENABLE_STEAM_ERROR(false); }
    bool SteamIsLobbyOwner() { M_ENABLE_STEAM_ERROR(false); }
    SteamLobbyID SteamGetLobbyID() { M_ENABLE_STEAM_ERROR({}); }
    bool SteamOpenInviteDialog() { M_ENABLE_STEAM_ERROR(false); }
    bool SteamSendLobbyInvite(SteamID userID) { M_ENABLE_STEAM_ERROR(false); }
    void SteamSetLobbyCallback(const SteamLobbyCallback& callback) {}
} // namespace magique
#else
#include <magique/steam/Matchmaking.h>


#include "internal/globals/SteamData.h"

namespace magique
{
    bool SteamCreateLobby(const SteamLobbyType type, const int maxPlayers)
    {
        SteamAPICall_t hSteamAPICall = k_uAPICallInvalid;
        hSteamAPICall = SteamMatchmaking()->CreateLobby(static_cast<ELobbyType>((int)type), maxPlayers);
        auto& steamData = global::STEAM_DATA;
        steamData.m_SteamCallResultCreateLobby.Set(hSteamAPICall, &steamData, &SteamData::OnLobbyCreated);
        return hSteamAPICall != k_uAPICallInvalid;
    }

    void SteamJoinLobby(SteamLobbyID lobbyID)
    {
        if (SteamIsInLobby())
        {
            LOG_WARNING("Cant join a lobby when in a lobby");
            return;
        }
        SteamMatchmaking()->JoinLobby(CSteamID(static_cast<uint64>(lobbyID)));
    }

    bool SteamLeaveLobby()
    {
        if (!SteamIsInLobby())
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

    bool SteamIsInLobby() { return global::STEAM_DATA.lobbyID.IsValid(); }

    bool SteamIsLobbyOwner()
    {
        const auto& steamData = global::STEAM_DATA;
        if (SteamIsInLobby())
        {
            const auto owner = SteamMatchmaking()->GetLobbyOwner(steamData.lobbyID);
            return owner == steamData.userID;
        }
        return false;
    }

    SteamLobbyID SteamGetLobbyID() { return static_cast<SteamLobbyID>(global::STEAM_DATA.lobbyID.ConvertToUint64()); }

    SteamID SteamGetLobbyOwner()
    {
        MAGIQUE_ASSERT(SteamIsInLobby(), "Cant get the lobby owner when not in a lobby");
        return static_cast<SteamID>(SteamMatchmaking()->GetLobbyOwner(global::STEAM_DATA.lobbyID).ConvertToUint64());
    }

    void SteamSetLobbyCallback(const SteamLobbyCallback& callback)
    {
        auto& steamData = global::STEAM_DATA;
        steamData.lobbyEventCallback = callback;
    }

    Connection SteamGetConnMapping(SteamID id)
    {
        for (auto& mapping : global::MP_DATA.steamMapping)
        {
            if (mapping.steam == id)
            {
                return mapping.conn;
            }
        }
        return Connection::INVALID;
    }

    SteamID SteamGetConnMapping(Connection conn)
    {
        for (auto& mapping : global::MP_DATA.steamMapping)
        {
            if (mapping.conn == conn)
            {
                return mapping.steam;
            }
        }
        return SteamID::INVALID;
    }

    bool SteamOpenInviteDialog()
    {
        auto& steam = global::STEAM_DATA;
        if (SteamIsInLobby())
        {
            SteamFriends()->ActivateGameOverlayInviteDialog(steam.lobbyID);
            return true;
        }
        return false;
    }

    bool SteamSendLobbyInvite(SteamID userID)
    {
        MAGIQUE_ASSERT(SteamIsInLobby(), "Cant invite others to lobby if not in a lobby");
        return SteamMatchmaking()->InviteUserToLobby(global::STEAM_DATA.lobbyID, SteamIDFromMagique(userID));
    }

} // namespace magique

#endif
