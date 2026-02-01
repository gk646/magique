#include <magique/multiplayer/Lobby.h>

#if defined(MAGIQUE_STEAM) || defined(MAGIQUE_LAN)
#include "internal/globals/MultiplayerData.h"
namespace magique
{
    void SetLobbyChatCallback(const LobbyChatCallback& callback) { global::LOBBY_DATA.chatCallback = callback; }

    void SetLobbyMetadataCallback(const LobbyMetadataCallback& callback)
    {
        global::LOBBY_DATA.metadataCallback = callback;
    }

    inline Lobby LOBBY{};

    Lobby& GetLobby() { return LOBBY; }

#define MG_SESSION_LOCK()                                                                                               \
    if (!GetInMultiplayerSession())                                                                                     \
    {                                                                                                                   \
        return;                                                                                                         \
    }

    void Lobby::setStartSignal(const bool value)
    {
        MG_SESSION_LOCK()
        global::LOBBY_DATA.startSignal = value;

        char buff[2]; // 1 type + signal
        buff[0] = (int8_t)LobbyPacketType::START_SIGNAL;
        buff[1] = (int8_t)value;

        const auto payload = CreatePayload(buff, 2, MAGIQUE_LOBBY_PACKET_TYPE);
        BatchMessageToAll(payload);
    }

    bool Lobby::getStartSignal() const { return global::LOBBY_DATA.startSignal; }

    void Lobby::sendChatMessage(const char* message)
    {
        MG_SESSION_LOCK()
        const auto len = strnlen(message, MAGIQUE_MAX_LOBBY_MESSAGE_LEN);
        if (len > MAGIQUE_MAX_LOBBY_MESSAGE_LEN)
        {
            LOG_WARNING("Sent message longer than limit: %d", MAGIQUE_MAX_LOBBY_MESSAGE_LEN);
            return;
        }
        if (len == 0)
        {
            return;
        }
        char buff[MAGIQUE_MAX_LOBBY_MESSAGE_LEN + 2]; // Message + 1 null terminators + 1 type
        buff[0] = (int8_t)LobbyPacketType::CHAT;
        std::memcpy(buff + 1, message, len);
        buff[len + 1] = '\0';
        const auto payload = CreatePayload(buff, 1 + len + 1, MAGIQUE_LOBBY_PACKET_TYPE);
        BatchMessageToAll(payload);
    }

    void Lobby::setMetadata(const char* key, const char* value)
    {
        MG_SESSION_LOCK()
        const auto keyLen = strnlen(key, MAGIQUE_MAX_LOBBY_MESSAGE_LEN);
        const auto valLen = strnlen(value, MAGIQUE_MAX_LOBBY_MESSAGE_LEN);
        if ((keyLen + valLen) > MAGIQUE_MAX_LOBBY_MESSAGE_LEN)
        {
            LOG_WARNING("Combined metadata longer than limit: %d", MAGIQUE_MAX_LOBBY_MESSAGE_LEN);
            return;
        }

        if (keyLen == 0 || valLen == 0)
        {
            LOG_WARNING("Passed empty strings");
            return;
        }
        global::LOBBY_DATA.metadata[key] = value;

        char buff[MAGIQUE_MAX_LOBBY_MESSAGE_LEN + 3]; // Message + 2 null terminators + 1 type

        buff[0] = (int8_t)LobbyPacketType::METADATA;
        std::memcpy(buff + 1, key, keyLen);
        buff[keyLen + 1] = '\0';
        std::memcpy(buff + 1 + keyLen + 1, value, valLen);
        buff[keyLen + 1 + valLen + 1] = '\0';

        const auto payload = CreatePayload(buff, 1 + keyLen + 1 + valLen + 1, MAGIQUE_LOBBY_PACKET_TYPE);
        if (GetIsClient())
        {
            BatchMessage(GetCurrentConnections().front(), payload);
        }
        else
        {
            BatchMessageToAll(payload);
        }
    }

    const std::string& Lobby::getMetadata(const char* key) { return global::LOBBY_DATA.metadata[key]; }

} // namespace magique
#else

namespace magique
{
    void SetLobbyChatCallback(const LobbyChatCallback& callback) { (void)callback; }

    void SetLobbyMetadataCallback(const LobbyMetadataCallback& callback) { (void)callback; }

    inline Lobby LOBBY{};

    Lobby& GetLobby() { return LOBBY; }

    void Lobby::setStartSignal(const bool value) { (void)value; }

    bool Lobby::getStartSignal() const { return false; }

    void Lobby::sendChatMessage(const char* message) { (void)message; }

    void Lobby::setMetadata(const char* key, const char* value)
    {
        (void)key;
        (void)value;
    }

    const std::string& Lobby::getMetadata(const char* key)
    {
        (void)key;
        static std::string empty;
        return empty;
    }
} // namespace magique
#endif
