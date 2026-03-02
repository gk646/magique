// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/multiplayer/LocalSocket.h>

#if defined(MAGIQUE_STEAM) || defined(MAGIQUE_LAN)
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef MAGIQUE_STEAM
#include "internal/globals/SteamData.h"
#endif
#include "internal/globals/NetworkingData.h"
#include "internal/utils/OSUtil.h"

namespace magique
{
    bool LocalSocketInit()
    {
        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
#ifndef MAGIQUE_STEAM
        SteamDatagramErrMsg errMsg;
        if (!GameNetworkingSockets_Init(nullptr, errMsg) ||
            !SteamNetworkingUtils()->SetGlobalConfigValuePtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                                                             (void*)OnConnectionStatusChange))
        {
            LOG_FATAL("Initializing local sockets failed.  %s", errMsg);
            return false;
        }

        LOG_INFO("Initialized local sockets");
        global::MP_DATA.isInitialized = true;
        return true;
#else
        if (!global::STEAM_DATA.isInitialized)
        {
            LOG_WARNING("Cannot initialize local multiplayer: Steam not initialized call InitSteam()");
            return false;
        }
        SteamNetworkingSockets()->InitAuthentication();
        SteamNetworkingUtils()->InitRelayNetworkAccess();
        global::MP_DATA.isInitialized = true;
        return true;
#endif
    }

    bool LocalSocketCreate(const uint16_t port)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.inSession, "Already in session. Close any existing connections or sockets first!");
        MAGIQUE_ASSERT(port < UINT16_MAX, "Port has to be smaller than 65536");
        MAGIQUE_ASSERT(data.isInitialized, "Local multiplayer is not initialized");

        SteamNetworkingIPAddr ip{};
        ip.Clear();
        ip.SetIPv4(0, port);
        data.listenSocket = SteamNetworkingSockets()->CreateListenSocketIP(ip, 0, nullptr);
        if (data.listenSocket == k_HSteamListenSocket_Invalid)
        {
            LOG_WARNING("Failed to create listen socket on port %d", port);
            return false;
        }
        LOG_INFO("Created listen socket on port %d", port);
        data.goOnline(true);
        return data.listenSocket != k_HSteamListenSocket_Invalid;
    }

    Connection LocalSocketConnect(const char* ip, const uint16_t port)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.inSession, "Already in session. Close any existing connections or sockets first!");
        MAGIQUE_ASSERT(ip != nullptr, "passed nullptr");
        MAGIQUE_ASSERT(data.isInitialized, "Local multiplayer is not initialized");

        SteamNetworkingIPAddr addr{};
        addr.Clear();
        if (!addr.ParseString(TextFormat("%s:%d", ip, port)))
        {
            LOG_WARNING("Given IP or port is not valid: %s:%d", ip, port);
            return Connection::INVALID;
        }

        const auto conn = SteamNetworkingSockets()->ConnectByIPAddress(addr, 0, nullptr);
        if (conn == k_HSteamNetConnection_Invalid)
        {
            char buffer[128];
            addr.ToString(buffer, 128, true);
            LOG_WARNING("Failed to connect to local socket with ip", buffer);
            return Connection::INVALID;
        }
        data.goOnline(false, static_cast<Connection>(conn));
        return data.connections[0];
    }

    const char* LocalSocketGetIP() { return OSUtilGetLocalIP(); }
} // namespace magique
#else
namespace magique
{
    bool LocalSocketInit() { return false; }

    bool LocalSocketCreate(const uint16_t port)
    {
        (void)port;
        return false;
    }

    bool LocalSocketClose(const int closeCode, const char* closeReason)
    {
        (void)closeCode;
        (void)closeReason;
        return false;
    }

    Connection LocalSocketConnect(const char* ip, const uint16_t port)
    {
        (void)ip;
        (void)port;
        return Connection::INVALID;
    }

    bool LocalSocketDisconnect(const int closeCode, const char* closeReason)
    {
        (void)closeCode;
        (void)closeReason;
        return false;
    }

    const char* LocalSocketGetIP() { return nullptr; }

} // namespace magique
#endif
