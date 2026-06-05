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
    bool SteamLobbyCreate(const SteamLobbyType type, const int maxPlayers)
    {
        SteamAPICall_t hSteamAPICall = k_uAPICallInvalid;
        hSteamAPICall = SteamMatchmaking()->CreateLobby(static_cast<ELobbyType>((int)type), maxPlayers);
        auto& steamData = global::STEAM_DATA;
        steamData.m_SteamCallResultCreateLobby.Set(hSteamAPICall, &steamData, &SteamData::OnLobbyCreated);
        return hSteamAPICall != k_uAPICallInvalid;
    }

    void SteamLobbyJoin(SteamLobbyID lobbyID)
    {
        if (SteamIsInLobby())
        {
            LOG_WARNING("Cant join a lobby when in a lobby");
            return;
        }
        if (lobbyID == SteamLobbyID::INVALID)
        {
            LOG_WARNING("Invalid lobby ID: %llu", static_cast<uint64>(lobbyID));
            return;
        }
        SteamMatchmaking()->JoinLobby(CSteamID(static_cast<uint64>(lobbyID)));
    }

    void SteamLobbyJoin(std::string_view lobbyID)
    {
        uint64_t value = std::strtoull(lobbyID.data(), nullptr, 10);
        CSteamID id{(uint64)value};
        if (!id.IsValid())
        {
            LOG_WARNING("Invalid lobby ID: %s", lobbyID.data());
            return;
        }
        SteamLobbyJoin(static_cast<SteamLobbyID>(id.ConvertToUint64()));
    }

    void SteamLobbySetData(std::string_view key, std::string_view value)
    {
        SteamMatchmaking()->SetLobbyData(CSteamID(static_cast<uint64>(SteamLobbyGetID())), key.data(), value.data());
    }

    bool SteamLobbySendMsg(std::string_view msg)
    {
        const auto& steam = global::STEAM_DATA;
        return SteamMatchmaking()->SendLobbyChatMsg(steam.lobbyID, msg.data(), msg.size() + 1);
    }

    std::string_view SteamGetLobbyData(std::string_view key, SteamLobbyID id)
    {
        return SteamMatchmaking()->GetLobbyData(CSteamID(static_cast<uint64>(id)), key.data());
    }

    std::pair<int, int> SteamLobbyGetPlayerCount(SteamLobbyID id)
    {
        const auto steamID = CSteamID(static_cast<uint64>(id));
        return {SteamMatchmaking()->GetNumLobbyMembers(steamID), SteamMatchmaking()->GetLobbyMemberLimit(steamID)};
    }

    bool SteamLeaveLobby()
    {
        if (!SteamIsInLobby())
            return false;
        auto& steam = global::STEAM_DATA;
        SteamMatchmaking()->LeaveLobby(steam.lobbyID);
        if (steam.lobbyEventCallback)
        {
            const auto lobbyID = LobbyIDFromSteam(steam.lobbyID);
            const auto steamID = MagiqueIDFromSteam(steam.userID);
            steam.lobbyEventCallback(lobbyID, steamID, SteamLobbyEvent::ON_LOBBY_EXIT);
        }
        steam.lobbyID = {};
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

    SteamLobbyID SteamLobbyGetID() { return static_cast<SteamLobbyID>(global::STEAM_DATA.lobbyID.ConvertToUint64()); }

    SteamID SteamGetLobbyOwner(SteamLobbyID lobby)
    {
        return static_cast<SteamID>(SteamMatchmaking()->GetLobbyOwner(CSteamID((uint64)lobby)).ConvertToUint64());
    }

    void SteamSetLobbyCallback(const SteamLobbyCallback& callback)
    {
        auto& steamData = global::STEAM_DATA;
        steamData.lobbyEventCallback = callback;
    }
    void SteamSetLobbySearchCallback(const SteamLobbySearchCallback& callback)
    {
        global::STEAM_DATA.lobbySearchCallback = callback;
    }

    SteamSearchFilter& SteamSearchFilter::string(std::string_view key, std::string_view value,
                                                 SteamFilterComparison comp)
    {
        SteamMatchmaking()->AddRequestLobbyListStringFilter(key.data(), value.data(),
                                                            static_cast<ELobbyComparison>(comp));
        return *this;
    }

    SteamSearchFilter& SteamSearchFilter::number(std::string_view key, int value, SteamFilterComparison comp)
    {
        SteamMatchmaking()->AddRequestLobbyListNumericalFilter(key.data(), value, static_cast<ELobbyComparison>(comp));
        return *this;
    }

    void SteamSearchLobbies()
    {
        auto call = SteamMatchmaking()->RequestLobbyList();
        auto& steam = global::STEAM_DATA;
        steam.m_CallResultLobbyMatchList.Set(call, &steam, &SteamData::OnLobbyMatchList);
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
