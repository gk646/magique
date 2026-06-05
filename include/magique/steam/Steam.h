// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STEAM_H
#define MAGIQUE_STEAM_H

#include <magique/core/Types.h>
#include <string_view>
#include <functional>
#include <optional>
#include <variant>

//===============================================
// Steam Module
//===============================================
// .....................................................................
// Note: This module needs steam to be enabled (via CMake: MAGIQUE_STEAM)
// This module is a wrapper for the steam sdk that allows easier and structured access to steam functionalities
// .....................................................................

namespace magique
{
    // Returns true if the initialization of steam was successful
    // If specified creates a test steam_appid.txt file with the id 480 (test project)
    bool SteamInit(bool createAppIDFile = true);

    // Returns the value of the launch parameter with the given key the game was launched with
    // Note: Check out https://partner.steamgames.com/doc/api/ISteamApps#GetLaunchCommandLine
    std::string_view SteamGetLaunchParam(std::string_view key);

    // Sets the callback called when steam launches your game with a commandline
    // Note: Use SteamGetLaunchParam() to access the params
    void SteamSetURLLaunchCallback(const std::function<void()>& callback);

    //================= GAME =================//

    // Returns the app id of the current game
    uint32_t SteamGetAppID();

    // Returns the language set for the game
    Language SteamGetGameLanguage();

    // Returns a list of all available languages for this game
    std::vector<Language> SteamGetGameLanguages();

    // Signals steam to verify and redownload the game files
    //      - missingFilesOnly: if true only checks for missing files, not if existing files are wrong/out-of-date
    void SteamMarkGameFilesCorrupt(bool missingFilesOnly = false);

    //================= USER =================//

    // Returns your own steam id
    SteamID SteamGetID();

    // Returns the name of current steam user
    const char* SteamGetLocalName();

    // Returns the name of the user with the given id (only works if it's your friend or in a common lobby etc.)
    const char* SteamGetUserName(SteamID id);

    using SteamOverlayCallback = std::function<void(bool isOpening)>;

    // Sets the callback function called when the steam overlay is opened or closed
    void SteamSetOverlayCallback(const SteamOverlayCallback& callback);

    // Retrieve the associated connection with the given steam id
    // Note: Automatically updated whenever possible (e.g. before a network event is fired)
    Connection SteamGetMappedConnection(SteamID id);
    SteamID SteamGetMappedID(Connection conn);

    //================= STATS =================//
    // Allows access to the Stats API https://partner.steamgames.com/doc/features/achievements

    // Requests the latest stat values for the given user - fires a stats callback when done
    void SteamRequestStats(SteamID user = SteamGetID());

    struct SteamStatResult
    {
        // Returns a value only if:
        //      - The stat exists and is published on the steamworks dashboard
        //      - The requested type matches the datatype in the dashboard
        std::optional<int32_t> getInt(std::string_view name) const;
        std::optional<float> getFloat(std::string_view name) const;

    private:
        M_MAKE_PUB()
        SteamID user;
    };

    // Called when a stat request finishes˛for the given user
    // Note: The data can be accessed by calling the getters on the result
    void SteamSetStatsRequestCallback(const std::function<void(const SteamStatResult& res, SteamID user)>& callback);

    // Sets the given stats to the given value
    // Note: This is a cheap call and only modifies the in memory value of the steam client -> call often incase of crash
    void SteamSetStat(std::string_view name, std::variant<int32_t, float> value);

    // Commits the current in-memory stats to the server
    // Note: Should be called during major changes (e.g. map change, new round) so rather minutes not seconds
    // Note: Might trigger a stats request callback if some updated values are rejected - the current values are then returned
    void SteamStoreStats();

    //================= PERSISTENCE =================//

    // Returns the path of the local user data folder specific to the game and steam id
    // Note: This should be used to load and store local data for steam games - automatically adjusts to game and user
    const char* SteamGetUserDataLocation();

    //================= UTIL =================//

    // Steam server time - Number of SECONDS since January 1, 1970, GMT (unix time)
    uint32_t SteamGetServerTime();

} // namespace magique


#endif // MAGIQUE_STEAM_H
