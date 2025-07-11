// SPDX-License-Identifier: zlib-acknowledgement
#include <string>

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

#include <magique/multiplayer/LocalSockets.h>

#ifdef MAGIQUE_STEAM
#include "internal/globals/SteamData.h"
#endif

#include "internal/globals/MultiplayerData.h"

namespace magique
{
    bool InitLocalMultiplayer()
    {
#ifndef MAGIQUE_STEAM
        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
        SteamDatagramErrMsg errMsg;
        if (!GameNetworkingSockets_Init(nullptr, errMsg))
        {
            LOG_FATAL("Initializing local sockets failed.  %s", errMsg);
            return false;
        }
        LOG_INFO("Successfully initialized local sockets");
        global::MP_DATA.isInitialized = true;
        return true;
#else
        MAGIQUE_ASSERT(global::STEAM_DATA.isInitialized, "Steam is not initialized!");
        const auto res = SteamNetworkingSockets()->InitAuthentication();
        SteamNetworkingUtils()->InitRelayNetworkAccess();
        global::MP_DATA.isInitialized = true;
        if (res == k_ESteamNetworkingAvailability_Current)
        {
            global::MP_DATA.isInitialized = true;
            return true;
        }
        return false;
#endif
    }

    //----------------- HOST -----------------//

    bool CreateLocalSocket(const int port)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.isInSession, "Already in session. Close any existing connections or sockets first!");
        MAGIQUE_ASSERT(port < UINT16_MAX, "Port has to be smaller than 65536");
        MAGIQUE_ASSERT(data.isInitialized, "Local multiplayer is not initialized");

        SteamNetworkingConfigValue_t opt{}; // Register callback
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChange);

        SteamNetworkingIPAddr ip{};
        ip.SetIPv4(0, static_cast<uint16>(port));
        data.listenSocket = SteamNetworkingSockets()->CreateListenSocketIP(ip, 1, &opt);
        if (data.listenSocket == k_HSteamListenSocket_Invalid)
        {
            LOG_WARNING("Failed to create listen socket on port %d", port);
            return false;
        }
        LOG_INFO("Successfully created listen socket on port %d", port);
        data.goOnline(true);
        return data.listenSocket != k_HSteamListenSocket_Invalid;
    }

    bool CloseLocalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(data.isInitialized, "Local multiplayer is not initialized");

        if (!data.isInSession || !data.isHost || data.listenSocket == k_HSteamListenSocket_Invalid)
            return false;

        for (const auto conn : data.connections)
        {
            const auto steamConn = static_cast<HSteamNetConnection>(conn);
            SteamNetworkingSockets()->CloseConnection(steamConn, closeCode, closeReason, true);
        }

        const auto res = SteamNetworkingSockets()->CloseListenSocket(data.listenSocket);
        data.goOffline();
        return res;
    }

    //----------------- CLIENT -----------------//

    Connection ConnectToLocalSocket(const char* ip, const uint16_t port)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.isInSession, "Already in session. Close any existing connections or sockets first!");
        MAGIQUE_ASSERT(ip != nullptr, "passed nullptr");
        MAGIQUE_ASSERT(data.isInitialized, "Local multiplayer is not initialized");

        char fullAddress[64]{};
        const int ipLen = static_cast<int>(strlen(ip));
        memcpy(fullAddress, ip, ipLen);
        fullAddress[ipLen] = ':';
        const std::string number = std::to_string(port); // no allocation - small buffer optimized
        memcpy(fullAddress + ipLen + 1, number.c_str(), number.length());

        SteamNetworkingIPAddr addr{};
        addr.Clear();
        if (!addr.ParseString(fullAddress))
        {
            LOG_WARNING("Given ip is not valid!");
            return Connection::INVALID_CONNECTION;
        }
        SteamNetworkingConfigValue_t opt{}; // Set callback
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChange);

        const auto conn = SteamNetworkingSockets()->ConnectByIPAddress(addr, 1, &opt);
        if (conn == k_HSteamNetConnection_Invalid)
        {
            char buffer[128];
            addr.ToString(buffer, 128, true);
            LOG_WARNING("Failed to connect to local socket with ip", buffer);
            return Connection::INVALID_CONNECTION;
        }
        data.connections.push_back(static_cast<Connection>(conn));
        data.goOnline(false);
        return data.connections[0];
    }

    bool DisconnectFromLocalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(data.isInitialized, "Local multiplayer is not initialized");

        if (!data.isInSession || data.isHost || data.connections[0] == Connection::INVALID_CONNECTION)
            return false;

        const auto steamConn = static_cast<HSteamNetConnection>(data.connections[0]);
        const auto res = SteamNetworkingSockets()->CloseConnection(steamConn, closeCode, closeReason, true);
        data.goOffline();
        return res;
    }

    static std::string IP_ADDR{};

    const char* GetLocalIP()
    {
        if (!IP_ADDR.empty()) // Cache
            return IP_ADDR.c_str();
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // Initialize Winsock
        {
            return nullptr;
        }

        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
        {
            WSACleanup();
            return nullptr;
        }

        addrinfo hints = {};
        hints.ai_family = AF_INET; // IPv4
        hints.ai_socktype = SOCK_STREAM;

        addrinfo* info = nullptr;
        if (getaddrinfo(hostname, "http", &hints, &info) != 0)
        {
            WSACleanup();
            return nullptr;
        }

        for (const addrinfo* p = info; p != nullptr;)
        {
            const auto* ipv4 = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
            const auto* ipPointer = inet_ntoa(ipv4->sin_addr);
            IP_ADDR.append(ipPointer);
            break; // Get the first IP
        }
        freeaddrinfo(info);
        WSACleanup();

#else // Unix-based systems (Linux/macOS)
        ifaddrs* ifAddrStruct = nullptr;
        const ifaddrs* ifa = nullptr;

        if (getifaddrs(&ifAddrStruct) == -1)
        {
            return nullptr;
        }

        for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == nullptr)
                continue;

            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                char buff[32];
                void* addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, addr, buff, 32);
                IP_ADDR.append(buff);
                break;
            }
        }
        if (ifAddrStruct != nullptr)
            freeifaddrs(ifAddrStruct);
#endif
        return IP_ADDR.c_str();
    }

} // namespace magique