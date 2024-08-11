#ifndef MULTIPLAYERDATA_H
#define MULTIPLAYERDATA_H

#include <functional>

#include "external/networkingsockets/steamnetworkingsockets.h"
#include "external/networkingsockets/isteamnetworkingutils.h"

#if MAGIQUE_STEAM == 0
#include <steam/steam_api.h>
#endif


namespace magique
{
    using MessageBuffer = cxstructs::SmallVector<SteamNetworkingMessage_t*, MAGIQUE_MESSAGES_ESTIMATE>;

    struct MultiplayerData final
    {
        STEAM_CALLBACK(MultiplayerData, onConnectionStatusChange, SteamNetConnectionStatusChangedCallback_t);

        HSteamListenSocket listenSocket = k_HSteamListenSocket_Invalid; // The global listen socket
        bool isHost = false;                                            // If the program is host or client
        bool isOnline = false;                                          // If program is part of multiplayer activity
        HSteamNetConnection connections[MAGIQUE_MAX_PLAYERS]{};         // All possible outgoing connections as host
        std::function<void(Connection)> onNewConnection;                // Callback
        MessageBuffer messages;                                         // Message buffer

        void clearState()
        {
            std::memset(connections, 0, sizeof(int) * MAGIQUE_MAX_PLAYERS);
            messages.clear();
            isHost = false;
            isOnline = false;
            listenSocket = k_HSteamListenSocket_Invalid;
        }
    };

    inline void MultiplayerData::onConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t* pParam)
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
                            if (onNewConnection)
                                onNewConnection(static_cast<Connection>(pParam->m_hConn));
                            break;
                        }
                    }
                    if (!accepted)
                        LOG_WARNING("Trying to accept more connections than configured: %d", MAGIQUE_MAX_PLAYERS);
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
                LOG_INFO("Client disconnected: %s", pParam->m_info.m_szEndDebug);
            }
            else // If you're a client and the host disconnects
            {
                LOG_INFO("Disconnected from the host: %s", pParam->m_info.m_szEndDebug);
                clearState();
            }
        }

        // Connection closed locally due to a problem
        if ((pParam->m_eOldState == k_ESteamNetworkingConnectionState_Connecting ||
             pParam->m_eOldState == k_ESteamNetworkingConnectionState_Connected) &&
            pParam->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
        {
            LOG_INFO("Local problem with  connection: %s", pParam->m_info.m_szEndDebug);

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
            }
            else
            {
                clearState();
            }
        }
    }

    namespace global
    {
        inline MultiplayerData MP_DATA{};
    }
} // namespace magique

#endif //MULTIPLAYERDATA_H