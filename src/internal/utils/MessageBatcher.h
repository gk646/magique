#ifndef BATCHUTIL_H
#define BATCHUTIL_H

// TODO Probably remove as it does it internally
namespace magique
{
    struct BatchBuffer final
    {
        static constexpr int BUFF_SIZE = 1024;
        char buffer[BUFF_SIZE]{};
        int size = 0;

        [[nodiscard]] bool canHold(const int packet) const
        {
            return size + packet + 3 < BUFF_SIZE; // packet, type and size
        }

        void clear() { size = 0; }

        void batch(const void* data, MessageType type, const int dsize)
        {
            auto msgSize = (uint16_t)dsize;
            // The type
            buffer[size] = (char)type;
            // The size
            std::memcpy(buffer + size + 1, &msgSize, 2);
            // the payload
            std::memcpy(buffer + size + 3, data, dsize);
            size += dsize + 3;
        }
    };

    struct ConnectionBuffer final
    {
        BatchBuffer reliable;
        BatchBuffer unreliable;
    };

    struct MessageBatcher final
    {
        std::array<ConnectionBuffer, MAGIQUE_MAX_PLAYERS - 1> buffers{};

        void batchMessage(MessageVec& vec, const Connection conn, const void* data, MessageType type, const int size,
                          const SendFlag flag)
        {
            auto& connBuff = getConBuffer(conn);
            BatchBuffer* buffer = flag == SendFlag::RELIABLE ? &connBuff.reliable : &connBuff.unreliable;
            if (!buffer->canHold(size))
            {
                sendConBuffer(vec, connBuff, conn);
            }
            buffer->batch(data, type, size);
        }

        void sendConBuffer(MessageVec& vec, ConnectionBuffer& buffer, Connection conn)
        {
            sendBatch(vec, buffer.reliable, conn, SendFlag::RELIABLE);
            sendBatch(vec, buffer.unreliable, conn, SendFlag::UNRELIABLE);
        }

        void clearConBuffer(const Connection conn)
        {
            auto& buff = getConBuffer(conn);
            buff.reliable.clear();
            buff.unreliable.clear();
        }

        void clear()
        {
            for (auto& buffer : buffers)
            {
                buffer.reliable.clear();
                buffer.unreliable.clear();
            }
        }

        void sendAll(MessageVec& vec)
        {
            for (const auto conn : GetCurrentConnections())
            {
                sendConBuffer(vec, getConBuffer(conn), conn);
            }
        }

        void handleBatchedPacket(std::vector<Message>& vec, const Message& message)
        {
            MAGIQUE_ASSERT(message.payload.size < BatchBuffer::BUFF_SIZE, "Invalid batched packet");
            // already skipped the type
            const auto* data = static_cast<const unsigned char*>(message.payload.data);
            int off = 0;
            while (off < message.payload.size)
            {
                // Layout (what, bytes) is: type 1: size: 2: payload: size
                auto type = MessageType{data[off]};
                off += 1;
                uint16_t size = 0;
                std::memcpy(&size, &data[off], 2);
                off += 2;

                Message batchedMsg{};
                batchedMsg.connection = message.connection;
                batchedMsg.timeStamp = message.timeStamp;
                batchedMsg.payload.size = size;
                batchedMsg.payload.data = data + off;
                batchedMsg.payload.type = type;

                if (type == MAGIQUE_LOBBY_PACKET_TYPE)
                {
                    global::LOBBY_DATA.handleLobbyPacket(batchedMsg);
                }
                else
                {
                    vec.push_back(batchedMsg);
                }
                off += size;
            }

            if (off != message.payload.size)
            {
                LOG_WARNING("Batched message size mismatch");
            }
        }


    private:
        ConnectionBuffer& getConBuffer(const Connection conn) { return buffers[GetConnectionNum(conn)]; }

        void sendBatch(MessageVec& outMsgBuf, BatchBuffer& buffer, Connection conn, SendFlag flag)
        {
            if (buffer.size == 0)
                return;

            const auto msg = SteamNetworkingUtils()->AllocateMessage(buffer.size + 1);
            // Reserved message type
            static_cast<char*>(msg->m_pData)[0] = static_cast<char>(MAGIQUE_BATCHED_PACKET_TYPE);
            std::memcpy(static_cast<char*>(msg->m_pData) + 1, buffer.buffer, buffer.size);

            // set the flags
            msg->m_nFlags = static_cast<int>(flag);
            msg->m_conn = static_cast<HSteamNetConnection>(conn);

            // sent later
            outMsgBuf.push_back(msg);
            buffer.clear();
        }
    };

    namespace global
    {
        inline MessageBatcher BATCHER{};
    }


} // namespace magique

#endif //BATCHUTIL_H
