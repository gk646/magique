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
    //      - handler: pass a new instance of your callback handler
    //      - createAppIDFile: if true creates a test steam_appid.txt file with the id 480 (test project)
    bool SteamInit(ISteamCallbackHandler* handler = nullptr, bool createAppIDFile = true);

    // Combines all callbacks into a single handler
    struct ISteamCallbackHandler
    {
        virtual ~ISteamCallbackHandler() = default;

        // Called for NewUrlLaunchParameters_t AND NewLaunchQueryParameters_t
        // This happens when the game is started with a command line like steam://run/<appid>//?param1=value1;param2=value2;param3=value3;
        // See: https://partner.steamgames.com/doc/api/ISteamApps#GetLaunchCommandLine
        virtual void onLaunchParam(std::string_view commandLine) {}

        // Called for GameOverlayActivated_t
        // This happens when the overlay is (de)activated
        virtual void onOverlay(bool isActive) {}

        // Helper
        struct SteamStatResult
        {
            // Returns a value only if:
            //      - The stat exists and is published on the steamworks dashboard
            //      - The requested type matches the datatype in the dashboard
            std::optional<int32_t> getInt(std::string_view name) const;
            std::optional<float> getFloat(std::string_view name) const;

        private:
            M_MAKE_PUB()
            SteamID user{};
        };

        // Called for UserStatsReceived_t
        // This happens after you requested stats
        virtual void onStatsRequest(const SteamStatResult& res, SteamID user) {}

        // Called for GamepadTextInputDismissed_t
        // This happens when the text overlay is closed
        virtual void onScreenKeyboardClose(std::string_view input, bool submitted) {}
    };

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

    std::string_view SteamGetLaunchParam(std::string_view key);

    //================= USER =================//

    // Returns your own steam id
    SteamID SteamGetID();

    // Returns the name of current steam user
    const char* SteamGetLocalName();

    // Returns the name of the user with the given id (only works if it's your friend or in a common lobby etc.)
    const char* SteamGetUserName(SteamID id);

    // Retrieve the associated connection with the given steam id
    // Note: Automatically updated whenever possible (e.g. before a network event is fired)
    Connection SteamGetMappedConnection(SteamID id);
    SteamID SteamGetMappedID(Connection conn);

    //================= OVERLAY =================//

    // Opens the overlay for the current user in the given category
    void SteamOpenOverlay(SteamOverlayUserCategory category);

    // Opens the overlay and displays the given users profile
    void SteamOpenOverlayForProfile(SteamID id);

    //================= STATS =================//
    // Allows access to the Stats API https://partner.steamgames.com/doc/features/achievements

    // Requests the latest stat values for the given user - fires a stats callback when done
    void SteamRequestStats(SteamID user = SteamGetID());

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

    // Opens the on screen keyboard for controllers - allows to display a text description and current text already filled in
    void SteamShowOnScreenKeyboard(std::string_view description, std::string_view current = {});

    // Returns true if the onscreen keyboard is visible
    bool SteamIsOnScreenKeyboardShown();

    // Sets the rich presence text
    // See https://partner.steamgames.com/doc/features/enhancedrichpresence
    void SteamSetRichPresence(std::string_view string);

} // namespace magique


#endif // MAGIQUE_STEAM_H
