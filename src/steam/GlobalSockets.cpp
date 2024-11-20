#define _CRT_SECURE_NO_WARNINGS
#include <steam/steam_api.h>

#include <magique/steam/GlobalSockets.h>

#include "internal/globals/MultiplayerData.h"

namespace magique
{
    //----------------- HOST -----------------//

    bool InitGlobalMultiplayer()
    {
        const auto res = SteamNetworkingSockets()->InitAuthentication();
        SteamNetworkingUtils()->InitRelayNetworkAccess();
        return res == k_ESteamNetworkingAvailability_Current;
    }

    bool CreateGlobalSocket()
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.isInSession, "Already in a session. Close any existing connections or sockets first!");

        SteamNetworkingConfigValue_t opt{}; // Register callback
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChange);

        data.listenSocket = SteamNetworkingSockets()->CreateListenSocketP2P(0, 1, &opt);
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
        if (steamID.IsValid())
        {
            LOG_WARNING("Passed invalid steam id");
            return Connection::INVALID_CONNECTION;
        }

        // Creates networking identity
        SteamNetworkingIdentity sni{};
        sni.SetSteamID(steamID);

        SteamNetworkingConfigValue_t opt{}; // Set callback
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChange);

        const auto conn = SteamNetworkingSockets()->ConnectP2P(sni, 0, 1, &opt);
        if (conn == k_HSteamNetConnection_Invalid)
        {
            LOG_WARNING("Failed to connect to global socket");
            return Connection::INVALID_CONNECTION;
        }
        data.connections.push_back(static_cast<Connection>(conn));
        data.goOnline(false);
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