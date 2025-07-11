// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <magique/internal/Macros.h>
#include <magique/multiplayer/Multiplayer.h>
#include <entt/entity/entity.hpp>

#include "internal/globals/MultiplayerData.h"
#include "internal/globals/EngineConfig.h"

namespace magique
{
    void EnterClientMode() { global::ENGINE_CONFIG.isClientMode = true; }

    void ExitClientMode() { global::ENGINE_CONFIG.isClientMode = false; }

    bool GetIsClientMode() { return global::ENGINE_CONFIG.isClientMode; }

    Payload CreatePayload(const void* data, const int size, const MessageType type) { return Payload{data, size, type}; }

    bool BatchMessage(const Connection conn, const Payload payload, const SendFlag flag)
    {
        MAGIQUE_ASSERT(conn != Connection::INVALID_CONNECTION && payload.size >= 0 &&
                           (flag == SendFlag::UN_RELIABLE || flag == SendFlag::RELIABLE),
                       "Passed invalid input parameters");

        if (payload.data == nullptr) [[unlikely]] // This can cause runtime crash
        {
            LOG_WARNING("Invalid payload data");
            return false;
        }

        // Allocate with buffer - +1 for the type
        const auto msg = SteamNetworkingUtils()->AllocateMessage(payload.size + 1);
        std::memcpy(static_cast<char*>(msg->m_pData) + 1, payload.data, payload.size);
        static_cast<char*>(msg->m_pData)[0] = static_cast<char>(payload.type);

        msg->m_nFlags = static_cast<int>(flag);
        msg->m_conn = static_cast<HSteamNetConnection>(conn);
        global::MP_DATA.batchedMsgs.push_back(msg);
        return true;
    }

    void BatchMessageToAll(const Payload payload, const SendFlag flag)
    {
        for (const auto conn : GetCurrentConnections())
        {
            BatchMessage(conn, payload, flag);
        }
    }

    bool SendBatch()
    {
        auto& data = global::MP_DATA;
        if (data.batchedMsgs.empty()) [[unlikely]]
            return false;
        const auto size = data.batchedMsgs.size();
        SteamNetworkingSockets()->SendMessages(size, data.batchedMsgs.data(), nullptr);
        data.batchedMsgs.clear();
        return true;
    }

    static char MESSAGE_BUFFER[1500]{}; // To insert the message type before the actual data

    bool SendMessage(Connection conn, Payload payload, SendFlag flag)
    {
        MAGIQUE_ASSERT((int)conn != k_HSteamNetConnection_Invalid, "Invalid connection");
        MAGIQUE_ASSERT(flag == SendFlag::RELIABLE || flag == SendFlag::UN_RELIABLE, "Invalid flag");

        if (payload.data == nullptr) [[unlikely]] // This can cause runtime crash
        {
            LOG_WARNING("Invalid payload data");
            return false;
        }

        const int totalSize = payload.size + 1;
        char* buffer = MESSAGE_BUFFER;
        if ((int)sizeof(MESSAGE_BUFFER) < totalSize)
        {
            buffer = new char[totalSize];
        }

        std::memcpy(buffer + 1, payload.data, payload.size);
        buffer[0] = (char)payload.type;

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

    const std::vector<Message>& ReceiveIncomingMessages(const int max)
    {
        auto& data = global::MP_DATA;

        if (data.buffSize != 0) [[likely]]
        {
            for (int i = 0; i < data.buffSize; ++i)
            {
                data.msgBuffer[i]->Release();
            }
            data.buffSize = 0;
            data.msgVec.clear();
        }

        if (!data.isInSession) [[unlikely]]
        {
            return data.msgVec;
        }

        auto ensureCapacity = [&](const int newCap)
        {
            if (newCap > data.buffCap) [[unlikely]]
            {
                auto* newBuff = new SteamNetworkingMessage_t*[data.buffCap * 2];
                memcpy(newBuff, data.msgBuffer, data.buffSize * sizeof(SteamNetworkingMessage_t*));
                delete[] data.msgBuffer;
                data.msgBuffer = newBuff;
                data.buffCap = data.buffCap * 2;
            }
        };

        // Lambda to process received messages
        auto processMessages = [&](const int startIdx, const int count)
        {
            for (int i = startIdx; i < startIdx + count; ++i)
            {
                const auto* msg = data.msgBuffer[i];
                Message message;
                message.payload.data = static_cast<uint8_t*>(msg->m_pData) + 1;
                message.payload.size = msg->m_cbSize - 1;
                message.payload.type = static_cast<MessageType>(*(uint8_t*)msg->m_pData); // Interpret first byte as type
                message.connection = static_cast<Connection>(msg->m_conn);
                message.timeStamp = msg->m_usecTimeReceived;
                data.msgVec.push_back(message);
            }
        };

        if (data.isHost) // Receiving messages for host
        {
            for (const auto conn : data.connections)
            {
                ensureCapacity(data.buffSize + max);
                const auto steamConn = static_cast<HSteamNetConnection>(conn);
                auto* msgBuffer = data.msgBuffer + data.buffSize;
                const auto n = SteamNetworkingSockets()->ReceiveMessagesOnConnection(steamConn, msgBuffer, max);
                if (n == -1) // No more messages
                    continue;
                processMessages(data.buffSize, n);
                data.buffSize += n;
            }
        }
        else // Receiving messages for non-host
        {
            ensureCapacity(max);
            const auto conn = data.connections[0];
            const auto steamConn = static_cast<HSteamNetConnection>(conn);
            const auto n = SteamNetworkingSockets()->ReceiveMessagesOnConnection(steamConn, data.msgBuffer, max);
            if (n == -1)
            {
                return data.msgVec;
            }
            data.buffSize = n;
            processMessages(0, n);
        }
        return data.msgVec;
    }

    const std::vector<Connection>& GetCurrentConnections() { return global::MP_DATA.connections; }

    //----------------- UTIL -----------------//

    void SetMultiplayerCallback(const MultiplayerCallback& func) { global::MP_DATA.callback = func; }

    bool GetInMultiplayerSession() { return global::MP_DATA.isInSession; }

    bool GetIsHost() { return global::MP_DATA.isInSession && global::MP_DATA.isHost; }

    bool GetIsActiveHost() { return GetIsHost() && !global::MP_DATA.connections.empty(); }

    bool GetIsClient() { return global::MP_DATA.isInSession && global::MP_DATA.isHost == false; }

    void SetConnectionEntityMapping(Connection conn, entt::entity entity)
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
        if (data.connectionMapping.size() >= MAGIQUE_MAX_PLAYERS)
        {
            LOG_WARNING("Too many mapped connections. Limit %d", MAGIQUE_MAX_PLAYERS);
            return;
        }
        data.connectionMapping.push_back(ConnMapping{conn, entity});
    }

    entt::entity GetConnectionEntityMapping(Connection conn)
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

    bool GetSteamLoaded()
    {
#ifdef MAGIQUE_STEAM
        return true;
#else
        return false;
#endif
    }

    bool GetNetworkingSocketsLoaded()
    {
#ifdef MAGIQUE_LAN
        return true;
#else
        return false;
#endif
    }


} // namespace magique