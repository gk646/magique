#ifndef MAGIQUE_STEAMDATA_H
#define MAGIQUE_STEAMDATA_H

#include <functional>

#if MAGIQUE_USE_STEAM == 0
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
        CCallResult<SteamData, LobbyCreated_t> m_SteamCallResultCreateLobby;

        std::function<void(SteamID, const std::string&)> chatCallback;
        std::function<void(SteamID, LobbyEvent)> lobbyEventCallback;
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

    inline CSteamID SteamIDFromMagique(SteamID magiqueID) { return {static_cast<uint64_t>(magiqueID)}; }

    inline SteamID MagiqueIDFromSteam(const CSteamID steamID) { return static_cast<SteamID>(steamID.ConvertToUint64()); }

    namespace global
    {
        inline SteamData STEAM_DATA{};
    }
} // namespace magique

#endif //MAGIQUE_STEAMDATA_H