// SPDX-License-Identifier: zlib-acknowledgement

#include <vector>
#include <magique/core/Types.h>
#include <entt/entity/entity.hpp>
#include <magique/multiplayer/Networking.h>
#if defined(MAGIQUE_STEAM) || defined(MAGIQUE_LAN)
#define _CRT_SECURE_NO_WARNINGS

#include "internal/globals/NetworkingData.h"
#include "internal/globals/EngineConfig.h"
#include "headers/MultiplayerStatistics.h"

namespace magique
{
    bool NetworkSend(Connection conn, Payload payload, SendFlag flag)
    {
        MAGIQUE_ASSERT(static_cast<int>(conn) != k_HSteamNetConnection_Invalid, "Invalid connection");
        MAGIQUE_ASSERT(flag <= SendFlag::RELIABLE, "Invalid flag");

        if (payload.data == nullptr || !NetworkInSession()) [[unlikely]] // This can cause runtime crash
        {
            return false;
        }

        auto* msg = SteamNetworkingUtils()->AllocateMessage(payload.size + 1);

        // Reserved message type
        static_cast<char*>(msg->m_pData)[0] = static_cast<char>(payload.type);
        std::memcpy(static_cast<char*>(msg->m_pData) + 1, payload.data, payload.size);

        // set the flags
        msg->m_nFlags = static_cast<int>(flag);
        msg->m_conn = static_cast<HSteamNetConnection>(conn);

        global::MP_DATA.statistics.addOutgoing(payload);
        SteamNetworkingSockets()->SendMessages(1, &msg, nullptr);
        return true;
    }

    bool NetworkSendAll(const Payload payload, const SendFlag flag)
    {
        bool success = true;
        for (const auto conn : NetworkGetConnections())
        {
            success &= NetworkSend(conn, payload, flag);
        }
        return success;
    }

    bool NetworkSendHost(Payload payload, SendFlag flag)
    {
        return NetworkSend(NetworkGetConnections().front(), payload, flag);
    }

    void NetworkFlush()
    {
        for (const auto conn : NetworkGetConnections())
        {
            SteamNetworkingSockets()->FlushMessagesOnConnection(static_cast<int>(conn));
        }
    }

    const std::vector<Message>& NetworkReceive(const int max)
    {
        auto& data = global::MP_DATA;
        if (!data.incMsgBuffer.empty()) [[likely]]
        {
            for (auto* msg : data.incMsgBuffer)
            {
                if (msg != nullptr)
                    msg->Release();
            }
            data.incMsgBuffer.clear();
            data.incMsgVec.clear();
        }

        if (!NetworkInSession()) [[unlikely]]
        {
            return data.incMsgVec;
        }

        // Lambda to process received messages
        auto processMessages = [&](const int startIdx, const int count)
        {
            for (int i = startIdx; i < startIdx + count; ++i)
            {
                const auto* msg = data.incMsgBuffer[i];
                Message message{};
                message.payload.data = static_cast<uint8_t*>(msg->m_pData) + 1;
                message.payload.size = msg->m_cbSize - 1;
                message.payload.type = static_cast<MessageType>(((uint8_t*)msg->m_pData)[0]); // Interpret first byte
                message.connection = static_cast<Connection>(msg->m_conn);
                message.timeStamp = msg->m_usecTimeReceived;

                if (message.payload.type == MAGIQUE_LOBBY_PACKET_TYPE)
                {
                    global::MP_DATA.lobby.handleLobbyPacket(message);
                }
                else
                {
                    data.incMsgVec.push_back(message);
#ifdef MAGIQUE_DEBUG
                    global::MP_DATA.statistics.addIncoming(message.payload);
#endif
                }
            }
        };

        MAGIQUE_ASSERT(data.incMsgBuffer.empty(), "Must be empty by now");
        MAGIQUE_ASSERT(data.incMsgVec.empty(), "Must be empty by now");

        int receivedMsgs = 0;
        data.incMsgBuffer.resize((int)data.connections.size() * max);
        for (const auto conn : data.connections)
        {
            const auto steamConn = static_cast<HSteamNetConnection>(conn);
            auto** buff = data.incMsgBuffer.data() + receivedMsgs;
            const auto n = SteamNetworkingSockets()->ReceiveMessagesOnConnection(steamConn, buff, max);
            if (n == -1 || n == 0) // No more messages
            {
                continue;
            }
            processMessages(receivedMsgs, n);
            receivedMsgs += n;
        }
        return data.incMsgVec;
    }

    const std::vector<Connection>& NetworkGetConnections() { return global::MP_DATA.connections; }

    //----------------- UTIL -----------------//

    void NetworkSetCallback(const NetworkCallback& func) { global::MP_DATA.callback = func; }

    bool NetworkInSession() { return global::MP_DATA.inSession; }

    bool NetworkIsHost() { return global::MP_DATA.inSession && global::MP_DATA.isHost; }

    bool NetworkIsClient() { return global::MP_DATA.inSession && !global::MP_DATA.isHost; }

    void NetworkSetConnMapping(const Connection conn, const entt::entity entity)
    {
        auto& data = global::MP_DATA;
        if (!NetworkInSession())
        {
            LOG_WARNING("Cant set mapping - not in multiplayer session");
            return;
        }
        for (auto& mapping : data.connectionMapping)
        {
            if (mapping.conn == conn)
            {
                mapping.entity = entity;
                return;
            }
        }
        if (data.connectionMapping.size() >= MAGIQUE_MAX_PLAYERS - 1)
        {
            LOG_WARNING("Too many mapped connections. Limit %d", MAGIQUE_MAX_PLAYERS - 1);
            return;
        }
        data.connectionMapping.push_back(ConnMapping{conn, entity});
    }

    entt::entity NetworkGetConnMapping(const Connection conn)
    {
        auto& data = global::MP_DATA;
        for (auto& mapping : data.connectionMapping)
        {
            if (mapping.conn == conn)
            {
                return mapping.entity;
            }
        }
        return entt::null;
    }

    Connection NetworkGetConnMapping(const entt::entity entity)
    {
        auto& data = global::MP_DATA;
        for (auto& mapping : data.connectionMapping)
        {
            if (mapping.entity == entity)
            {
                return mapping.conn;
            }
        }
        return Connection::INVALID;
    }

    int NetworkGetConnNumber(const Connection conn) { return global::MP_DATA.numberMapping.getNum(conn); }

    void NetworkEnterClientMode() { global::ENGINE_CONFIG.isClientMode = true; }

    void NetworkExitClientMode() { global::ENGINE_CONFIG.isClientMode = false; }

    bool NetworkIsClientMode() { return global::ENGINE_CONFIG.isClientMode; }

    internal::MultiplayerStatsData internal::getStats() { return global::MP_DATA.statistics.getStats(); }

} // namespace magique

#else
namespace magique
{
    bool NetworkSend(Connection conn, Payload payload, SendFlag flag)
    {
        (void)conn;
        (void)payload;
        (void)flag;
        return false;
    }

    bool NetworkSendAll(const Payload payload, const SendFlag flag)
    {
        (void)payload;
        (void)flag;
        return false;
    }

    bool NetworkSendHost(Payload payload, SendFlag flag)
    {
        (void)payload;
        (void)flag;
        return false;
    }

    void NetworkFlush() {}

    const std::vector<Message>& NetworkReceive(const int max)
    {
        (void)max;
        static std::vector<Message> empty;
        return empty;
    }

    const std::vector<Connection>& GetCurrentConnections()
    {
        static std::vector<Connection> empty;
        return empty;
    }

    void NetworkSetCallback(const NetworkCallback& func) { (void)func; }

    bool NetworkInSession() { return false; }

    bool NetworkIsHost() { return false; }

    bool NetworkIsClient() { return false; }

    void NetworkSetConnMapping(const Connection conn, const entt::entity entity)
    {
        (void)conn;
        (void)entity;
    }

    entt::entity NetworkGetConnMapping(const Connection conn)
    {
        (void)conn;
        return entt::null;
    }

    Connection NetworkGetConnMapping(const entt::entity entity)
    {
        (void)entity;
        return Connection::INVALID;
    }

    int NetworkGetConnNumber(const Connection conn)
    {
        (void)conn;
        return -1;
    }

    void NetworkEnterClientMode() {}

    void NetworkExitClientMode() {}

    bool NetworkIsClientMode() { return false; }

    internal::MultiplayerStatsData internal::getStats() { return {}; }

} // namespace magique
#endif
