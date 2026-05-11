// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STEAMDATA_H
#define MAGIQUE_STEAMDATA_H

#include <functional>
#include <magique/steam/Steam.h>
#include <magique/steam/Matchmaking.h>

#include "internal/globals/NetworkingData.h"

#ifdef MAGIQUE_STEAM
namespace magique
{
    struct SteamCallback
    {
        STEAM_CALLBACK(SteamCallback, OnLobbyChatUpdate, LobbyChatUpdate_t);
        STEAM_CALLBACK(SteamCallback, OnLobbyDataUpdate, LobbyDataUpdate_t);
        STEAM_CALLBACK(SteamCallback, OnLobbyChatMessage, LobbyChatMsg_t);
        STEAM_CALLBACK(SteamCallback, OnLobbyEntered, LobbyEnter_t);
        STEAM_CALLBACK(SteamCallback, OnJoinRequested, GameLobbyJoinRequested_t);
        STEAM_CALLBACK(SteamCallback, OnConnectionStatusChange, SteamNetConnectionStatusChangedCallback_t);
        STEAM_CALLBACK(SteamCallback, OnNewUrlLaunch, NewUrlLaunchParameters_t);
        STEAM_CALLBACK(SteamCallback, OnUserStats, UserStatsReceived_t);
    };

    struct SteamData final
    {
        SteamCallback* callback = nullptr;
        CCallResult<SteamData, LobbyCreated_t> m_SteamCallResultCreateLobby;
        CCallResult<SteamData, LobbyMatchList_t> m_CallResultLobbyMatchList;

        std::function<void(SteamID, std::string_view)> chatCallback;
        SteamLobbyCallback lobbyEventCallback;
        SteamLobbySearchCallback lobbySearchCallback;
        SteamOverlayCallback overlayCallback = nullptr;
        std::function<void()> urlLaunchCallback;
        std::function<void(const SteamStatResult& res, SteamID)> statsCallback;

        std::string cacheString;
        std::vector<SteamLobbyID> lobbySearchResult;
        CSteamID userID;
        CSteamID lobbyID = CSteamID(0, k_EUniverseInvalid, k_EAccountTypeInvalid);
        bool isInitialized = false;

        void init() { callback = new SteamCallback(); }

        void close()
        {
            delete callback;
            if (lobbyID.IsValid())
            {
                SteamMatchmaking()->LeaveLobby(lobbyID);
                lobbyID.Clear();
            }
            SteamAPI_RunCallbacks(); // Run callbacks one more time - flushing
            SteamAPI_Shutdown();
        }

        void update() { SteamAPI_RunCallbacks(); }

        void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
        void OnLobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure);
    };

    inline CSteamID SteamIDFromMagique(SteamID magiqueID) { return CSteamID{static_cast<uint64>(magiqueID)}; }

    inline SteamID MagiqueIDFromSteam(const CSteamID steamID) { return static_cast<SteamID>(steamID.ConvertToUint64()); }

    inline CSteamID SteamIDFromLobby(SteamLobbyID magiqueID) { return CSteamID{static_cast<uint64>(magiqueID)}; }

    inline SteamLobbyID LobbyIDFromSteam(const CSteamID steamID)
    {
        return static_cast<SteamLobbyID>(steamID.ConvertToUint64());
    }

    inline void SteamCallback::OnConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t* pParam)
    {
        global::MP_DATA.onConnectionStatusChange(pParam);
    }

    namespace global
    {
        inline SteamData STEAM_DATA{};
    } // namespace global

    inline void SteamCallback::OnJoinRequested(GameLobbyJoinRequested_t* pParam)
    {
        auto& steam = global::STEAM_DATA;
        if (steam.lobbyEventCallback)
        {
            const auto lobbyID = LobbyIDFromSteam(pParam->m_steamIDLobby);
            const auto steamID = MagiqueIDFromSteam(pParam->m_steamIDFriend);
            steam.lobbyEventCallback(lobbyID, steamID, SteamLobbyEvent::ON_LOBBY_INVITE);
        }
    }

    inline void SteamCallback::OnLobbyChatUpdate(LobbyChatUpdate_t* pCallback)
    {
        auto& steam = global::STEAM_DATA;
        if (!steam.lobbyEventCallback || pCallback->m_ulSteamIDLobby != steam.lobbyID.ConvertToUint64())
            return;

        const auto steamID = static_cast<SteamID>(pCallback->m_ulSteamIDUserChanged);
        const auto lobbyID = LobbyIDFromSteam(global::STEAM_DATA.lobbyID);
        if ((pCallback->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered) != 0)
        {
            MAGIQUE_ASSERT(pCallback->m_ulSteamIDUserChanged != steam.userID.ConvertToUint64(),
                           "Our event already fired?");
            steam.lobbyEventCallback(lobbyID, steamID, SteamLobbyEvent::ON_USER_JOINED);
        }
        else if ((pCallback->m_rgfChatMemberStateChange & k_EChatMemberStateChangeLeft) != 0)
        {
            MAGIQUE_ASSERT(pCallback->m_ulSteamIDUserChanged != steam.userID.ConvertToUint64(),
                           "Our event already fired?");
            steam.lobbyEventCallback(lobbyID, steamID, SteamLobbyEvent::ON_USER_LEFT);
        }
    }

    inline void SteamCallback::OnLobbyDataUpdate(LobbyDataUpdate_t* pCallback)
    {
        auto& steam = global::STEAM_DATA;
        if (pCallback->m_ulSteamIDLobby == steam.lobbyID.ConvertToUint64())
        {
            // LOG_INFO("Lobby data updated");
        }
    }

    inline void SteamCallback::OnLobbyChatMessage(LobbyChatMsg_t* pCallback)
    {
        auto& steam = global::STEAM_DATA;
        if (pCallback->m_ulSteamIDLobby == steam.lobbyID.ConvertToUint64())
        {
            CSteamID senderID;
            EChatEntryType chatEntryType;
            char message[MAGIQUE_MAX_LOBBY_MESSAGE_LEN];
            const auto chatId = static_cast<int>(pCallback->m_iChatID);
            const int messageSize = SteamMatchmaking()->GetLobbyChatEntry(steam.lobbyID, chatId, &senderID, message,
                                                                          MAGIQUE_MAX_LOBBY_MESSAGE_LEN, &chatEntryType);

            if (messageSize > 0 && chatEntryType == k_EChatEntryTypeChatMsg)
            {
                steam.cacheString = message;
                if (steam.chatCallback)
                    steam.chatCallback(static_cast<SteamID>(senderID.ConvertToUint64()), steam.cacheString);
            }
        }
    }

    inline void SteamCallback::OnLobbyEntered(LobbyEnter_t* pCallback)
    {
        auto& steam = global::STEAM_DATA;
        steam.lobbyID = pCallback->m_ulSteamIDLobby;
        if (steam.lobbyEventCallback)
        {
            const auto lobbyID = LobbyIDFromSteam(steam.lobbyID);
            const auto steamID = MagiqueIDFromSteam(steam.userID);
            steam.lobbyEventCallback(lobbyID, steamID, SteamLobbyEvent::ON_LOBBY_ENTERED);
        }
    }

    inline void SteamData::OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure)
    {
        if (bIOFailure)
        {
            LOG_WARNING("Failed to create lobby");
            return;
        }

        auto& steam = global::STEAM_DATA;
        if (pCallback->m_eResult == k_EResultOK)
        {
            steam.lobbyID = pCallback->m_ulSteamIDLobby;
            if (lobbyEventCallback)
            {
                const auto lobbyID = LobbyIDFromSteam(steam.lobbyID);
                const auto steamID = MagiqueIDFromSteam(steam.userID);
                lobbyEventCallback(lobbyID, steamID, SteamLobbyEvent::ON_LOBBY_CREATED);
            }
        }
    }

    inline void SteamData::OnLobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure)
    {
        if (bIOFailure)
        {
            LOG_WARNING("Failed to retrieve lobby list");
            return;
        }

        lobbySearchResult.clear();
        for (uint32_t i = 0; i < pLobbyMatchList->m_nLobbiesMatching; i++)
        {
            lobbySearchResult.push_back(
                static_cast<SteamLobbyID>(SteamMatchmaking()->GetLobbyByIndex(i).ConvertToUint64()));
        }

        if (lobbySearchCallback)
            lobbySearchCallback(lobbySearchResult);
    }

    inline void SteamCallback::OnNewUrlLaunch(NewUrlLaunchParameters_t* pCallback)
    {
        if (global::STEAM_DATA.urlLaunchCallback)
            global::STEAM_DATA.urlLaunchCallback();
    }

    inline void SteamCallback::OnUserStats(UserStatsReceived_t* pParam)
    {
        SteamStatResult result{static_cast<SteamID>(pParam->m_steamIDUser.ConvertToUint64())};
        if (pParam->m_nGameID == SteamGetAppID() && pParam->m_eResult == k_EResultOK)
            if (global::STEAM_DATA.statsCallback)
                global::STEAM_DATA.statsCallback(result, result.user);
    }

} // namespace magique
#endif

#endif // MAGIQUE_STEAMDATA_H
