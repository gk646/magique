#ifndef MAGEQUEST_LOBBYDATA_H
#define MAGEQUEST_LOBBYDATA_H

#include <magique/util/Datastructures.h>

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
                const auto *key = data;
                const auto *value = data + strlen(key) + 1;
                if (metadataCallback)
                {
                    metadataCallback(msg.connection, key, value);
                }
                if (NetworkIsClient())
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
} // namespace magique
// namespace magique
#endif //MAGEQUEST_LOBBYDATA_H
