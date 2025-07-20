// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MULTIPLAYER_DATA_H
#define MAGIQUE_MULTIPLAYER_DATA_H

#include <magique/util/Logging.h>
#include <magique/multiplayer/Multiplayer.h>
#include <magique/multiplayer/Lobby.h>
#include <magique/internal/Macros.h>

#include "internal/datastructures/VectorType.h"
#include "internal/utils/STLUtil.h"
#include "internal/datastructures/StringHashMap.h"

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

    enum class LobbyPacketType : char
    {
        CHAT,
        METADATA,
        START_SIGNAL
    };

    struct LobbyData final
    {
        LobbyChatCallback chatCallback;         // callback for chat
        LobbyMetadataCallback metadataCallback; // Callback for metadata
        StringHashMap<std::string> metadata;    // meta data map
        bool startSignal = false;

        void handleLobbyPacket(const Message& msg)
        {
            const auto type = LobbyPacketType{((const int8_t*)msg.payload.data)[0]};
            const auto data = ((const char*)msg.payload.data) + 1;

            if (type == LobbyPacketType::CHAT)
            {
                MAGIQUE_ASSERT(strlen(data) < MAGIQUE_MAX_LOBBY_MESSAGE_LEN, "Missing null terminator");
                if (chatCallback)
                {
                    chatCallback(msg.connection, data);
                }
            }
            else if (type == LobbyPacketType::METADATA)
            {
                auto key = data;
                auto value = data + strlen(key) + 1;
                if (metadataCallback)
                {
                    metadataCallback(msg.connection, key, value);
                }
                if (GetIsClient())
                {
                    metadata[key] = value;
                }
            }
            else if (type == LobbyPacketType::START_SIGNAL)
            {
                startSignal = (bool)data[1] == true;
            }
            else
            {
                LOG_WARNING("MessageType=255 is reserved for lobby packet!");
            }
        }

        void closeLobby()
        {
            startSignal = false;
            metadata.clear();
        }
    };

    namespace global
    {
        inline LobbyData LOBBY_DATA{};
    } // namespace global

    struct ConnMapping final
    {
        Connection conn;
        entt::entity entity;

        static bool DeleteFunc(const ConnMapping& m1, const ConnMapping& m2) { return m1.conn == m2.conn; }
    };

    struct ConnNumberMapping final
    {
        std::array<Connection, MAGIQUE_MAX_PLAYERS - 1> conns{}; // Not invalid

        void addConnection(Connection conn)
        {
            for (auto& savedConn : conns)
            {
                if (savedConn == Connection::INVALID_CONNECTION)
                {
                    savedConn = conn;
                    return;
                }
            }
            LOG_ERROR("Too many connections");
        }

        void removeConnection(Connection conn)
        {
            for (auto& savedConn : conns)
            {
                if (savedConn == conn)
                {
                    savedConn = Connection::INVALID_CONNECTION;
                    return;
                }
            }
            LOG_ERROR("Connection could not be removed?!");
        }

        [[nodiscard]] int getNum(Connection conn) const
        {
            for (int i = 0; i < MAGIQUE_MAX_PLAYERS - 1; ++i)
            {
                if (conns[i] == conn)
                {
                    return i;
                }
            }
            return -1;
        }

        void clear()
        {
            for (auto& conn : conns)
            {
                conn = Connection::INVALID_CONNECTION;
            }
        }
    };

    struct MultiplayerData final
    {
        MultiplayerCallback callback;                                   // Callback
        std::vector<Connection> connections;                            // Holds all current valid connections
        std::vector<ConnMapping> connectionMapping;                     // Holds all the manually set mappings
        std::vector<Message> incMsgVec;                                 // Incoming magique::Messages
        vector<SteamNetworkingMessage_t*> outMsgBuffer;                 // Outgoing message buffer
        vector<SteamNetworkingMessage_t*> incMsgBuffer;                 // Incoming message buffer
        ConnNumberMapping numberMapping{};                              // Maps connection to a consistent number
        HSteamListenSocket listenSocket = k_HSteamListenSocket_Invalid; // The global listen socket
        bool isHost = false;                                            // If the program is host or client
        bool isInSession = false;                                       // If program is part of multiplayer activity
        bool isInitialized = false;                                     // Manual flag to give clean error msg

        MultiplayerData()
        {
            incMsgVec.reserve(MAGIQUE_ESTIMATED_MESSAGES);
            outMsgBuffer.reserve(MAGIQUE_ESTIMATED_MESSAGES);
            incMsgBuffer.reserve(MAGIQUE_ESTIMATED_MESSAGES);
            connections.reserve(MAGIQUE_MAX_PLAYERS + 1);
            connectionMapping.reserve(MAGIQUE_MAX_PLAYERS + 1);
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

        void goOnline(const bool asHost, Connection conn = Connection::INVALID_CONNECTION)
        {
            if (asHost) // For client deferred until host actually accepts
            {
                isInSession = true;
            }
            else if (conn != Connection::INVALID_CONNECTION)
            {
                connections.push_back(conn);
                numberMapping.addConnection(conn);
            }
            isHost = asHost;
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
                msg->Release();
            }
            incMsgVec.clear();
            incMsgBuffer.clear();
            connections.clear();
            connectionMapping.clear();
            isHost = false;
            isInSession = false;
            numberMapping.clear();
            global::LOBBY_DATA.closeLobby();
        }

        void update() const
        {
#ifndef MAGIQUE_STEAM
                SteamNetworkingSockets()->RunCallbacks();
#endif
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
                if ((pParam->m_eOldState == k_ESteamNetworkingConnectionState_Connecting ||
                     pParam->m_eOldState == k_ESteamNetworkingConnectionState_FindingRoute) &&
                    pParam->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected)
                {
                    isInSession = true; // Deferred until here - could be just connection build up
                    LOG_INFO("A connection initiated by us was accepted by the remote host.");
                    if (callback)
                    {
                        callback(MultiplayerEvent::CLIENT_CONNECTION_ACCEPTED, conn);
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
                    UnorderedDelete(connections, conn);
                    if (callback)
                    {
                        callback(MultiplayerEvent::HOST_CLIENT_DISCONNECTED, conn);
                    }
                    numberMapping.removeConnection(conn);
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
                    UnorderedDelete(connections, conn);
                    if (callback)
                    {
                        callback(MultiplayerEvent::HOST_LOCAL_PROBLEM, conn);
                    }
                    numberMapping.removeConnection(conn);
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
    } // namespace global

    inline void OnConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t* pParam)
    {
        global::MP_DATA.onConnectionStatusChange(pParam);
    }
} // namespace magique

#endif //MAGIQUE_MULTIPLAYER_DATA_H