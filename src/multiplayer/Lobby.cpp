#include <magique/multiplayer/Lobby.h>

#if defined(MAGIQUE_STEAM) || defined(MAGIQUE_LAN)
#include "internal/globals/NetworkingData.h"
#include "magique/steam/Matchmaking.h"
namespace magique
{
    void LobbySetChatCallback(const LobbyChatCallback& callback) { global::MP_DATA.lobby.chatCallback = callback; }

    void LobbySetMetadataCallback(const LobbyMetadataCallback& callback)
    {
        global::MP_DATA.lobby.metadataCallback = callback;
    }

    void LobbySendChatMsg(std::string_view sender, std::string_view message)
    {
        auto& data = global::MP_DATA.lobby;
        for (auto conn : NetworkGetConnections())
            LobbyData::SendChatMessageTo(sender, message, conn);

        if (SteamIsInLobby())
            SteamLobbySendMsg(message);

        if (data.chatCallback)
            data.chatCallback(sender, message);
    }

    std::string_view LobbyGetMetadata(std::string_view key) { return global::MP_DATA.lobby.metadata[key]; }

    void LobbySetMetadata(std::string_view key, std::string_view value)
    {
        if (!NetworkIsHost())
            return;

        if (key.empty())
        {
            LOG_WARNING("Empty key not allowed");
            return;
        }

        if (SteamIsInLobby())
            SteamLobbySetData(key, value);

        for (auto conn : NetworkGetConnections())
            LobbyData::SendMetadataTo(key, value, conn);

        if (NetworkIsHost())
            global::MP_DATA.lobby.metadata[key] = value;
    }

} // namespace magique
#else

namespace magique
{
    void LobbySetChatCallback(const LobbyChatCallback& callback) { (void)callback; }

    void LobbySetMetadataCallback(const LobbyMetadataCallback& callback) { (void)callback; }

    inline Lobby LOBBY{};

    Lobby& GetLobby() { return LOBBY; }

    void Lobby::setStartSignal(const bool value) { (void)value; }

    bool Lobby::getStartSignal() const { return false; }

    void Lobby::sendChatMsg(std::string_view message) { (void)message; }

    void Lobby::setMetadata(std::string_view key, std::string_view value)
    {
        (void)key;
        (void)value;
    }

    const std::string& Lobby::getMetadata(std::string_view key)
    {
        (void)key;
        static std::string empty;
        return empty;
    }

} // namespace magique
#endif
