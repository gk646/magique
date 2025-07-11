// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MULTIPLAYER_DATA_H
#define MAGIQUE_MULTIPLAYER_DATA_H

#include <magique/util/Logging.h>
#include <magique/multiplayer/Multiplayer.h>
#include <magique/internal/Macros.h>

#include "internal/datastructures/VectorType.h"
#include "internal/utils/STLUtil.h"

#ifdef MAGIQUE_LAN
#include "external/networkingsockets/steamnetworkingsockets.h"
#include "external/networkingsockets/isteamnetworkingutils.h"
#elif MAGIQUE_STEAM
#include <steam/steam_api.h>
#else
#error "Using Networking without enabling it in CMake! Set the networking status in CMakeLists.txt in the root!"
#endif

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
    struct ConnMapping final
    {
        Connection conn;
        entt::entity entity;

        static bool DeleteFunc(const ConnMapping& m1, const ConnMapping& m2) { return m1.conn == m2.conn; }
    };


    struct MultiplayerData final
    {
        MultiplayerCallback callback;                                   // Callback
        std::vector<Connection> connections;                            // Holds all current valid connections
        std::vector<ConnMapping> connectionMapping;                     // Holds all the manually set mapping
        std::vector<Message> msgVec;                                    // Message buffer
        vector<SteamNetworkingMessage_t*> batchedMsgs;                  // Outgoing message buffer
        SteamNetworkingMessage_t** msgBuffer = nullptr;                 // Incoming message data buffer
        HSteamListenSocket listenSocket = k_HSteamListenSocket_Invalid; // The global listen socket
        int buffSize = 0;                                               // Buffer size
        int buffCap = 0;                                                // Buffer capacity
        bool isHost = false;                                            // If the program is host or client
        bool isInSession = false;                                       // If program is part of multiplayer activity
        bool isInitialized = false;                                     // Manual flag to give clean error msg

        MultiplayerData()
        {
            msgVec.reserve(101);
            batchedMsgs.reserve(MAGIQUE_ESTIMATED_MESSAGES);
            msgBuffer = new SteamNetworkingMessage_t*[MAGIQUE_ESTIMATED_MESSAGES];
            buffCap = MAGIQUE_ESTIMATED_MESSAGES;
            connections.reserve(MAGIQUE_MAX_PLAYERS + 1);
            connectionMapping.reserve(MAGIQUE_MAX_PLAYERS + 1);
        }

        void close()
        {
            delete[] msgBuffer;
            isInSession = false;
            for (auto* const msg : batchedMsgs)
            {
                msg->Release();
            }
            batchedMsgs.clear();
#ifdef MAGIQUE_LAN
            GameNetworkingSockets_Kill();
#else
            for (const auto conn : connections)
            {
                const char* msg = nullptr;
                if (isHost)
                    msg = "Host closed application";
                else
                    msg = "Client closed application";
                const auto steamConn = static_cast<HSteamNetConnection>(conn);
                SteamNetworkingSockets()->CloseConnection(steamConn, 0, msg, false);
            }
            if (listenSocket != k_HSteamListenSocket_Invalid)
                SteamNetworkingSockets()->CloseListenSocket(listenSocket);
            if (isInitialized)
                SteamNetworkingSockets()->RunCallbacks();
#endif
        }

        void goOnline(const bool asHost)
        {
            isInSession = true;
            isHost = asHost;
        }

        void goOffline()
        {
            MAGIQUE_ASSERT(listenSocket == k_HSteamListenSocket_Invalid, "Socket wasnt closed!");
            connections.clear();
            connectionMapping.clear();
            isHost = false;
            isInSession = false;
            msgVec.clear();
            buffSize = 0;
            // Release the batch if exists
            for (const auto msg : batchedMsgs)
            {
                msg->Release();
            }
            batchedMsgs.clear();
        }

        void update() const
        {
            if (isInSession)
            {
                SteamNetworkingSockets()->RunCallbacks();
            }
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
                        if (connections.size() == MAGIQUE_MAX_PLAYERS)
                        {
                            LOG_WARNING("Configured client limit is reached! MAGIQUE_MAX_PLAYERS: %d",
                                        MAGIQUE_MAX_PLAYERS);
                        }
                        else
                        {
                            connections.push_back(conn);
                            if (callback)
                            {
                                callback(MultiplayerEvent::HOST_NEW_CONNECTION, conn);
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
                if (pParam->m_eOldState == k_ESteamNetworkingConnectionState_Connecting &&
                    pParam->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected)
                {
                    LOG_INFO("A connection initiated by us was accepted by the remote host.");
                    if (callback)
                    {
                        callback(MultiplayerEvent::CLIENT_CONNECTION_ACCEPTED, static_cast<Connection>(pParam->m_hConn));
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
                    UnorderedDelete(connections, static_cast<Connection>(pParam->m_hConn));
                    if (callback)
                    {
                        callback(MultiplayerEvent::HOST_CLIENT_DISCONNECTED, conn);
                    }
                    UnorderedDelete(connectionMapping, ConnMapping{conn, entt::entity{0}}, ConnMapping::DeleteFunc);
                    LOG_INFO("Client disconnected: %s", pParam->m_info.m_szEndDebug);
                }
                else // If you're a client and the host disconnects
                {
                    if (callback)
                    {
                        callback(MultiplayerEvent::CLIENT_CONNECTION_CLOSED, conn);
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
                    UnorderedDelete(connections, static_cast<Connection>(pParam->m_hConn));
                    if (callback)
                    {
                        callback(MultiplayerEvent::HOST_LOCAL_PROBLEM, conn);
                    }
                    UnorderedDelete(connectionMapping, ConnMapping{conn, entt::entity{0}}, ConnMapping::DeleteFunc);
                    LOG_INFO("Local problem with connection. Disconnected client from session: %s", errStr);
                }
                else
                {
                    if (callback)
                    {
                        callback(MultiplayerEvent::CLIENT_LOCAL_PROBLEM, conn);
                    }
                    goOffline();
                    LOG_INFO("Local problem with connection. Closed session: %s", errStr);
                }
            }
        }
    };

    namespace global
    {
        inline MultiplayerData MP_DATA{};
    }

    inline void OnConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t* pParam)
    {
        global::MP_DATA.onConnectionStatusChange(pParam);
    }
} // namespace magique

#endif //MAGIQUE_MULTIPLAYER_DATA_H