#include <fstream>

#include <magique/steam/Steam.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>

#include "internal/globals/SteamData.h"

namespace magique
{
    bool InitSteam(const bool createFile)
    {
        auto& steamData = global::STEAM_DATA;
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
                    LOG_INFO("Created steam_appid.txt with test id 480");
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

        // Cache the steam id - will stay the same
        const auto id = SteamUser()->GetSteamID();
        memcpy(&steamData.userID, &id, sizeof(id));

        steamData.initialized = true;
        LOG_INFO("Successfully initialized steam");
        return true;
    }

    SteamID GetUserSteamID()
    {
        auto& steamData = global::STEAM_DATA;
        MAGIQUE_ASSERT(steamData.initialized, "Steam is not initialized");
        return static_cast<SteamID>(steamData.userID.ConvertToUint64());
    }

    void SetSteamOverlayCallback(SteamOverlayCallback steamOverlayCallback)
    {

    }

    static char TEMP[512]{};

    const char* GetUserDataLocation()
    {
        MAGIQUE_ASSERT(global::STEAM_DATA.initialized, "Steam is not initialized");
        SteamUser()->GetUserDataFolder(TEMP, 512);
        return TEMP;
    }

} // namespace magique