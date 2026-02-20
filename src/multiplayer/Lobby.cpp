#include <magique/multiplayer/Lobby.h>

#if defined(MAGIQUE_STEAM) || defined(MAGIQUE_LAN)
#include "internal/globals/NetworkingData.h"
namespace magique
{
    void LobbySetChatCallback(const LobbyChatCallback& callback) { global::MP_DATA.lobby.chatCallback = callback; }

    void LobbySetMetadataCallback(const LobbyMetadataCallback& callback)
    {
        global::MP_DATA.lobby.metadataCallback = callback;
    }

    inline Lobby LOBBY{};

    Lobby& LobbyGet() { return LOBBY; }

#define MG_SESSION_LOCK()                                                                                               \
    if (!NetworkInSession())                                                                                            \
    {                                                                                                                   \
        return;                                                                                                         \
    }

    void Lobby::setStartSignal(const bool value)
    {
        MG_SESSION_LOCK()
        global::MP_DATA.lobby.startSignal = value;

        char buff[2]; // 1 type + signal
        buff[0] = (int8_t)LobbyPacketType::START_SIGNAL;
        buff[1] = (int8_t)value;

        const auto payload = Payload(buff, 2, MAGIQUE_LOBBY_PACKET_TYPE);
        NetworkSendAll(payload);
    }

    bool Lobby::getStartSignal() const { return global::MP_DATA.lobby.startSignal; }

    void Lobby::sendChatMessage(std::string_view message)
    {
        MG_SESSION_LOCK()
        if (message.size() > MAGIQUE_MAX_LOBBY_MESSAGE_LEN)
        {
            LOG_WARNING("Sent message longer than limit: %d", MAGIQUE_MAX_LOBBY_MESSAGE_LEN);
            return;
        }
        if (message.empty())
        {
            return;
        }

        char buff[MAGIQUE_MAX_LOBBY_MESSAGE_LEN + 2]; // Message + 1 null terminators + 1 type
        buff[0] = (int8_t)LobbyPacketType::CHAT;
        std::memcpy(buff + 1, message.data(), message.size());
        buff[1 + message.size() + 1] = '\0';
        const auto payload = Payload(buff, 1 + message.size() + 1, MAGIQUE_LOBBY_PACKET_TYPE);
        NetworkSendAll(payload);
    }

    void Lobby::setMetadata(std::string_view key, std::string_view value)
    {
        MG_SESSION_LOCK()
        if ((key.size() + value.size()) > MAGIQUE_MAX_LOBBY_MESSAGE_LEN)
        {
            LOG_WARNING("Combined metadata longer than limit: %d", MAGIQUE_MAX_LOBBY_MESSAGE_LEN);
            return;
        }
        if (key.empty())
        {
            LOG_WARNING("Empty key not allowed");
            return;
        }

        global::MP_DATA.lobby.metadata[key] = value;

        char buff[MAGIQUE_MAX_LOBBY_MESSAGE_LEN + 3]; // Message + 2 null terminators + 1 type

        buff[0] = (int8_t)LobbyPacketType::METADATA;
        std::memcpy(buff + 1, key.data(), key.size());
        buff[key.size() + 1] = '\0';
        std::memcpy(buff + 1 + key.size() + 1, value.data(), value.size());
        buff[key.size() + 1 + value.size() + 1] = '\0';

        const auto payload = Payload(buff, key.size() + 1 + value.size() + 1 + 1, MAGIQUE_LOBBY_PACKET_TYPE);
        NetworkSendAll(payload);
    }

    const std::string& Lobby::getMetadata(std::string_view key) { return global::MP_DATA.lobby.metadata[key]; }

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
