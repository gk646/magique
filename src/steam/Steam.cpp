#include <magique/steam/Steam.h>
#include <magique/util/Logging.h>

#include "internal/globals/SteamData.h"

namespace magique
{
    bool InitSteam(const bool createFile)
    {
        if (createFile)
        {
            constexpr auto* filename = "steam_appid.txt";
            constexpr auto* id = "480";

            const std::ifstream file(filename);
            if (!file.good())
            {
                const auto newFile = fopen(filename, "wb");
                if (newFile != nullptr)
                {
                    fwrite(id, 3, 1, newFile);
                    fclose(newFile);
                }
                else
                {
                    LOG_ERROR("Unable to create steam_appid.txt file with test id 480 - Do it manually!");
                }
            }
        }

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

    static char TEMP[512]{};

    const char* GetUserDataLocation()
    {
        SteamUser()->GetUserDataFolder(TEMP, 512);
        return TEMP;
    }

} // namespace magique