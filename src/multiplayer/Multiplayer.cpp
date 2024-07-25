
#include <fstream>
#include <steam/steam_api.h>
#include <cxstructs/SmallVector.h>

#include <magique/util/Defines.h>
#include <magique/multiplayer/Multiplayer.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>

static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
    printf("%s\n", pszMsg);
    fflush(stdout);
    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
    {
        fflush(stdout);
        fflush(stderr);
        exit(1);
    }
}

namespace magique
{
    cxstructs::SmallVector<SteamNetworkingMessage_t*, MAGIQUE_MESSAGES_ESTIMATE> MESSAGES;

    bool InitMultiplayer()
    {
#if MAGIQUE_STEAM == 1
        SteamNetworkingUtils()->InitRelayNetworkAccess();
#else
#endif
       // SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
        return true;
    }

    Connection CreateListenSocket()
    {
        HSteamListenSocket socket;
#if MAGIQUE_STEAM == 1
        socket = SteamNetworkingSockets()->CreateListenSocketP2P(0, 0, nullptr);
#else
        SteamNetworkingIPAddr ip;
        ip.SetIPv4(0, 42000);
       // socket = SteamNetworkingSockets()->CreateListenSocketIP(ip, 0, nullptr);
#endif
        return static_cast<Connection>(socket);
    }

    bool BatchMessage(const Connection conn, const void* message, const int size, SendFlag flag)
    {
        if (conn == Connection::INVALID_CONNECTION || message == nullptr || size == 0 ||
            (flag != SendFlag::UN_RELIABLE && flag != SendFlag::RELIABLE))
            return false;

        // Allocate with buffer
        /*
        const auto msg = SteamNetworkingUtils()->AllocateMessage(size);
        std::memcpy(msg->m_pData, message, size);
        msg->m_nFlags = static_cast<int>(flag);
        msg->m_conn = static_cast<uint32_t>(conn);

        MESSAGES.push_back(msg);
        */
        return true;
    }

    bool SendBatch()
    {
        if (MESSAGES.empty())
            return false;
    }


} // namespace magique