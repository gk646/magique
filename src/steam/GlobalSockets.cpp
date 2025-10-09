// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <magique/steam/GlobalSockets.h>

#include "internal/globals/MultiplayerData.h"
#include "internal/globals/SteamData.h"

namespace magique
{
    //----------------- HOST -----------------//

    bool InitGlobalMultiplayer()
    {
        if (!global::STEAM_DATA.isInitialized)
        {
            LOG_WARNING("Cannot initialize global multiplayer: Steam not initialized / call InitSteam()");
            return false;
        }
        const auto res = SteamNetworkingSockets()->InitAuthentication();
        SteamNetworkingUtils()->InitRelayNetworkAccess();
        return res == k_ESteamNetworkingAvailability_Current;
    }

    bool CreateGlobalSocket()
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.isInSession, "Already in a session. Close any existing connections or sockets first!");

        data.listenSocket = SteamNetworkingSockets()->CreateListenSocketP2P(0, 0, nullptr);
        if (data.listenSocket == k_HSteamListenSocket_Invalid)
        {
            LOG_WARNING("Failed to create global listen socket");
            return false;
        }
        data.goOnline(true);
        return data.listenSocket != k_HSteamListenSocket_Invalid;
    }

    bool CloseGlobalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        if (!data.isInSession || !data.isHost || data.listenSocket == k_HSteamListenSocket_Invalid)
            return false;

        for (const auto conn : data.connections)
        {
            const auto steamConn = static_cast<HSteamNetConnection>(conn);
            if (!SteamNetworkingSockets()->CloseConnection(steamConn, closeCode, closeReason, true))
            {
                LOG_ERROR("Failed to close existing connections when closing the global socket");
            }
        }

        const auto res = SteamNetworkingSockets()->CloseListenSocket(data.listenSocket);
        data.goOffline();
        return res;
    }

    //----------------- CLIENT -----------------//

    Connection ConnectToGlobalSocket(const SteamID magiqueSteamID)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.isInSession, "Already in session. Close any existing connections or sockets first!");

        const CSteamID steamID{static_cast<uint64>(magiqueSteamID)};
        if (!steamID.IsValid())
        {
            LOG_WARNING("Cannot connect to invalid SteamID");
            return Connection::INVALID_CONNECTION;
        }

        // Creates networking identity
        SteamNetworkingIdentity sni{};
        sni.SetSteamID(steamID);

        const auto conn = SteamNetworkingSockets()->ConnectP2P(sni, 0, 0, nullptr);
        if (conn == k_HSteamNetConnection_Invalid)
        {
            LOG_WARNING("Failed to connect to global socket");
            return Connection::INVALID_CONNECTION;
        }
        data.goOnline(false, static_cast<Connection>(conn));
        return data.connections[0];
    }

    bool DisconnectFromGlobalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        if (!data.isInSession || data.isHost || data.connections[0] == Connection::INVALID_CONNECTION)
            return false;

        const auto steamConn = static_cast<HSteamNetConnection>(data.connections[0]);
        const auto res = SteamNetworkingSockets()->CloseConnection(steamConn, closeCode, closeReason, true);
        data.goOffline();
        return res;
    }

} // namespace magique