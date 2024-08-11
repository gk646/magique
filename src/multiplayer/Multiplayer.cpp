#include <cxstructs/SmallVector.h>

#include <magique/util/Defines.h>
#include <magique/multiplayer/Multiplayer.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>

#include "internal/globals/MultiplayerData.h"

static void DebugOutput(const ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
    LOG_INFO(pszMsg);
    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
    {
        LOG_ERROR(pszMsg);
    }
}

namespace magique
{
    bool InitMultiplayer()
    {
#if MAGIQUE_STEAM == 1
        SteamNetworkingUtils()->InitRelayNetworkAccess();
#else
        SteamDatagramErrMsg errMsg;
        if (!GameNetworkingSockets_Init(nullptr, errMsg))
            LOG_FATAL("GameNetworkingSockets_Init failed.  %s", errMsg);
#endif
        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
        return true;
    }

    //----------------- HOST -----------------//

    bool CreateListenSocket(const char* adress)
    {
        auto& data = global::MP_DATA;
#if MAGIQUE_STEAM == 1
        data.listenSocket = SteamNetworkingSockets()->CreateListenSocketP2P(0, 0, nullptr);
#else
        SteamNetworkingIPAddr ip{};
        ip.ParseString(adress);
        data.listenSocket = SteamNetworkingSockets()->CreateListenSocketIP(ip, 0, nullptr);
#endif
        data.isHost = false;
        return data.listenSocket != k_HSteamListenSocket_Invalid;
    }

    bool CloseListenSocket(const int closeCode, const char* closeReason)
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

    Connection ConnectToSocket(const SteamID steamID)
    {
        auto& data = global::MP_DATA;
        SteamNetworkingIdentity id{};
        id.SetSteamID({(uint64_t)steamID});
        data.connections[0] = SteamNetworkingSockets()->ConnectP2P(id, 0, 0, nullptr);
        global::MP_DATA.isHost = false;
        return static_cast<Connection>(data.connections[0]);
    }

    Connection ConnectToSocket(const char* adress)
    {
        auto& data = global::MP_DATA;
        SteamNetworkingIPAddr addr{};
        addr.ParseString(adress);
        data.connections[0] = SteamNetworkingSockets()->ConnectByIPAddress(addr, 0, nullptr);
        global::MP_DATA.isHost = false;
        return static_cast<Connection>(data.connections[0]);
    }

    bool DisconnectFromSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        if (data.isHost || data.connections[0] == k_HSteamNetConnection_Invalid)
            return false;
        return SteamNetworkingSockets()->CloseConnection(data.connections[0], closeCode, closeReason, true);
    }

    bool BatchMessage(const Connection conn, const void* message, const int size, const SendFlag flag)
    {
        ASSERT(conn == Connection::INVALID_CONNECTION || message == nullptr || size == 0 ||
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

    bool SendBatch()
    {
        auto& data = global::MP_DATA;
        if (data.messages.empty())
            return false;

        SteamNetworkingSockets()->SendMessages(data.messages.size(), data.messages.data(), nullptr);
        data.messages.clear();
        return true;
    }

    //----------------- UTIL -----------------//

    bool IsHost() { return global::MP_DATA.isHost; }

} // namespace magique