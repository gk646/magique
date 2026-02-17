// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#ifndef MAGIQUE_STEAM
#include "magique/core/Types.h"
#include "magique/util/Logging.h"
namespace magique
{
    bool InitSteam(const bool createFile) { return false; };
    SteamID GetUserSteamID() { M_ENABLE_STEAM_ERROR({}) };
    const char* GetSteamUserName() { M_ENABLE_STEAM_ERROR({}); };
    void SetSteamOverlayCallback(SteamOverlayCallback steamOverlayCallback) { M_ENABLE_STEAM_ERROR() }
    const char* GetSteamUserDataLocation() { M_ENABLE_STEAM_ERROR({}) };
} // namespace magique
#else
#include <fstream>

#include <magique/steam/Steam.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>

#include "internal/globals/SteamData.h"

namespace magique
{
    static void SteamDebugCallback(int isWarning, const char* text)
    {
        if (isWarning == 1)
        {
            LOG_WARNING("Steam: %s", text);
        }
        else
        {
            LOG_INFO("Steam: %s", text);
        }
    }

    bool SteamInit(const bool createFile)
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

        MAGIQUE_ASSERT(steamData.isInitialized == false, "Steam already has been initialized");
        SteamErrMsg errMsg;
        if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK)
        {
            LOG_ERROR(errMsg);
            SteamAPI_Shutdown();
            return false;
        }

        SteamUtils()->SetWarningMessageHook(SteamDebugCallback);
        steamData.init();

        // Cache the steam id - will stay the same
        const auto id = SteamUser()->GetSteamID();
        std::memcpy(&steamData.userID, &id, sizeof(id));

        steamData.isInitialized = true;
        LOG_INFO("Initialized steam");
        return true;
    }

    SteamID SteamGetID()
    {
        auto& steamData = global::STEAM_DATA;
        MAGIQUE_ASSERT(steamData.isInitialized, "Steam is not initialized");
        return static_cast<SteamID>(steamData.userID.ConvertToUint64());
    }

    const char* SteamGetUserName() { return SteamFriends()->GetPersonaName(); }

    const char* SteamGetName(SteamID id) { return SteamFriends()->GetFriendPersonaName((uint64)id); }

    void SteamSetOverlayCallback(SteamOverlayCallback steamOverlayCallback)
    {
        global::STEAM_DATA.overlayCallback = steamOverlayCallback;
    }

    static char TEMP[256]{};

    const char* SteamGetUserDataLocation()
    {
        MAGIQUE_ASSERT(global::STEAM_DATA.isInitialized, "Steam is not initialized");
        SteamUser()->GetUserDataFolder(TEMP, 512);
        return TEMP;
    }

} // namespace magique
#endif
