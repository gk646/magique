// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <magique/internal/Macros.h>
#include <magique/multiplayer/Multiplayer.h>
#include <entt/entity/entity.hpp>

#include "internal/globals/MultiplayerData.h"
#include "internal/globals/EngineConfig.h"

namespace magique
{


    Payload CreatePayload(const void* data, const int size, const MessageType type) { return Payload{data, size, type}; }

    void BatchMessage(const Connection conn, const Payload payload, const SendFlag flag)
    {
        MAGIQUE_ASSERT(conn != Connection::INVALID_CONNECTION, "Passed invalid input parameters");
        MAGIQUE_ASSERT(payload.size > 0, "Passed invalid input parameters");
        MAGIQUE_ASSERT(flag == SendFlag::UNRELIABLE || flag == SendFlag::RELIABLE, "Passed invalid input parameters");

        // This can cause runtime crash
        if (payload.data == nullptr || payload.size > BatchBuffer::BUFF_SIZE) [[unlikely]]
        {
            LOG_WARNING("Invalid payload data");
            return;
        }
        if (!GetInMultiplayerSession()) [[unlikely]]
        {
            return;
        }
        auto& data = global::MP_DATA;
        global::BATCHER.batchMessage(data.outMsgBuffer, conn, payload.data, payload.type, payload.size, flag);
    }

    void BatchMessageToAll(const Payload payload, const SendFlag flag)
    {
        for (const auto conn : GetCurrentConnections())
        {
            BatchMessage(conn, payload, flag);
        }
    }

    void SendBatch()
    {
        auto& data = global::MP_DATA;
        global::BATCHER.sendAll(data.outMsgBuffer);
        if (data.outMsgBuffer.empty())
        {
            [[unlikely]] return;
        }
        const auto size = data.outMsgBuffer.size();
        SteamNetworkingSockets()->SendMessages(size, data.outMsgBuffer.data(), nullptr);
        data.outMsgBuffer.clear();
    }

    static char MESSAGE_BUFFER[1500]{}; // To insert the message type before the actual data

    bool SendMessage(Connection conn, Payload payload, SendFlag flag)
    {
        MAGIQUE_ASSERT((int)conn != k_HSteamNetConnection_Invalid, "Invalid connection");
        MAGIQUE_ASSERT(flag <= SendFlag::RELIABLE, "Invalid flag");

        if (payload.data == nullptr) [[unlikely]] // This can cause runtime crash
        {
            LOG_WARNING("Invalid payload data");
            return false;
        }
        if (!GetInMultiplayerSession()) [[unlikely]]
        {
            return false;
        }
        const int totalSize = payload.size + 1;
        char* buffer = MESSAGE_BUFFER;
        if ((int)sizeof(MESSAGE_BUFFER) < totalSize)
        {
            buffer = new char[totalSize];
        }
        buffer[0] = (char)payload.type;
        std::memcpy(buffer + 1, payload.data, payload.size);

        const auto res =
            SteamNetworkingSockets()->SendMessageToConnection((int)conn, buffer, totalSize, (int)flag, nullptr);

        if (buffer != MESSAGE_BUFFER)
            delete[] buffer;

        return res == k_EResultOK;
    }

    void SendMessageToAll(const Payload payload, const SendFlag flag)
    {
        for (const auto conn : GetCurrentConnections())
        {
            SendMessage(conn, payload, flag);
        }
    }

    void FlushMessages()
    {
        for (const auto conn : GetCurrentConnections())
        {
            SteamNetworkingSockets()->FlushMessagesOnConnection((int)conn);
        }
    }

    const std::vector<Message>& ReceiveIncomingMessages(const int max)
    {
        auto& data = global::MP_DATA;
        auto& lobby = global::LOBBY_DATA;

        if (!data.incMsgBuffer.empty()) [[likely]]
        {
            for (const auto msg : data.incMsgBuffer)
            {
                msg->Release();
            }
            data.incMsgBuffer.clear();
            data.incMsgVec.clear();
        }

        if (!GetInMultiplayerSession()) [[unlikely]]
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

                // Batched message
                if (message.payload.type == MessageType{UINT8_MAX - 1}) [[likely]]
                {
                    global::BATCHER.handleBatchedPacket(data.incMsgVec, message);
                    continue;
                }
                // Lobby message handling - reserved message type
                else if (message.payload.type == MessageType{UINT8_MAX}) [[unlikely]]
                {
                    lobby.handleLobbyPacket(message);
                    continue;
                }
                data.incMsgVec.push_back(message);
            }
        };

        MAGIQUE_ASSERT(data.incMsgBuffer.empty(), "Must be empty by now");
        MAGIQUE_ASSERT(data.incMsgVec.empty(), "Must be empty by now");

        data.incMsgBuffer.reserve((int)data.connections.size() * max);
        for (const auto conn : data.connections)
        {
            const auto steamConn = static_cast<HSteamNetConnection>(conn);
            auto** buff = data.incMsgBuffer.data() + data.incMsgBuffer.size();
            const auto n = SteamNetworkingSockets()->ReceiveMessagesOnConnection(steamConn, buff, max);
            if (n == -1) // No more messages
            {
                continue;
            }
            auto start = data.incMsgBuffer.size();
            data.incMsgBuffer.set_size(data.incMsgBuffer.size() + n);
            processMessages(start, n);
        }
        return data.incMsgVec;
    }

    const std::vector<Connection>& GetCurrentConnections() { return global::MP_DATA.connections; }

    //----------------- UTIL -----------------//

    void SetMultiplayerCallback(const MultiplayerCallback& func) { global::MP_DATA.callback = func; }

    bool GetInMultiplayerSession() { return global::MP_DATA.isInSession; }

    bool GetIsHost() { return global::MP_DATA.isInSession && global::MP_DATA.isHost; }

    bool GetIsActiveHost() { return GetIsHost() && !global::MP_DATA.connections.empty(); }

    bool GetIsClient() { return global::MP_DATA.isInSession && global::MP_DATA.isHost == false; }

    void SetConnectionEntityMapping(const Connection conn, const entt::entity entity)
    {
        auto& data = global::MP_DATA;
        if (!GetInMultiplayerSession())
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

    entt::entity GetConnectionEntityMapping(const Connection conn)
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

    Connection GetConnectionEntityMapping(const entt::entity entity)
    {
        auto& data = global::MP_DATA;
        for (auto& mapping : data.connectionMapping)
        {
            if (mapping.entity == entity)
            {
                return mapping.conn;
            }
        }
        return Connection::INVALID_CONNECTION;
    }

    int GetConnectionNum(const Connection conn) { return global::MP_DATA.numberMapping.getNum(conn); }

    void EnterClientMode() { global::ENGINE_CONFIG.isClientMode = true; }

    void ExitClientMode() { global::ENGINE_CONFIG.isClientMode = false; }

    bool GetIsClientMode() { return global::ENGINE_CONFIG.isClientMode; }

} // namespace magique