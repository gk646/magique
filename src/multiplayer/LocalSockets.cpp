#include <magique/multiplayer/LocalSockets.h>

#include "internal/globals/MultiplayerData.h"

namespace magique
{
    bool InitLocalMultiplayer()
    {
#if MAGIQUE_USE_STEAM == 0
        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
        SteamDatagramErrMsg errMsg;
        if (!GameNetworkingSockets_Init(nullptr, errMsg))
        {
            LOG_FATAL("GameNetworkingSockets_Init failed.  %s", errMsg);
            return false;
        }
#endif
        return true;
    }

    //----------------- HOST -----------------//

    bool CreateLocalSocket(const int port)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.inSession, "Already in session. Close any existing connections or sockets first!");

        SteamNetworkingConfigValue_t opt{}; // Register callback
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChange);

        SteamNetworkingIPAddr ip{};
        ip.SetIPv4(0, static_cast<uint16>(port));
        data.listenSocket = SteamNetworkingSockets()->CreateListenSocketIP(ip, 0, &opt);

        data.goOnline(true);
        return data.listenSocket != k_HSteamListenSocket_Invalid;
    }

    bool CloseLocalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        if (!data.inSession || !data.isHost || data.listenSocket == k_HSteamListenSocket_Invalid)
            return false;

        for (const auto conn : data.connections)
        {
            if (conn != k_HSteamNetConnection_Invalid)
            {
                SteamNetworkingSockets()->CloseConnection(conn, closeCode, closeReason, true);
            }
        }

        const auto res = SteamNetworkingSockets()->CloseListenSocket(data.listenSocket);
        data.goOffline();

        return res;
    }

    //----------------- CLIENT -----------------//

    Connection ConnectToLocalSocket(const int ip, const int port)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.inSession, "Already in session. Close any existing connections or sockets first!");

        SteamNetworkingIPAddr addr{};
        addr.SetIPv4(ip, static_cast<uint16>(port));

        SteamNetworkingConfigValue_t opt{}; // Set callback
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChange);

        data.connections[0] = SteamNetworkingSockets()->ConnectByIPAddress(addr, 1, &opt);

        data.goOnline(false);
        return static_cast<Connection>(data.connections[0]);
    }

    bool DisconnectFromLocalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        if (!data.inSession || data.isHost || data.connections[0] == k_HSteamNetConnection_Invalid)
            return false;

        const auto res = SteamNetworkingSockets()->CloseConnection(data.connections[0], closeCode, closeReason, true);
        data.goOffline();
        return res;
    }

    //----------------- UTIL -----------------//

    uint32_t GetIPAdress()
    {
        SteamNetworkingIdentity id{};
        SteamNetworkingSockets()->GetIdentity(&id);
        return id.GetIPv4();
    }

} // namespace magique