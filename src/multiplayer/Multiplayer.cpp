#include <magique/internal/Macros.h>
#include <magique/multiplayer/Multiplayer.h>

#include "internal/globals/MultiplayerData.h"

namespace magique
{

    Payload CreatePayload(const void* data, const int size, const MessageType type)
    {
        return {data, size, type};
    }

    bool BatchMessage(const Connection conn, const Payload payload, const SendFlag flag)
    {
        MAGIQUE_ASSERT(conn == Connection::INVALID_CONNECTION || payload.data == nullptr || payload.size == 0 ||
                           (flag != SendFlag::UN_RELIABLE && flag != SendFlag::RELIABLE),
                       "Passed invalid input parameters");

        if (payload.data == nullptr) [[unlikely]] // This can cause runtime crash
        {
            LOG_WARNING("Invalid payload data");
            return false;
        }

        // Allocate with buffer - +1 for the type
        const auto msg = SteamNetworkingUtils()->AllocateMessage(payload.size + 1);
        std::memcpy((char*)msg->m_pData + 1, payload.data, payload.size);
        ((char*)msg->m_pData)[0] = (char)payload.type;

        msg->m_nFlags = static_cast<int>(flag);
        msg->m_conn = static_cast<HSteamNetConnection>(conn);
        global::MP_DATA.batchedMsgs.push_back(msg);
        return true;
    }

    bool SendBatch()
    {
        auto& data = global::MP_DATA;
        if (data.batchedMsgs.empty())
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
        if (sizeof(MESSAGE_BUFFER) < totalSize)
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

    const std::vector<Message>& ReceiveMessages(const int max)
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
                std::memcpy(newBuff, data.msgBuffer, data.buffSize * sizeof(SteamNetworkingMessage_t*));
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
                if (conn != k_HSteamNetConnection_Invalid)
                {
                    ensureCapacity(data.buffSize + max);
                    const auto n =
                        SteamNetworkingSockets()->ReceiveMessagesOnConnection(conn, data.msgBuffer + data.buffSize, max);
                    if (n == -1)
                        continue;
                    processMessages(data.buffSize, n);
                    data.buffSize += n;
                }
            }
        }
        else // Receiving messages for non-host
        {
            ensureCapacity(max);
            const auto conn = data.connections[0];
            const auto n = SteamNetworkingSockets()->ReceiveMessagesOnConnection(conn, data.msgBuffer, max);
            if (n == -1)
            {
                return data.msgVec;
            }
            data.buffSize = n;
            processMessages(0, n);
        }
        return data.msgVec;
    }

    //----------------- UTIL -----------------//

    void SetMultiplayerCallback(const std::function<void(MultiplayerEvent event)>& func)
    {
        global::MP_DATA.callback = func;
    }

    bool IsInMultiplayerSession() { return global::MP_DATA.isInSession; }

    bool IsHost() { return global::MP_DATA.isInSession && global::MP_DATA.isHost; }

    bool IsClient() { return global::MP_DATA.isInSession && global::MP_DATA.isHost == false; }

} // namespace magique