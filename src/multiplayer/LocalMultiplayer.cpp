#include <magique/util/Defines.h>
#include <magique/multiplayer/LocalMultiplayer.h>
#include <magique/internal/Macros.h>

#include "internal/globals/LocalMultiplayerData.h"

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

    bool CreateLocalSocket(const char* adress)
    {
        auto& data = global::MP_DATA;
        SteamNetworkingConfigValue_t opt{};
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChange);
        SteamNetworkingIPAddr ip{};
        ip.ParseString(adress);
        data.listenSocket = SteamNetworkingSockets()->CreateListenSocketIP(ip, 0, &opt);
        data.isHost = false;
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

    LocalConnection ConnectLocalSocket(const int port)
    {
        auto& data = global::MP_DATA;
        SteamNetworkingIPAddr addr{};
        addr.SetIPv4(0, static_cast<uint16>(port));
        SteamNetworkingConfigValue_t opt{};
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChange);
        data.connections[0] = SteamNetworkingSockets()->ConnectByIPAddress(addr, 1, &opt);
        global::MP_DATA.isHost = false;
        return static_cast<LocalConnection>(data.connections[0]);
    }

    bool DisconnectLocalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        if (data.isHost || data.connections[0] == k_HSteamNetConnection_Invalid)
            return false;
        return SteamNetworkingSockets()->CloseConnection(data.connections[0], closeCode, closeReason, true);
    }

    bool BatchLocalMessage(const LocalConnection conn, const void* message, const int size, const SendFlag flag)
    {
        ASSERT(conn == LocalConnection::INVALID_CONNECTION || message == nullptr || size == 0 ||
                   (flag != SendFlag::UN_RELIABLE && flag != SendFlag::RELIABLE),
               "Passed invalid input parameters");

        if (message == nullptr) [[unlikely]] // This can cause crash
            return false;

        // Allocate with buffer
        const auto msg = SteamNetworkingUtils()->AllocateMessage(size);
        std::memcpy(msg->m_pData, message, size);
        msg->m_nFlags = static_cast<int>(flag);
        msg->m_conn = static_cast<HSteamNetConnection>(conn);
        global::MP_DATA.messages.push_back(msg);
        return true;
    }

    bool SendLocalBatch()
    {
        auto& data = global::MP_DATA;
        if (data.messages.empty())
            return false;

        SteamNetworkingSockets()->SendMessages(data.messages.size(), data.messages.data(), nullptr);
        data.messages.clear();
        return true;
    }

    bool SendLocalMessage(LocalConnection conn, const void* message, int size, SendFlag flag)
    {
        ASSERT((int)conn != k_HSteamNetConnection_Invalid, "Invalid connection");
        ASSERT(flag == SendFlag::RELIABLE || flag == SendFlag::UN_RELIABLE, "Invalid flag");
        if (message == nullptr || size == 0) [[unlikely]]
            return false;
        SteamNetworkingSockets()->SendMessageToConnection((int)conn, message, size, (int)flag, nullptr);
        return true;
    }

    uint32_t GetIPAdress()
    {
        SteamNetworkingIdentity id;
        SteamNetworkingSockets()->GetIdentity(&id);
        return id.GetIPv4();
    }

    //----------------- UTIL -----------------//

    bool IsHost() { return global::MP_DATA.isHost; }

} // namespace magique