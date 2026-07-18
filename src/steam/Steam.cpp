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
    std::string_view SteamGetUserName() { M_ENABLE_STEAM_ERROR({}); };
    void SteamSetOverlayCallback(const SteamOverlayCallback& steamOverlayCallback){
        M_ENABLE_STEAM_ERROR()} std::string_view SteamGetUserDataLocation() {
        M_ENABLE_STEAM_ERROR({})
    };
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

    bool SteamInit(ISteamCallbacks* handler, ISteamMatchmakingCallbacks* matchmaking, const bool createFile)
    {
        auto& steamData = global::STEAM_DATA;
        steamData.handler = handler;
        steamData.matchmakingHandler = matchmaking;

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
        return true;
    }

    std::string_view SteamGetLaunchParam(std::string_view key)
    {
        static std::string buffer;
        buffer.resize(256);
        SteamApps()->GetLaunchCommandLine(buffer.data(), buffer.capacity());

        std::string_view view = buffer;
        const auto keyStart = view.find_first_of(key);
        if (keyStart == std::string_view::npos)
            return {};

        const auto valueStart = keyStart + key.length() + 1;
        const auto valueEnd = buffer.find(' ', valueStart);
        return view.substr(valueStart, valueEnd - valueStart);
    }

    uint32_t SteamGetAppID() { return SteamUtils()->GetAppID(); }

    const char* ClientAPIToISO(std::string_view api)
    {
        switch (api.front())
        {
        case 'a': // arabic
            return "ar";
        case 'b':
            if (api == "bulgarian")
                return "bg";
            if (api == "brazilian")
                return "pt";
            break;
        case 'c':
            if (api == "czech")
                return "cs";
            break;
        case 'd':
            if (api == "danish")
                return "da";
            if (api == "dutch")
                return "nl";
            break;
        case 'e':
            if (api == "english")
                return "en";
            break;
        case 'f':
            if (api == "finnish")
                return "fi";
            if (api == "french")
                return "fr";
            break;
        case 'g':
            if (api == "german")
                return "de";
            if (api == "greek")
                return "el";
            break;
        case 'h':
            if (api == "hungarian")
                return "hu";
            break;
        case 'i':
            if (api == "indonesian")
                return "id";
            if (api == "italian")
                return "it";
            break;
        case 'j':
            if (api == "japanese")
                return "ja";
            break;
        case 'k':
            if (api == "koreana")
                return "ko";
            break;
        case 'n':
            if (api == "norwegian")
                return "no";
            break;
        case 'p':
            if (api == "polish")
                return "pl";
            if (api == "portuguese")
                return "pt";
            break;
        case 'r':
            if (api == "romanian")
                return "ro";
            if (api == "russian")
                return "ru";
            break;
        case 's':
            if (api == "schinese")
                return "zh";
            if (api == "spanish")
                return "es";
            if (api == "swedish")
                return "sv";
            break;
        case 't':
            if (api == "tchinese")
                return "zh";
            if (api == "thai")
                return "th";
            if (api == "turkish")
                return "tr";
            break;
        case 'u':
            if (api == "ukrainian")
                return "uk";
            break;
        case 'v':
            if (api == "vietnamese")
                return "vi";
            break;
        case 'l':
            if (api == "latam")
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

    std::vector<Language> SteamGetGameLanguages()
    {
        std::vector<Language> res;
        auto langs = StringSplit(SteamApps()->GetAvailableGameLanguages(), ',');
        for (const auto& lang : langs)
        {
            res.push_back(LocalizationParseLanguage(ClientAPIToISO(lang)));
        }
        return res;
    }

    void SteamMarkGameFilesCorrupt(bool missingFilesOnly) { SteamApps()->MarkContentCorrupt(missingFilesOnly); }

    SteamID SteamGetID()
    {
        auto& steamData = global::STEAM_DATA;
        MAGIQUE_ASSERT(steamData.isInitialized, "Steam is not initialized");
        return static_cast<SteamID>(steamData.userID.ConvertToUint64());
    }

    std::string_view SteamGetLocalName() { return SteamFriends()->GetPersonaName(); }

    std::string_view SteamGetUserName(SteamID id) { return SteamFriends()->GetFriendPersonaName((uint64)id); }

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

    void SteamOpenOverlay(SteamOverlayUserCategory category)
    {
        SteamFriends()->ActivateGameOverlay(enchantum::to_string(category).data());
    }

    void SteamOpenOverlayForProfile(SteamID id)
    {
        auto steamId = CSteamID((uint64)id);
        if (steamId.IsValid())
        {
            SteamFriends()->ActivateGameOverlayToUser("steamid", steamId);
        }
        else
        {
            LOG_WARNING("Cannot show overlay for invalid steam id");
        }
    }

    void SteamRequestStats(SteamID user) { SteamUserStats()->RequestUserStats(CSteamID{static_cast<uint64>(user)}); }

    std::optional<int32_t> ISteamCallbacks::SteamStatResult::getInt(std::string_view name) const
    {
        int32_t data{};
        if (SteamUserStats()->GetUserStat(CSteamID(static_cast<uint64>(user)), name.data(), &data))
            return {data};
        return {};
    }

    std::optional<float> ISteamCallbacks::SteamStatResult::getFloat(std::string_view name) const
    {
        float data{};
        if (SteamUserStats()->GetUserStat(CSteamID(static_cast<uint64>(user)), name.data(), &data))
            return {data};
        return {};
    }

    void SteamSetStat(std::string_view name, std::variant<int32_t, float> value)
    {
        std::visit([&](auto&& data) { SteamUserStats()->SetStat(name.data(), data); }, value);
    }

    void SteamStoreStats() { SteamUserStats()->StoreStats(); }

    std::string_view SteamGetUserDataLocation()
    {
        auto& data = global::STEAM_DATA;
        data.cacheString.resize(512);
        SteamUser()->GetUserDataFolder(data.cacheString.data(), 512);
        return data.cacheString;
    }

    uint32_t SteamGetServerTime() { return SteamUtils()->GetServerRealTime(); }

    void SteamShowOnScreenKeyboard(std::string_view description, std::string_view current)
    {
        static std::string currBuffer;
        currBuffer = current;

        if (global::STEAM_DATA.onscreenKeyboardShown)
        {
            global::STEAM_DATA.onscreenKeyboardShown = false;
        }

        const auto res =
            SteamUtils()->ShowGamepadTextInput(k_EGamepadTextInputModeNormal, k_EGamepadTextInputLineModeSingleLine,
                                               description.data(), 256, currBuffer.c_str());
        if (!res)
        {
            LOG_WARNING("Failed to show steam on screen keyboard");
            global::STEAM_DATA.onscreenKeyboardShown = false;
        }
        else
        {
            global::STEAM_DATA.onscreenKeyboardShown = true;
        }
    }

    bool SteamIsOnScreenKeyboardShown() { return global::STEAM_DATA.onscreenKeyboardShown; }

    void SteamSetRichPresence(std::string_view string)
    {
        // SteamFriends()->SetRichPresence( "steam_display", bDisp
    }

} // namespace magique
#endif
