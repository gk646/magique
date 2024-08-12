#include <magique/util/Defines.h>
#include <magique/multiplayer/LocalSockets.h>

#include "internal/globals/MultiplayerData.h"

namespace magique
{
    bool InitLocalMultiplayer()
    {
#if MAGIQUE_STEAM == 0
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

    bool CreateLocalSocket(int port)
    {
        auto& data = global::MP_DATA;
        SteamNetworkingConfigValue_t opt{};
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChange);
        SteamNetworkingIPAddr ip{};
        ip.SetIPv4(0,port);
        data.listenSocket = SteamNetworkingSockets()->CreateListenSocketIP(ip, 0, &opt);
        data.isHost = false;
        data.isOnline = true;
        return data.listenSocket != k_HSteamListenSocket_Invalid;
    }

    bool CloseLocalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        if (!data.isHost || data.listenSocket == k_HSteamListenSocket_Invalid)
            return false;

        for (const auto conn : data.connections)
        {
            if (conn != k_HSteamNetConnection_Invalid)
            {
                SteamNetworkingSockets()->CloseConnection(conn, closeCode, closeReason, true);
            }
        }
        return SteamNetworkingSockets()->CloseListenSocket(data.listenSocket);
    }

    //----------------- CLIENT -----------------//

    Connection ConnectToLocalSocket(const int port)
    {
        auto& data = global::MP_DATA;
        SteamNetworkingIPAddr addr{};
        addr.SetIPv4(0, static_cast<uint16>(port));
        SteamNetworkingConfigValue_t opt{};
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChange);
        data.connections[0] = SteamNetworkingSockets()->ConnectByIPAddress(addr, 1, &opt);
        global::MP_DATA.isHost = false;
        return static_cast<Connection>(data.connections[0]);
    }

    bool DisconnectFromLocalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        if (data.isHost || data.connections[0] == k_HSteamNetConnection_Invalid)
            return false;
        return SteamNetworkingSockets()->CloseConnection(data.connections[0], closeCode, closeReason, true);
    }

    //----------------- UTIL -----------------//

    uint32_t GetIPAdress()
    {
        SteamNetworkingIdentity id;
        SteamNetworkingSockets()->GetIdentity(&id);
        return id.GetIPv4();
    }

} // namespace magique