#ifndef MAGIQUE_MULTIPLAYER_DATA_H
#define MAGIQUE_MULTIPLAYER_DATA_H

#include <functional>

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>

#if MAGIQUE_USE_STEAM == 0
#include "external/networkingsockets/steamnetworkingsockets.h"
#include "external/networkingsockets/isteamnetworkingutils.h"
#else
#include <steam/steam_api.h>
#endif

#include "internal/datastructures/VectorType.h"

inline void DebugOutput(const ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
    LOG_INFO(pszMsg);
    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
    {
        LOG_ERROR(pszMsg);
    }
}

namespace magique
{
    struct MultiplayerData final
    {
        HSteamListenSocket listenSocket = k_HSteamListenSocket_Invalid; // The global listen socket
        bool isHost = false;                                            // If the program is host or client
        bool isInSession = false;                                       // If program is part of multiplayer activity
        HSteamNetConnection connections[MAGIQUE_MAX_ACTORS]{};          // All possible outgoing connections as host
        std::function<void(MultiplayerEvent)> callback;                 // Callback
        vector<SteamNetworkingMessage_t*> batchedMsgs;                  // Outgoing message buffer
        std::vector<Message> msgVec{};                                  // Message buffer
        SteamNetworkingMessage_t** msgBuffer = nullptr;                 // Incoming message data buffer
        int buffSize = 0;                                               // Buffer size
        int buffCap = 0;                                                // Buffer capacity

        MultiplayerData()
        {
            batchedMsgs.reserve(MAGIQUE_ESTIMATED_MESSAGES);
            msgBuffer = new SteamNetworkingMessage_t*[MAGIQUE_ESTIMATED_MESSAGES];
            buffCap = MAGIQUE_ESTIMATED_MESSAGES;
        }

        ~MultiplayerData()
        {
            delete[] msgBuffer;
#if MAGIQUE_STEAM == 0
            GameNetworkingSockets_Kill();
#endif
            isInSession = false;
        }

        void goOnline(const bool asHost)
        {
            isInSession = true;
            isHost = asHost;
        }

        void goOffline()
        {
            MAGIQUE_ASSERT(listenSocket == k_HSteamListenSocket_Invalid, "Socket wasnt closed!");
            std::memset(connections, 0, sizeof(int) * MAGIQUE_MAX_ACTORS);
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
#if MAGIQUE_STEAM == 0
                SteamNetworkingSockets()->RunCallbacks();
#endif
            }
        }

        void onConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t* pParam)
        {
            if (isHost)
            {
                // New connection arriving at a listen socket
                if (pParam->m_info.m_hListenSocket == listenSocket &&
                    pParam->m_eOldState == k_ESteamNetworkingConnectionState_None &&
                    pParam->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting)
                {
                    if (SteamNetworkingSockets()->AcceptConnection(pParam->m_hConn) == k_EResultOK)
                    {
                        LOG_INFO("Host accepted a new client connection");
                        bool accepted = false;
                        for (auto& clientConn : connections)
                        {
                            if (clientConn == k_HSteamNetConnection_Invalid)
                            {
                                clientConn = pParam->m_hConn;
                                accepted = true;
                                if (callback)
                                    callback(MultiplayerEvent::HOST_NEW_CONNECTION);
                                break;
                            }
                        }
                        if (!accepted)
                            LOG_WARNING("Trying to accept more connections than configured: %d", MAGIQUE_MAX_ACTORS);
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
                        callback(MultiplayerEvent::CLIENT_CONNECTION_ACCEPTED);
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
                    for (auto& conn : connections)
                    {
                        if (conn == pParam->m_hConn)
                        {
                            conn = k_HSteamNetConnection_Invalid;
                            break;
                        }
                    }
                    if (callback)
                        callback(MultiplayerEvent::HOST_CLIENT_CLOSED_CONNECTION);
                    LOG_INFO("Client disconnected: %s", pParam->m_info.m_szEndDebug);
                }
                else // If you're a client and the host disconnects
                {
                    LOG_INFO("Disconnected from the host: %s", pParam->m_info.m_szEndDebug);
                    goOffline();
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
                    int i = 0;
                    for (auto& conn : connections)
                    {
                        if (conn == pParam->m_hConn)
                            conn = k_HSteamNetConnection_Invalid;
                        else if (conn == k_HSteamNetConnection_Invalid)
                            ++i;
                    }
                    if (i == 0) // Go offline if no more clients are connected
                    {
                        goOffline();
                        LOG_INFO("(Host) Local problem with connection. Closed session: %s", errStr);
                    }
                    else
                    {
                        LOG_INFO("(Host) Local problem with connection. Disconnected client from session: %s", errStr);
                    }
                }
                else
                {
                    goOffline();
                    LOG_INFO("(Client) Local problem with connection. Closed session: %s", errStr);
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