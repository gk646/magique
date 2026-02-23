// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/steam/GlobalSocket.h>
#ifndef MAGIQUE_STEAM
#include "magique/util/Logging.h"
namespace magique
{
    bool GlobalSocketInit() { M_ENABLE_STEAM_ERROR(false) }
    bool GlobalSocketCreate() { M_ENABLE_STEAM_ERROR(false) }
    Connection GlobalSocketConnect(const SteamID magiqueSteamID) { M_ENABLE_STEAM_ERROR({}) }
} // namespace magique
#else
#include "internal/globals/NetworkingData.h"
#include "internal/globals/SteamData.h"

namespace magique
{
    bool GlobalSocketInit()
    {
        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
        if (!global::STEAM_DATA.isInitialized)
        {
            LOG_WARNING("Cannot initialize global multiplayer: Steam not initialized / call InitSteam()");
            return false;
        }
        const auto res = SteamNetworkingSockets()->InitAuthentication();
        SteamNetworkingUtils()->InitRelayNetworkAccess();
        return res == k_ESteamNetworkingAvailability_Current;
    }

    bool GlobalSocketCreate()
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.inSession, "Already in a session. Close any existing connections or sockets first!");

        data.listenSocket = SteamNetworkingSockets()->CreateListenSocketP2P(0, 0, nullptr);
        if (data.listenSocket == k_HSteamListenSocket_Invalid)
        {
            LOG_WARNING("Failed to create global listen socket");
            return false;
        }
        data.goOnline(true);
        return data.listenSocket != k_HSteamListenSocket_Invalid;
    }

    Connection GlobalSocketConnect(const SteamID magiqueSteamID)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.inSession, "Already in session. Close any existing connections or sockets first!");

        const CSteamID steamID{static_cast<uint64>(magiqueSteamID)};
        if (!steamID.IsValid())
        {
            LOG_WARNING("Cannot connect to invalid SteamID");
            return Connection::INVALID;
        }

        // Creates networking identity
        SteamNetworkingIdentity sni{};
        sni.SetSteamID(steamID);

        const auto conn = SteamNetworkingSockets()->ConnectP2P(sni, 0, 0, nullptr);
        if (conn == k_HSteamNetConnection_Invalid)
        {
            LOG_WARNING("Failed to connect to global socket");
            return Connection::INVALID;
        }
        data.goOnline(false, static_cast<Connection>(conn));
        return data.connections[0];
    }

} // namespace magique
#endif
