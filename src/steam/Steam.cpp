// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <magique/steam/Steam.h>


#ifndef MAGIQUE_STEAM
#include "magique/core/Types.h"
#include "magique/util/Logging.h"
namespace magique
{
    bool SteamInit(const bool createFile) { return false; };
    SteamID SteamGetID() { M_ENABLE_STEAM_ERROR({}) };
    const char* SteamGetUserName() { M_ENABLE_STEAM_ERROR({}); };
    void SteamSetOverlayCallback(const SteamOverlayCallback& steamOverlayCallback) { M_ENABLE_STEAM_ERROR() }
    const char* SteamGetUserDataLocation() { M_ENABLE_STEAM_ERROR({}) };
} // namespace magique
#else
#include <fstream>

#include <magique/util/Logging.h>
#include <magique/gamedev/Localization.h>

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
        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);

        steamData.init();

        // Cache the steam id - will stay the same
        const auto id = SteamUser()->GetSteamID();
        std::memcpy(&steamData.userID, &id, sizeof(id));

        steamData.isInitialized = true;
        LOG_INFO("Initialized steam");
        return true;
    }

    std::string_view SteamGetLaunchParam(std::string_view key) { return SteamApps()->GetLaunchQueryParam(key.data()); }

    void SteamSetURLLaunchCallback(const std::function<void()>& callback)
    {
        global::STEAM_DATA.urlLaunchCallback = callback;
    }

    uint32_t SteamGetAppID() { return SteamUtils()->GetAppID(); }

    const char* ClientAPIToISO(const char* apiLanguageCode)
    {
        switch (*apiLanguageCode)
        {
        case 'a': // arabic
            return "ar";
        case 'b':
            if (strcmp(apiLanguageCode, "bulgarian") == 0)
                return "bg";
            if (strcmp(apiLanguageCode, "brazilian") == 0)
                return "pt";
            break;
        case 'c':
            if (strcmp(apiLanguageCode, "czech") == 0)
                return "cs";
            break;
        case 'd':
            if (strcmp(apiLanguageCode, "danish") == 0)
                return "da";
            if (strcmp(apiLanguageCode, "dutch") == 0)
                return "nl";
            break;
        case 'e':
            if (strcmp(apiLanguageCode, "english") == 0)
                return "en";
            break;
        case 'f':
            if (strcmp(apiLanguageCode, "finnish") == 0)
                return "fi";
            if (strcmp(apiLanguageCode, "french") == 0)
                return "fr";
            break;
        case 'g':
            if (strcmp(apiLanguageCode, "german") == 0)
                return "de";
            if (strcmp(apiLanguageCode, "greek") == 0)
                return "el";
            break;
        case 'h':
            if (strcmp(apiLanguageCode, "hungarian") == 0)
                return "hu";
            break;
        case 'i':
            if (strcmp(apiLanguageCode, "indonesian") == 0)
                return "id";
            if (strcmp(apiLanguageCode, "italian") == 0)
                return "it";
            break;
        case 'j':
            if (strcmp(apiLanguageCode, "japanese") == 0)
                return "ja";
            break;
        case 'k':
            if (strcmp(apiLanguageCode, "koreana") == 0)
                return "ko";
            break;
        case 'n':
            if (strcmp(apiLanguageCode, "norwegian") == 0)
                return "no";
            break;
        case 'p':
            if (strcmp(apiLanguageCode, "polish") == 0)
                return "pl";
            if (strcmp(apiLanguageCode, "portuguese") == 0)
                return "pt";
            break;
        case 'r':
            if (strcmp(apiLanguageCode, "romanian") == 0)
                return "ro";
            if (strcmp(apiLanguageCode, "russian") == 0)
                return "ru";
            break;
        case 's':
            if (strcmp(apiLanguageCode, "schinese") == 0)
                return "zh";
            if (strcmp(apiLanguageCode, "spanish") == 0)
                return "es";
            if (strcmp(apiLanguageCode, "swedish") == 0)
                return "sv";
            break;
        case 't':
            if (strcmp(apiLanguageCode, "tchinese") == 0)
                return "zh";
            if (strcmp(apiLanguageCode, "thai") == 0)
                return "th";
            if (strcmp(apiLanguageCode, "turkish") == 0)
                return "tr";
            break;
        case 'u':
            if (strcmp(apiLanguageCode, "ukrainian") == 0)
                return "uk";
            break;
        case 'v':
            if (strcmp(apiLanguageCode, "vietnamese") == 0)
                return "vi";
            break;
        case 'l':
            if (strcmp(apiLanguageCode, "latam") == 0)
                return "es";
            break;
        default:
            return nullptr;
        }
        return nullptr;
    }

    Language SteamGetGameLanguage()
    {
        const auto lang = SteamApps()->GetCurrentGameLanguage();
        const auto res = ClientAPIToISO(lang);
        return LocalizationParseLanguage(res);
    }

    void SteamMarkGameFilesCorrupt(bool missingFilesOnly) { SteamApps()->MarkContentCorrupt(missingFilesOnly); }

    SteamID SteamGetID()
    {
        auto& steamData = global::STEAM_DATA;
        MAGIQUE_ASSERT(steamData.isInitialized, "Steam is not initialized");
        return static_cast<SteamID>(steamData.userID.ConvertToUint64());
    }

    const char* SteamGetLocalName() { return SteamFriends()->GetPersonaName(); }

    const char* SteamGetUserName(SteamID id) { return SteamFriends()->GetFriendPersonaName((uint64)id); }

    void SteamSetOverlayCallback(const SteamOverlayCallback& callback) { global::STEAM_DATA.overlayCallback = callback; }

    Connection SteamGetMappedConnection(SteamID id)
    {
        for (auto& mapping : global::MP_DATA.steamMapping)
        {
            if (mapping.steam == id)
            {
                return mapping.conn;
            }
        }
        return Connection::INVALID;
    }

    SteamID SteamGetMappedID(Connection conn)
    {
        for (auto& mapping : global::MP_DATA.steamMapping)
        {
            if (mapping.conn == conn)
            {
                return mapping.steam;
            }
        }
        return SteamID::INVALID;
    }

    void SteamRequestStats(SteamID user) { SteamUserStats()->RequestUserStats(CSteamID{static_cast<uint64>(user)}); }

    std::optional<int32_t> SteamStatResult::getInt(std::string_view name) const
    {
        int32_t data{};
        if (SteamUserStats()->GetUserStat(CSteamID(static_cast<uint64>(user)), name.data(), &data))
            return {data};
        return {};
    }

    std::optional<float> SteamStatResult::getFloat(std::string_view name) const
    {
        float data{};
        if (SteamUserStats()->GetUserStat(CSteamID(static_cast<uint64>(user)), name.data(), &data))
            return {data};
        return {};
    }

    void SteamSetStatsRequestCallback(const std::function<void(const SteamStatResult& res, SteamID user)>& callback)
    {
        global::STEAM_DATA.statsCallback = callback;
    }

    void SteamSetStat(std::string_view name, std::variant<int32_t, float> value)
    {
        std::visit(
            [&](auto&& data)
            {
                SteamUserStats()->SetStat(name.data(), data);
            },
            value);
    }

    void SteamStoreStats() { SteamUserStats()->StoreStats(); }

    const char* SteamGetUserDataLocation()
    {
        static char TEMP[512]{};
        MAGIQUE_ASSERT(global::STEAM_DATA.isInitialized, "Steam is not initialized");
        SteamUser()->GetUserDataFolder(TEMP, 512);
        return TEMP;
    }

} // namespace magique
#endif
