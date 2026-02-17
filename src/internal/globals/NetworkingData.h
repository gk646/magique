// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MULTIPLAYER_DATA_H
#define MAGIQUE_MULTIPLAYER_DATA_H

#include <magique/util/Logging.h>
#include <magique/multiplayer/Networking.h>
#include <magique/multiplayer/Lobby.h>
#include <magique/internal/Macros.h>

#include "multiplayer/headers/MultiplayerStatistics.h"
#include "multiplayer/headers/LobbyData.h"
#include "multiplayer/headers/ConnectionMapping.h"

#ifdef MAGIQUE_LAN
#include "external/networkingsockets/steamnetworkingsockets.h"
#include "external/networkingsockets/isteamnetworkingutils.h"
#elif MAGIQUE_STEAM
#include <steam/steam_api.h>
#else
#error "Using Networking without enabling it in CMake! Set the networking status in CMakeLists.txt in the root!"
#endif

using MessageVec = std::vector<SteamNetworkingMessage_t*>;

inline void DebugOutput(const ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Msg)
    {
        LOG_INFO(pszMsg);
    }
    else if (eType == k_ESteamNetworkingSocketsDebugOutputType_Warning)
    {
        LOG_WARNING(pszMsg);
    }
    else if (eType == k_ESteamNetworkingSocketsDebugOutputType_Important)
    {
        LOG_WARNING(pszMsg);
    }
    else if (eType == k_ESteamNetworkingSocketsDebugOutputType_Error ||
             eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
    {
        LOG_ERROR(pszMsg);
    }
}

namespace magique
{
    struct NetworkingData final
    {
        LobbyData lobby;
        MultiplayerStatistics statistics{};
        NetworkCallback callback;                                       // Callback
        std::vector<Connection> connections;                            // Holds all current valid connections
        std::vector<ConnMapping> connectionMapping;                     // Holds all the manually set mappings
        std::vector<Message> incMsgVec;                                 // Incoming magique::Messages
        MessageVec outMsgBuffer;                                        // Outgoing message buffer
        MessageVec incMsgBuffer;                                        // Incoming message buffer
        ConnNumberMapping numberMapping{};                              // Maps connection to a consistent number
        HSteamListenSocket listenSocket = k_HSteamListenSocket_Invalid; // The global listen socket
        bool isHost = false;                                            // If the program is host or client
        bool inSession = false;                                         // If program is part of multiplayer activity
        bool isInitialized = false;                                     // Manual flag to give clean error msg

        NetworkingData()
        {
            incMsgVec.reserve(150);
            outMsgBuffer.reserve(150);
            incMsgBuffer.reserve(150);
            connections.reserve(8);
            connectionMapping.reserve(8);
        }

        void close()
        {
#ifdef MAGIQUE_LAN
            GameNetworkingSockets_Kill();
#else
            for (const auto conn : connections)
            {
                const char* msg = nullptr;
                if (isHost)
                    msg = "Host closed application abruptly";
                else
                    msg = "Client closed application abruptly";
                const auto steamConn = static_cast<HSteamNetConnection>(conn);
                SteamNetworkingSockets()->CloseConnection(steamConn, 0, msg, false);
                // Second time to flush?
                SteamNetworkingSockets()->CloseConnection(steamConn, 0, nullptr, false);
            }
            goOffline();
#endif
        }

        void goOnline(const bool asHost, Connection conn = Connection::INVALID)
        {
            if (asHost) // For client deferred until host actually accepts
            {
                inSession = true;
            }
            else if (conn != Connection::INVALID)
            {
                connections.push_back(conn);
                numberMapping.addConnection(conn);
            }
            isHost = asHost;
            statistics.reset();
        }

        void goOffline()
        {
            if (listenSocket != k_HSteamListenSocket_Invalid)
            {
                SteamNetworkingSockets()->CloseListenSocket(listenSocket);
                listenSocket = k_HSteamListenSocket_Invalid;
            }
            MAGIQUE_ASSERT(listenSocket == k_HSteamListenSocket_Invalid, "Socket wasnt closed!");
            // Release the batch if exists
            for (const auto msg : outMsgBuffer)
            {
                msg->Release();
            }
            outMsgBuffer.clear();
            for (const auto msg : incMsgBuffer)
            {
                if (msg != nullptr)
                    msg->Release();
            }
            incMsgVec.clear();
            incMsgBuffer.clear();
            connections.clear();
            connectionMapping.clear();
            isHost = false;
            inSession = false;
            numberMapping.clear();
            lobby.closeLobby();
        }

        void update() const
        {
#ifndef MAGIQUE_STEAM
            if (isInitialized)
            {
                SteamNetworkingSockets()->RunCallbacks();
            }
#endif
        }

        // Called if host and a client disconnects or is removed
        void onClientDisconnected(Connection client)
        {
            numberMapping.removeConnection(client);
            std::erase_if(connectionMapping, [&](auto& mapping) { return mapping.conn == client; });
            std::erase(connections, client);
        }

        void onConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t* pParam)
        {
            const auto conn = static_cast<Connection>(pParam->m_hConn);
            if (isHost)
            {
                // New connection arriving at a listen socket
                if (pParam->m_info.m_hListenSocket == listenSocket &&
                    pParam->m_eOldState == k_ESteamNetworkingConnectionState_None &&
                    pParam->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting)
                {
                    if (SteamNetworkingSockets()->AcceptConnection(pParam->m_hConn) == k_EResultOK)
                    {
                        if (connections.size() == MAGIQUE_MAX_PLAYERS - 1)
                        {
                            LOG_WARNING("Configured client limit is reached! MAGIQUE_MAX_PLAYERS - 1: %d",
                                        MAGIQUE_MAX_PLAYERS - 1);
                        }
                        else
                        {
                            connections.push_back(conn);
                            numberMapping.addConnection(conn);
                            if (callback)
                            {
                                callback(NetworkEvent::HOST_NEW_CONNECTION, conn);
                            }
                            LOG_INFO("Host accepted a new client connection");
                        }
                    }
                    else
                    {
                        LOG_WARNING("Failed to accept a new client connection.");
                    }
                }
            }

            if (!isHost)
            {
                // Connection has been accepted by the remote host
                if ((pParam->m_eOldState == k_ESteamNetworkingConnectionState_Connecting ||
                     pParam->m_eOldState == k_ESteamNetworkingConnectionState_FindingRoute) &&
                    pParam->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected)
                {
                    inSession = true; // Deferred until here - could be just connection build up
                    LOG_INFO("A connection initiated by us was accepted by the remote host.");
                    if (callback)
                    {
                        callback(NetworkEvent::CLIENT_CONNECTION_ACCEPTED, conn);
                    }
                }
            }

            // Connection actively rejected or closed by the remote host
            if ((pParam->m_eOldState == k_ESteamNetworkingConnectionState_Connecting ||
                 pParam->m_eOldState == k_ESteamNetworkingConnectionState_Connected) &&
                pParam->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer)
            {
                SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, nullptr, false);
                if (isHost)
                {
                    if (callback)
                    {
                        callback(NetworkEvent::HOST_CLIENT_DISCONNECTED, conn);
                    }
                    onClientDisconnected(conn);
                    LOG_INFO("Client disconnected: %s", pParam->m_info.m_szEndDebug);
                }
                else // If you're a client and the host disconnects
                {
                    if (callback)
                    {
                        callback(NetworkEvent::CLIENT_CONNECTION_CLOSED, conn);
                    }
                    goOffline();
                    LOG_INFO("Disconnected from the host: %s", pParam->m_info.m_szEndDebug);
                }
            }

            // Connection closed locally due to a problem
            if ((pParam->m_eOldState == k_ESteamNetworkingConnectionState_Connecting ||
                 pParam->m_eOldState == k_ESteamNetworkingConnectionState_Connected) &&
                pParam->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
            {
                SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, nullptr, false);
                const auto* errStr = pParam->m_info.m_szEndDebug;
                if (isHost)
                {
                    if (callback)
                    {
                        callback(NetworkEvent::HOST_LOCAL_PROBLEM, conn);
                    }
                    onClientDisconnected(conn);
                    LOG_INFO("Local problem with connection. Disconnected client from session: %s", errStr);
                }
                else
                {
                    if (callback)
                    {
                        callback(NetworkEvent::CLIENT_LOCAL_PROBLEM, conn);
                    }
                    goOffline();
                    LOG_INFO("Local problem with connection. Closed session: %s", errStr);
                }
            }
        }
    };

    namespace global
    {
        inline NetworkingData MP_DATA{};
    } // namespace global

    inline void OnConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t* pParam)
    {
        global::MP_DATA.onConnectionStatusChange(pParam);
    }
} // namespace magique

#endif // MAGIQUE_MULTIPLAYER_DATA_H
