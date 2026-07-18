#ifndef MAGEQUEST_LOBBYDATA_H
#define MAGEQUEST_LOBBYDATA_H

#include <magique/util/Datastructures.h>

namespace magique
{
    enum class LobbyPacketType : int8_t
    {
        CHAT,
        METADATA,
    };

    struct LobbyData final
    {
        LobbyChatCallback chatCallback;         // callback for chat
        LobbyMetadataCallback metadataCallback; // Callback for metadata
        StringHashMap<std::string> metadata;    // meta data map
        std::string firstCache, secondCache;

        struct LobbyPacket
        {
            LobbyPacketType type;
            std::string_view first;
            std::string_view second;
        };

        static void SendChatMessageTo(std::string_view sender, std::string_view message, Connection conn)
        {
            if (message.empty())
                return;
            auto payload = FormatMessage(LobbyPacketType::CHAT, sender, message);
            NetworkSend(conn, payload);
        }

        static void SendMetadataTo(std::string_view key, std::string_view value, Connection conn)
        {
            if (value.empty())
                return;
            auto payload = FormatMessage(LobbyPacketType::METADATA, key, value);
            NetworkSend(conn, payload);
        }

        void handleLobbyPacket(const Message& msg)
        {
            auto packet = readMessage(msg);
            if (packet.type == LobbyPacketType::CHAT)
            {
                if (chatCallback)
                    chatCallback(packet.first, packet.second);
            }
            else if (packet.type == LobbyPacketType::METADATA)
            {
                if (NetworkIsClient())
                    metadata[packet.first] = packet.second;

                if (metadataCallback)
                    metadataCallback(packet.first, packet.second);
            }
            else
            {
                LOG_WARNING("MessageType=255 is reserved for lobby packet!");
            }

            // Replicate step
            if (NetworkIsHost())
            {
                for (const auto conn : NetworkGetConnections())
                {
                    if (conn == msg.connection)
                        continue;

                    if (packet.type == LobbyPacketType::CHAT)
                        SendChatMessageTo(packet.first, packet.second, conn);

                    if (packet.type == LobbyPacketType::METADATA)
                        SendMetadataTo(packet.first, packet.second, conn);
                }
            }
        }

        static Payload FormatMessage(LobbyPacketType type, std::string_view first, std::string_view second)
        {
            first = first.substr(0, MAGIQUE_MAX_LOBBY_MESSAGE_LEN);
            second = second.substr(0, MAGIQUE_MAX_LOBBY_MESSAGE_LEN);

            char buffer[MAGIQUE_MAX_LOBBY_MESSAGE_LEN * 3]{};
            std::memcpy(buffer, &type, 1);
            std::memcpy(buffer + 1, first.data(), first.size() + 1);
            std::memcpy(buffer + 1 + first.size() + 1, second.data(), second.size() + 1);
            return Payload{buffer, (int)first.size() + (int)second.size() + 3, MAGIQUE_LOBBY_PACKET_TYPE};
        }

        LobbyPacket readMessage(const Message& msg)
        {
            auto data = msg.payload.asString();
            const auto type = LobbyPacketType{*(int8_t*)data};

            auto firstEnd = strlen(data + 1);
            firstCache = data + 1;
            secondCache = data + 1 + firstEnd + 1;
            return {type, firstCache, secondCache};
        }

        void closeLobby() { metadata.clear(); }
    };
} // namespace magique
// namespace magique
#endif // MAGEQUEST_LOBBYDATA_H
