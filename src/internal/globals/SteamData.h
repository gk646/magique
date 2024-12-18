// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STEAMDATA_H
#define MAGIQUE_STEAMDATA_H

#include <functional>

#ifndef MAGIQUE_STEAM
#error "Using Steam features without enabling it! To enable Steam use CMake: MAGIQUE_STEAM"
#else
#include <steam/steam_api.h>
#endif

namespace magique
{
    struct SteamData final
    {
        STEAM_CALLBACK(SteamData, OnLobbyChatUpdate, LobbyChatUpdate_t) const;
        STEAM_CALLBACK(SteamData, OnLobbyDataUpdate, LobbyDataUpdate_t) const;
        STEAM_CALLBACK(SteamData, OnLobbyChatMessage, LobbyChatMsg_t);
        STEAM_CALLBACK(SteamData, OnLobbyEntered, LobbyEnter_t) const;
        STEAM_CALLBACK(SteamData, OnJoinRequested, GameLobbyJoinRequested_t) const;

        CCallResult<SteamData, LobbyCreated_t> m_SteamCallResultCreateLobby;

        std::function<void(SteamID, const std::string&)> chatCallback;
        std::function<void(LobbyID, SteamID, LobbyEvent)> lobbyEventCallback;
        std::string cacheString;
        SteamOverlayCallback overlayCallback = nullptr;

        CSteamID userID;
        CSteamID lobbyID = CSteamID(0, k_EUniverseInvalid, k_EAccountTypeInvalid);
        bool isInitialized = false;

        void close()
        {
            SteamAPI_RunCallbacks(); // Run callbacks one more time - flushing
            SteamAPI_Shutdown();
        }

        void update() { SteamAPI_RunCallbacks(); }

        void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
    };

    inline CSteamID SteamIDFromMagique(SteamID magiqueID) { return CSteamID{static_cast<uint64>(magiqueID)}; }

    inline SteamID MagiqueIDFromSteam(const CSteamID steamID) { return static_cast<SteamID>(steamID.ConvertToUint64()); }

    inline CSteamID SteamIDFromLobby(LobbyID magiqueID) { return CSteamID{static_cast<uint64>(magiqueID)}; }

    inline LobbyID LobbyIDFromSteam(const CSteamID steamID) { return static_cast<LobbyID>(steamID.ConvertToUint64()); }

    inline void SteamData::OnJoinRequested(GameLobbyJoinRequested_t* pParam) const
    {
        if (lobbyEventCallback)
        {
            const auto lobbyID = LobbyIDFromSteam(pParam->m_steamIDLobby);
            const auto steamID = MagiqueIDFromSteam(pParam->m_steamIDFriend);
            lobbyEventCallback(lobbyID, steamID, LobbyEvent::ON_LOBBY_INVITE);
        }
    }

    namespace global
    {
        inline SteamData STEAM_DATA{};
    }
} // namespace magique

#endif //MAGIQUE_STEAMDATA_H