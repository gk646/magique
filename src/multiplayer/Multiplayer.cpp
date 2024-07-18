#include <fstream>
#include <steam/steam_api.h>

#include <magique/multiplayer/Multiplayer.h>
#include <magique/util/Logging.h>
#include <magique/util/Defines.h>

// Thank me later for that
void CreateAppIDFile()
{
    auto* filename = "steam_appid.txt";
    std::ofstream file(filename);

    if (file.is_open())
    {
        file << "480";
        file.close();
    }
    else
    {
        LOG_ERROR("Unable to create steam_appid.txt file with test id 480 - Do it manually!");
    }
}

namespace magique
{
    bool InitMultiplayer()
    {
#if MAGIQUE_DEBUG == 1
        CreateAppIDFile();
#endif

        SteamErrMsg errMsg;
        if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK)
        {
            LOG_ERROR(errMsg);
            return false;
        }
        LOG_INFO("Successfully initialized multiplayer");
        return true;
    }

    bool CreateGameLobby(const LobbyType type, const int maxPlayers)
    {
        SteamAPICall_t hSteamAPICall = k_uAPICallInvalid;
        switch (type)
        {
        case LobbyType::PRIVATE:
            hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePrivate, maxPlayers);
            break;
        case LobbyType::FRIENDS_ONLY:
            hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypeFriendsOnly, maxPlayers);
            break;
        case LobbyType::PUBLIC:
            hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, maxPlayers);
            break;
        }
        return hSteamAPICall != k_uAPICallInvalid;
    }


} // namespace magique