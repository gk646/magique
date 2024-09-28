#include <string>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

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
        return true;
#else
        const auto res = SteamNetworkingSockets()->InitAuthentication();
        SteamNetworkingUtils()->InitRelayNetworkAccess();
        return res == k_ESteamNetworkingAvailability_Current;
#endif
    }

    //----------------- HOST -----------------//

    bool CreateLocalSocket(const int port)
    {
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.isInSession, "Already in session. Close any existing connections or sockets first!");

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
        data.goOnline(true);
        return data.listenSocket != k_HSteamListenSocket_Invalid;
    }

    bool CloseLocalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        if (!data.isInSession || !data.isHost || data.listenSocket == k_HSteamListenSocket_Invalid)
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

    Connection ConnectToLocalSocket(const char* ip, const int port)
    {
        MAGIQUE_ASSERT(ip != nullptr, "passed nullptr");
        MAGIQUE_ASSERT(port >= 0 && port <= UINT16_MAX, "passed nullptr");
        auto& data = global::MP_DATA;
        MAGIQUE_ASSERT(!data.isInSession, "Already in session. Close any existing connections or sockets first!");

        char fullAddress[64]{};
        const int ipLen = static_cast<int>(strlen(ip));
        std::memcpy(fullAddress, ip, ipLen);
        fullAddress[ipLen] = ':'; // no allocation - small buffer optimized
        const std::string number = std::to_string(port);
        std::memcpy(fullAddress + ipLen + 1, number.c_str(), number.length());

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
        data.connections[0] = conn;
        data.goOnline(false);
        return static_cast<Connection>(data.connections[0]);
    }

    bool DisconnectFromLocalSocket(const int closeCode, const char* closeReason)
    {
        auto& data = global::MP_DATA;
        if (!data.isInSession || data.isHost || data.connections[0] == k_HSteamNetConnection_Invalid)
            return false;

        const auto res = SteamNetworkingSockets()->CloseConnection(data.connections[0], closeCode, closeReason, true);
        data.goOffline();
        return res;
    }

    static char IP_BUFFER[64]{};

    const char* GetLocalIP()
    {
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

        for (const addrinfo* p = info; p != nullptr; p = p->ai_next)
        {
            auto* ipv4 = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
            auto ipPointer = inet_ntoa(ipv4->sin_addr);
            const int ipLen = (int)strlen(ipPointer);
            std::memcpy(IP_BUFFER, ipPointer, ipLen);
            IP_BUFFER[ipLen] = '\0';
            break; // Get the first IP
        }
        freeaddrinfo(info);
        WSACleanup();

#else // Unix-based systems (Linux/macOS)
        struct ifaddrs* ifAddrStruct = nullptr;
        struct ifaddrs* ifa = nullptr;

        if (getifaddrs(&ifAddrStruct) == -1)
        {
            return localIP;
        }

        for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == nullptr)
                continue;

            if (ifa->ifa_addr->sa_family == AF_INET)
            { // Check it is IPv4
                char addressBuffer[INET_ADDRSTRLEN];
                void* addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, addr, addressBuffer, INET_ADDRSTRLEN);
                localIP = addressBuffer;
                break; // Get the first IPv4
            }
        }

        if (ifAddrStruct != nullptr)
            freeifaddrs(ifAddrStruct);
#endif
        return IP_BUFFER;
    }

} // namespace magique