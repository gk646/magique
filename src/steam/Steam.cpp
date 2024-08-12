#include <magique/steam/Steam.h>
#include <magique/util/Logging.h>

#include "internal/globals/SteamData.h"

void CreateAppIDFile()
{
    auto* filename = "steam_appid.txt";
    auto* id = "480";
    auto file = fopen(filename, "wb");

    if (file != nullptr)
    {
        fwrite(id, 3, 1, file);
        fclose(file);
    }
    else
    {
        LOG_ERROR("Unable to create steam_appid.txt file with test id 480 - Do it manually!");
    }
}

namespace magique
{
    bool InitSteam(const bool createFile)
    {
        if (createFile)
            CreateAppIDFile();
        SteamErrMsg errMsg;
        if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK)
        {
            LOG_ERROR(errMsg);
            return false;
        }
        SteamNetworkingSockets()->InitAuthentication();
        SteamNetworkingUtils()->InitRelayNetworkAccess();
        LOG_INFO("Successfully initialized steam");
        return true;
    }
} // namespace magique